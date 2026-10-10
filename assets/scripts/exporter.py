#!/usr/bin/env python3
"""
Motor de Exportación para Cuaderno de Notas
Soporta exportación limpia a EPUB, DOCX, PDF y Markdown sin dependencias externas.
Usa exclusivamente la biblioteca estándar de Python (zipfile, json, re, sys, os).
"""

import sys
import os
import json
import re
import zipfile
import html
import datetime

# ==============================================================================
# 1. Utilidades y Extracción de Markdown
# ==============================================================================

def clean_filename(name):
    clean = re.sub(r'[\\/*?:"<>|]', "", name)
    return clean.strip().replace(" ", "_")

def markdown_to_html_blocks(md_text):
    """Convierte texto markdown básico a bloques HTML."""
    lines = md_text.splitlines()
    html_parts = []
    in_code = False
    code_lines = []
    in_list = False

    for line in lines:
        stripped = line.strip()

        if stripped.startswith("```"):
            if in_code:
                in_code = False
                code_content = html.escape("\n".join(code_lines))
                html_parts.append(f"<pre><code>{code_content}</code></pre>")
                code_lines = []
            else:
                in_code = True
                code_lines = []
            continue

        if in_code:
            code_lines.append(line)
            continue

        if not stripped:
            if in_list:
                html_parts.append("</ul>")
                in_list = False
            continue

        # Formato inline
        fmt = html.escape(stripped)
        fmt = re.sub(r'\*\*(.+?)\*\*', r'<strong>\1</strong>', fmt)
        fmt = re.sub(r'\*(.+?)\*', r'<em>\1</em>', fmt)
        fmt = re.sub(r'`(.+?)`', r'<code>\1</code>', fmt)

        if stripped.startswith("### "):
            if in_list: html_parts.append("</ul>"); in_list = False
            html_parts.append(f"<h3>{fmt[4:]}</h3>")
        elif stripped.startswith("## "):
            if in_list: html_parts.append("</ul>"); in_list = False
            html_parts.append(f"<h2>{fmt[3:]}</h2>")
        elif stripped.startswith("# "):
            if in_list: html_parts.append("</ul>"); in_list = False
            html_parts.append(f"<h1>{fmt[2:]}</h1>")
        elif stripped.startswith("> "):
            if in_list: html_parts.append("</ul>"); in_list = False
            html_parts.append(f"<blockquote>{fmt[2:]}</blockquote>")
        elif stripped.startswith("* ") or stripped.startswith("- "):
            if not in_list:
                html_parts.append("<ul>")
                in_list = True
            html_parts.append(f"<li>{fmt[2:]}</li>")
        elif stripped.startswith("---") or stripped.startswith("***"):
            if in_list: html_parts.append("</ul>"); in_list = False
            html_parts.append("<hr/>")
        else:
            if in_list: html_parts.append("</ul>"); in_list = False
            html_parts.append(f"<p>{fmt}</p>")

    if in_list:
        html_parts.append("</ul>")
    if in_code:
        code_content = html.escape("\n".join(code_lines))
        html_parts.append(f"<pre><code>{code_content}</code></pre>")

    return "\n".join(html_parts)


# ==============================================================================
# 2. Generador EPUB (.epub)
# ==============================================================================

def export_epub(doc_data, out_path):
    title = html.escape(doc_data.get("title", "Documento"))
    author = html.escape(doc_data.get("author", "Usuario"))
    subtitle = html.escape(doc_data.get("subtitle", ""))
    chapters = doc_data.get("chapters", [])
    now_iso = datetime.datetime.utcnow().strftime("%Y-%m-%dT%H:%M:%SZ")

    with zipfile.ZipFile(out_path, 'w') as zf:
        # 1. mimetype (debe ser el primero y sin comprimir según spec EPUB)
        zf.writestr("mimetype", "application/epub+zip", compress_type=zipfile.ZIP_STORED)

        # 2. container.xml
        container_xml = '''<?xml version="1.0" encoding="UTF-8"?>
<container version="1.0" xmlns="urn:oasis:names:tc:opendocument:xmlns:container">
  <rootfiles>
    <rootfile full-path="OEBPS/content.opf" media-type="application/oebps-package+xml"/>
  </rootfiles>
</container>'''
        zf.writestr("META-INF/container.xml", container_xml)

        # 3. CSS
        css = '''
body {
    font-family: Georgia, "Times New Roman", serif;
    line-height: 1.6;
    margin: 5% 8%;
    color: #222;
}
h1 { color: #1a365d; border-bottom: 2px solid #e2e8f0; padding-bottom: 6px; margin-top: 1.5em; }
h2 { color: #2b6cb0; margin-top: 1.2em; }
h3 { color: #4a5568; }
blockquote {
    border-left: 4px solid #cbd5e0;
    margin: 1em 0;
    padding-left: 1em;
    color: #4a5568;
    font-style: italic;
}
.marginalia {
    background: #f7fafc;
    border: 1px dashed #cbd5e0;
    border-radius: 6px;
    padding: 8px 14px;
    margin: 1.2em 0;
    font-size: 0.9em;
    color: #718096;
}
.cover-title { font-size: 2.2em; text-align: center; margin-top: 25%; color: #1a202c; }
.cover-sub { font-size: 1.2em; text-align: center; color: #718096; font-style: italic; }
.cover-author { font-size: 1.1em; text-align: center; margin-top: 4em; color: #4a5568; }
hr { border: 0; height: 1px; background: #e2e8f0; margin: 2em 0; }
'''
        zf.writestr("OEBPS/style.css", css)

        # 4. Portada (Cover)
        cover_html = f'''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <title>{title}</title>
  <link rel="stylesheet" type="text/css" href="style.css"/>
</head>
<body>
  <div class="cover-title">{title}</div>
  <div class="cover-sub">{subtitle}</div>
  <div class="cover-author">Por: {author}</div>
</body>
</html>'''
        zf.writestr("OEBPS/cover.xhtml", cover_html)

        # 5. Generar cada página/hoja
        manifest_items = [
            '<item id="style" href="style.css" media-type="text/css"/>',
            '<item id="cover" href="cover.xhtml" media-type="application/xhtml+xml"/>',
            '<item id="ncx" href="toc.ncx" media-type="application/x-dtbncx+xml"/>'
        ]
        spine_items = ['<itemref idref="cover"/>']
        nav_points = []
        nav_idx = 1

        for c_idx, ch in enumerate(chapters):
            ch_title = html.escape(ch.get("title", f"Sección {c_idx+1}"))
            for p_idx, pg in enumerate(ch.get("pages", [])):
                p_title = html.escape(pg.get("title", f"Hoja {p_idx+1}"))
                content_html = markdown_to_html_blocks(pg.get("content", ""))
                marginalia = pg.get("marginalia", "").strip()

                marg_html = ""
                if marginalia:
                    marg_html = f'<div class="marginalia"><strong>✍ Notas:</strong> {html.escape(marginalia)}</div>'

                page_id = f"page_{c_idx}_{p_idx}"
                filename = f"{page_id}.xhtml"

                page_xhtml = f'''<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html>
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <title>{p_title}</title>
  <link rel="stylesheet" type="text/css" href="style.css"/>
</head>
<body>
  <p style="font-size:0.85em; color:#a0aec0; text-transform:uppercase;">{ch_title}</p>
  <h1>{p_title}</h1>
  {content_html}
  {marg_html}
</body>
</html>'''
                zf.writestr(f"OEBPS/{filename}", page_xhtml)
                manifest_items.append(f'<item id="{page_id}" href="{filename}" media-type="application/xhtml+xml"/>')
                spine_items.append(f'<itemref idref="{page_id}"/>')

                nav_points.append(f'''    <navPoint id="nav_{nav_idx}" playOrder="{nav_idx}">
      <navLabel><text>{p_title}</text></navLabel>
      <content src="{filename}"/>
    </navPoint>''')
                nav_idx += 1

        # 6. toc.ncx (Tabla de contenidos NCX)
        toc_ncx = f'''<?xml version="1.0" encoding="UTF-8"?>
<ncx xmlns="http://www.daisy.org/z3986/2005/ncx/" version="2005-1">
  <head>
    <meta name="dtb:uid" content="urn:uuid:cuadernonotas-{int(datetime.datetime.now().timestamp())}"/>
    <meta name="dtb:depth" content="1"/>
    <meta name="dtb:totalPageCount" content="0"/>
    <meta name="dtb:maxPageNumber" content="0"/>
  </head>
  <docTitle><text>{title}</text></docTitle>
  <navMap>
{chr(10).join(nav_points)}
  </navMap>
</ncx>'''
        zf.writestr("OEBPS/toc.ncx", toc_ncx)

        # 7. content.opf
        content_opf = f'''<?xml version="1.0" encoding="UTF-8"?>
<package xmlns="http://www.idpf.org/2007/opf" unique-identifier="BookId" version="2.0">
  <metadata xmlns:dc="http://purl.org/dc/elements/1.1/" xmlns:opf="http://www.idpf.org/2007/opf">
    <dc:title>{title}</dc:title>
    <dc:creator>{author}</dc:creator>
    <dc:language>es</dc:language>
    <dc:date>{now_iso}</dc:date>
    <dc:identifier id="BookId">urn:uuid:cuadernonotas-{int(datetime.datetime.now().timestamp())}</dc:identifier>
  </metadata>
  <manifest>
    {chr(10).join(manifest_items)}
  </manifest>
  <spine toc="ncx">
    {chr(10).join(spine_items)}
  </spine>
</package>'''
        zf.writestr("OEBPS/content.opf", content_opf)

    return True


# ==============================================================================
# 3. Generador DOCX (.docx)
# ==============================================================================

def export_docx(doc_data, out_path):
    title = html.escape(doc_data.get("title", "Documento"))
    author = html.escape(doc_data.get("author", "Usuario"))
    subtitle = html.escape(doc_data.get("subtitle", ""))
    chapters = doc_data.get("chapters", [])

    body_xml_parts = []

    # Portada DOCX
    body_xml_parts.append(f'''
    <w:p>
      <w:pPr><w:pStyle w:val="Title"/><w:jc w:val="center"/></w:pPr>
      <w:r><w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:b/><w:sz w:val="52"/></w:rPr><w:t>{title}</w:t></w:r>
    </w:p>''')

    if subtitle:
        body_xml_parts.append(f'''
    <w:p>
      <w:pPr><w:jc w:val="center"/></w:pPr>
      <w:r><w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:i/><w:color w:val="666666"/><w:sz w:val="28"/></w:rPr><w:t>{subtitle}</w:t></w:r>
    </w:p>''')

    body_xml_parts.append(f'''
    <w:p>
      <w:pPr><w:jc w:val="center"/></w:pPr>
      <w:r><w:rPr><w:rFonts w:ascii="Calibri" w:hAnsi="Calibri"/><w:sz w:val="22"/><w:color w:val="888888"/></w:rPr><w:t>Autor: {author}</w:t></w:r>
    </w:p>
    <w:p><w:r><w:br w:type="page"/></w:r></w:p>''')

    # Contenido por capítulos y hojas
    for ch in chapters:
        ch_title = html.escape(ch.get("title", "Sección"))
        body_xml_parts.append(f'''
    <w:p>
      <w:pPr><w:pStyle w:val="Heading1"/></w:pPr>
      <w:r><w:rPr><w:b/><w:sz w:val="36"/><w:color w:val="1F497D"/></w:rPr><w:t>{ch_title}</w:t></w:r>
    </w:p>''')

        for pg in ch.get("pages", []):
            p_title = html.escape(pg.get("title", "Hoja"))
            content = pg.get("content", "")
            marginalia = pg.get("marginalia", "").strip()

            body_xml_parts.append(f'''
    <w:p>
      <w:pPr><w:pStyle w:val="Heading2"/></w:pPr>
      <w:r><w:rPr><w:b/><w:sz w:val="28"/><w:color w:val="2E75B6"/></w:rPr><w:t>{p_title}</w:t></w:r>
    </w:p>''')

            # Párrafos de contenido
            for line in content.splitlines():
                stripped = line.strip()
                if not stripped:
                    continue

                if stripped.startswith("### "):
                    h3 = html.escape(stripped[4:])
                    body_xml_parts.append(f'<w:p><w:pPr><w:pStyle w:val="Heading3"/></w:pPr><w:r><w:rPr><w:b/><w:sz w:val="24"/></w:rPr><w:t>{h3}</w:t></w:r></w:p>')
                elif stripped.startswith("## "):
                    h2 = html.escape(stripped[3:])
                    body_xml_parts.append(f'<w:p><w:pPr><w:pStyle w:val="Heading2"/></w:pPr><w:r><w:rPr><w:b/><w:sz w:val="26"/></w:rPr><w:t>{h2}</w:t></w:r></w:p>')
                elif stripped.startswith("# "):
                    h1 = html.escape(stripped[2:])
                    body_xml_parts.append(f'<w:p><w:pPr><w:pStyle w:val="Heading1"/></w:pPr><w:r><w:rPr><w:b/><w:sz w:val="30"/></w:rPr><w:t>{h1}</w:t></w:r></w:p>')
                elif stripped.startswith("> "):
                    q = html.escape(stripped[2:])
                    body_xml_parts.append(f'<w:p><w:pPr><w:ind w:left="720"/></w:pPr><w:r><w:rPr><w:i/><w:color w:val="555555"/></w:rPr><w:t>{q}</w:t></w:r></w:p>')
                elif stripped.startswith("* ") or stripped.startswith("- "):
                    b = html.escape(stripped[2:])
                    body_xml_parts.append(f'<w:p><w:pPr><w:ind w:left="480"/></w:pPr><w:r><w:t>• {b}</w:t></w:r></w:p>')
                else:
                    t = html.escape(stripped)
                    body_xml_parts.append(f'<w:p><w:r><w:t>{t}</w:t></w:r></w:p>')

            # Marginalia en DOCX
            if marginalia:
                m_esc = html.escape(marginalia)
                body_xml_parts.append(f'''
    <w:p>
      <w:pPr><w:ind w:left="720"/><w:shd w:val="clear" w:color="auto" w:fill="F2F4F7"/></w:pPr>
      <w:r><w:rPr><w:b/><w:sz w:val="18"/><w:color w:val="777777"/></w:rPr><w:t>✍ Notas: </w:t></w:r>
      <w:r><w:rPr><w:i/><w:sz w:val="18"/><w:color w:val="555555"/></w:rPr><w:t>{m_esc}</w:t></w:r>
    </w:p>''')

            body_xml_parts.append('<w:p><w:pPr><w:spacing w:after="240"/></w:pPr></w:p>')

    doc_xml = f'''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
  <w:body>
    {''.join(body_xml_parts)}
    <w:sectPr>
      <w:pgSz w:w="11906" w:h="16838"/>
      <w:pgMar w:top="1440" w:right="1440" w:bottom="1440" w:left="1440"/>
    </w:sectPr>
  </w:body>
</w:document>'''

    content_types = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
  <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
  <Default Extension="xml" ContentType="application/xml"/>
  <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
</Types>'''

    rels = '''<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
  <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
</Relationships>'''

    with zipfile.ZipFile(out_path, 'w', compression=zipfile.ZIP_DEFLATED) as zf:
        zf.writestr("[Content_Types].xml", content_types)
        zf.writestr("_rels/.rels", rels)
        zf.writestr("word/document.xml", doc_xml)

    return True


# ==============================================================================
# 4. Generador PDF 1.4 Nativo Puro
# ==============================================================================

class SimplePDFBuilder:
    def __init__(self):
        self.objects = []
        self.pages = []
        self.page_w = 595.28  # A4 pt
        self.page_h = 841.89  # A4 pt
        self.margin_x = 54.0
        self.margin_y = 54.0

    def add_object(self, content_bytes):
        self.objects.append(content_bytes)
        return len(self.objects)

    def escape_pdf_str(self, text):
        safe = []
        replacements = {
            '—': '-', '–': '-', '“': '"', '”': '"', '‘': "'", '’': "'", '•': '*'
        }
        for ch in text:
            if ch in replacements:
                ch = replacements[ch]
            if ch in ('(', ')', '\\'):
                safe.append('\\' + ch)
            elif ord(ch) < 256:
                safe.append(ch)
            else:
                safe.append('?')
        return "".join(safe)

    def build(self, doc_data, out_path):
        title = doc_data.get("title", "Documento")
        author = doc_data.get("author", "Usuario")
        subtitle = doc_data.get("subtitle", "")
        chapters = doc_data.get("chapters", [])

        # Armar líneas de texto formateadas
        lines = []
        lines.append(("COVER_TITLE", title))
        if subtitle:
            lines.append(("COVER_SUB", subtitle))
        lines.append(("COVER_AUTH", f"Autor: {author}"))
        lines.append(("PAGE_BREAK", ""))

        for ch in chapters:
            lines.append(("H1", ch.get("title", "Sección")))
            lines.append(("SEP", ""))
            for pg in ch.get("pages", []):
                lines.append(("H2", pg.get("title", "Hoja")))
                content = pg.get("content", "")
                for raw_line in content.splitlines():
                    s = raw_line.strip()
                    if not s:
                        lines.append(("SPACE", ""))
                    elif s.startswith("### "):
                        lines.append(("H3", s[4:]))
                    elif s.startswith("## "):
                        lines.append(("H3", s[3:]))
                    elif s.startswith("# "):
                        lines.append(("H2", s[2:]))
                    elif s.startswith("> "):
                        lines.append(("QUOTE", s[2:]))
                    elif s.startswith("* ") or s.startswith("- "):
                        lines.append(("BULLET", s[2:]))
                    elif s.startswith("---"):
                        lines.append(("SEP", ""))
                    else:
                        lines.append(("P", s))

                marginalia = pg.get("marginalia", "").strip()
                if marginalia:
                    lines.append(("MARG", f"Notas: {marginalia}"))
                lines.append(("PAGE_BREAK", ""))

        # Paginador
        pdf_pages = []
        cur_ops = []
        cur_y = self.page_h - self.margin_y - 20.0
        max_w = self.page_w - (self.margin_x * 2.0)

        def new_page():
            nonlocal cur_ops, cur_y
            if cur_ops:
                pdf_pages.append("\n".join(cur_ops))
            cur_ops = []
            cur_y = self.page_h - self.margin_y - 20.0

        for kind, text in lines:
            if kind == "PAGE_BREAK":
                new_page()
                continue

            if cur_y < self.margin_y + 40.0:
                new_page()

            if kind == "COVER_TITLE":
                cur_y -= 120.0
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"BT /F2 26 Tf 0.1 0.2 0.4 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 36.0
            elif kind == "COVER_SUB":
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"BT /F3 14 Tf 0.4 0.4 0.4 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 24.0
            elif kind == "COVER_AUTH":
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"BT /F1 11 Tf 0.5 0.5 0.5 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 40.0
            elif kind == "H1":
                cur_y -= 24.0
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"BT /F2 18 Tf 0.12 0.28 0.52 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 10.0
                cur_ops.append(f"0.8 0.8 0.8 RG 1 w {self.margin_x} {cur_y} m {self.page_w - self.margin_x} {cur_y} l S")
                cur_y -= 14.0
            elif kind == "H2":
                cur_y -= 16.0
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"BT /F2 14 Tf 0.18 0.38 0.65 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 18.0
            elif kind == "H3":
                cur_y -= 10.0
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"BT /F2 11 Tf 0.25 0.25 0.25 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 15.0
            elif kind == "QUOTE":
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"0.7 0.7 0.7 RG 2 w {self.margin_x} {cur_y+10} m {self.margin_x} {cur_y-2} l S")
                cur_ops.append(f"BT /F3 10 Tf 0.35 0.35 0.35 rg {self.margin_x + 12} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 16.0
            elif kind == "BULLET":
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"BT /F1 10.5 Tf 0.15 0.15 0.15 rg {self.margin_x + 12} {cur_y} Td (* {safe}) Tj ET")
                cur_y -= 15.0
            elif kind == "MARG":
                safe = self.escape_pdf_str(text)
                cur_ops.append(f"0.92 0.94 0.96 rg {self.margin_x} {cur_y-4} {max_w} 20 re f")
                cur_ops.append(f"BT /F3 9 Tf 0.4 0.4 0.4 rg {self.margin_x + 8} {cur_y} Td ({safe}) Tj ET")
                cur_y -= 24.0
            elif kind == "SEP":
                cur_ops.append(f"0.85 0.85 0.85 RG 0.5 w {self.margin_x} {cur_y} m {self.page_w - self.margin_x} {cur_y} l S")
                cur_y -= 14.0
            elif kind == "SPACE":
                cur_y -= 10.0
            elif kind == "P":
                # Partir párrafos largos en líneas de ~80 caracteres
                words = text.split(" ")
                cur_line = []
                for w in words:
                    cur_line.append(w)
                    if len(" ".join(cur_line)) > 80:
                        safe = self.escape_pdf_str(" ".join(cur_line))
                        cur_ops.append(f"BT /F1 10.5 Tf 0.15 0.15 0.15 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                        cur_y -= 14.0
                        cur_line = []
                        if cur_y < self.margin_y + 30.0:
                            new_page()
                if cur_line:
                    safe = self.escape_pdf_str(" ".join(cur_line))
                    cur_ops.append(f"BT /F1 10.5 Tf 0.15 0.15 0.15 rg {self.margin_x} {cur_y} Td ({safe}) Tj ET")
                    cur_y -= 16.0

        if cur_ops:
            pdf_pages.append("\n".join(cur_ops))

        if not pdf_pages:
            pdf_pages.append(f"BT /F1 12 Tf {self.margin_x} {self.page_h/2} Td (Documento vacio) Tj ET")

        # Escribir estructura PDF 1.4
        total_p = len(pdf_pages)
        out = open(out_path, "wb")
        out.write(b"%PDF-1.4\n")

        offsets = []

        def write_obj(content):
            offsets.append(out.tell())
            obj_num = len(offsets)
            out.write(f"{obj_num} 0 obj\n".encode('ascii'))
            if isinstance(content, str):
                out.write(content.encode('latin1', errors='replace'))
            else:
                out.write(content)
            out.write(b"\nendobj\n")
            return obj_num

        # Obj 1: Catalog
        write_obj("<< /Type /Catalog /Pages 2 0 R >>")

        # Obj 2: Pages tree placeholder
        pages_obj_num = 2
        offsets.append(out.tell())
        out.write(f"2 0 obj\n<< /Type /Pages /Kids [] /Count {total_p} >>\nendobj\n".encode('ascii'))

        # Obj 3, 4, 5: Fuentes estándar con WinAnsiEncoding
        write_obj("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica /Encoding /WinAnsiEncoding >>")
        write_obj("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold /Encoding /WinAnsiEncoding >>")
        write_obj("<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Oblique /Encoding /WinAnsiEncoding >>")

        page_refs = []
        for p_i, p_stream in enumerate(pdf_pages):
            # Pie de página con número
            footer = f"\nBT /F1 9 Tf 0.6 0.6 0.6 rg {self.page_w/2 - 20} {self.margin_y/2} Td (Pagina {p_i+1} de {total_p}) Tj ET"
            full_stream = p_stream + footer
            s_bytes = full_stream.encode('latin1', errors='replace')

            # Stream object
            st_num = write_obj(f"<< /Length {len(s_bytes)} >>\nstream\n".encode('ascii') + s_bytes + b"\nendstream")

            # Page object
            p_obj = write_obj(f"<< /Type /Page /Parent 2 0 R /MediaBox [0 0 {self.page_w} {self.page_h}] "
                              f"/Resources << /Font << /F1 3 0 R /F2 4 0 R /F3 5 0 R >> >> "
                              f"/Contents {st_num} 0 R >>")
            page_refs.append(f"{p_obj} 0 R")

        # Re-escribir Pages tree (Obj 2)
        kids_str = " ".join(page_refs)
        pages_content = f"<< /Type /Pages /Kids [{kids_str}] /Count {total_p} >>"
        out.seek(offsets[1])
        out.write(f"2 0 obj\n{pages_content}\nendobj\n".encode('ascii'))
        out.seek(0, os.SEEK_END)

        # XREF table
        xref_pos = out.tell()
        out.write(f"xref\n0 {len(offsets)+1}\n".encode('ascii'))
        out.write(b"0000000000 65535 f \n")
        for off in offsets:
            out.write(f"{off:010d} 00000 n \n".encode('ascii'))

        trailer = f"trailer\n<< /Size {len(offsets)+1} /Root 1 0 R >>\nstartxref\n{xref_pos}\n%%EOF\n"
        out.write(trailer.encode('ascii'))
        out.close()
        return True


# ==============================================================================
# Main CLI
# ==============================================================================

def main():
    if len(sys.argv) < 4:
        print("Uso: exporter.py <input.json> <output_file> <format: epub|docx|pdf|md>")
        sys.exit(1)

    json_path = sys.argv[1]
    out_path = sys.argv[2]
    fmt = sys.argv[3].lower()

    if not os.path.exists(json_path):
        print(f"Error: archivo no encontrado {json_path}")
        sys.exit(1)

    with open(json_path, 'r', encoding='utf-8') as f:
        doc_data = json.load(f)

    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)

    success = False
    if fmt == "epub":
        success = export_epub(doc_data, out_path)
    elif fmt == "docx":
        success = export_docx(doc_data, out_path)
    elif fmt == "pdf":
        success = SimplePDFBuilder().build(doc_data, out_path)
    elif fmt == "md":
        # Exportación simple en Markdown
        with open(out_path, 'w', encoding='utf-8') as f:
            f.write(f"# {doc_data.get('title')}\n\n")
            for ch in doc_data.get('chapters', []):
                f.write(f"## {ch.get('title')}\n\n")
                for pg in ch.get('pages', []):
                    f.write(f"### {pg.get('title')}\n\n{pg.get('content')}\n\n")
        success = True
    else:
        print(f"Formato no reconocido: {fmt}")
        sys.exit(1)

    if success:
        print(f"EXITO: Exportado a {out_path}")
        sys.exit(0)
    else:
        print("Error en la exportación.")
        sys.exit(1)

if __name__ == "__main__":
    main()
