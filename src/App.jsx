import React from 'react';

export default function App() {
  const GITHUB_URL = "https://github.com/TheVixcho/WitcherNotes";
  const RELEASES_URL = "https://github.com/TheVixcho/WitcherNotes/releases/latest";

  return (
    <div className="page-wrapper">
      {/* NAVBAR */}
      <header className="navbar">
        <div className="container nav-content">
          <div className="brand">
            <img src="/witchernotes.png" alt="WitcherNotes Logo" />
            <span>WitcherNotes</span>
            <span className="brand-badge">Open Source</span>
          </div>

          <nav className="nav-links">
            <a href="#caracteristicas">Características</a>
            <a href="#previsualizacion">Espacio de Trabajo</a>
            <a href="#opensource">Código Abierto</a>
            <a href="#faq">Preguntas Frecuentes</a>
          </nav>

          <div className="nav-cta">
            <a href={GITHUB_URL} target="_blank" rel="noreferrer" className="btn btn-secondary" style={{ padding: '8px 16px', fontSize: '0.85rem' }}>
              ⭐ GitHub
            </a>
            <a href="#descargar" className="btn btn-primary" style={{ padding: '8px 16px', fontSize: '0.85rem' }}>
              📥 Descargar
            </a>
          </div>
        </div>
      </header>

      {/* HERO SECTION */}
      <section className="hero">
        <div className="container">
          <div className="hero-pill">
            <span>✨ 100% Gratuito, Libre y de Código Abierto</span>
          </div>

          <h1 className="hero-title">
            El santuario definitivo para tus <br />
            <span className="gradient-text">notas, cuadernos y lectura.</span>
          </h1>

          <p className="hero-desc">
            Organiza tus ideas en cuadernos independientes con estructura de hojas, escribe sin distracciones
            en Markdown y disfruta de un modo de lectura en dos páginas con exportación a PDF, EPUB, DOCX y Markdown.
            Sin cuentas obligatorias, sin planes de pago y sin telemetría intrusiva.
          </p>

          <div className="hero-actions" id="descargar">
            <a 
              href={RELEASES_URL} 
              target="_blank" 
              rel="noreferrer" 
              className="btn btn-primary"
            >
              <span>🐧 Descargar para Linux (.AppImage)</span>
            </a>

            <a 
              href={RELEASES_URL} 
              target="_blank" 
              rel="noreferrer" 
              className="btn btn-secondary"
            >
              <span>🪟 Descargar para Windows (.exe)</span>
            </a>

            <a 
              href={GITHUB_URL} 
              target="_blank" 
              rel="noreferrer" 
              className="btn btn-secondary"
            >
              <span>⭐ Ver en GitHub (Licencia MIT)</span>
            </a>
          </div>

          <div className="hero-platforms">
            <span>✓ C++17 nativo y Dear ImGui</span>
            <span>•</span>
            <span>✓ Ultra ligero (~10 MB)</span>
            <span>•</span>
            <span>✓ 100% Offline & Sin Cuentas</span>
            <span>•</span>
            <span>✓ Licencia Libre MIT</span>
          </div>

          {/* MOCKUP INTERACTIVO DEL ESPACIO DE TRABAJO */}
          <div className="app-preview" id="previsualizacion">
            <div className="mockup-header">
              <div className="mockup-dot dot-red"></div>
              <div className="mockup-dot dot-yellow"></div>
              <div className="mockup-dot dot-green"></div>
              <span className="mockup-title">WitcherNotes — Crónicas del Continente (Libro Activo)</span>
            </div>

            <div className="mockup-body">
              {/* Columna 1: Hojas */}
              <div className="mockup-col" style={{ textAlign: 'left', background: 'rgba(0,0,0,0.2)' }}>
                <div style={{ fontSize: '0.75rem', fontWeight: 700, color: '#f59e0b', textTransform: 'uppercase', marginBottom: '14px', letterSpacing: '0.05em' }}>
                  📄 Hojas del Documento
                </div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '6px', fontSize: '0.85rem' }}>
                  <div style={{ padding: '8px 10px', background: 'rgba(245, 158, 11, 0.15)', borderRadius: '6px', borderLeft: '3px solid #f59e0b', color: '#fff', fontWeight: 600 }}>
                    1. Introducción y Prólogo
                  </div>
                  <div style={{ padding: '8px 10px', color: '#94a3b8', borderRadius: '6px' }}>
                    2. Bestiario de las Ciénagas
                  </div>
                  <div style={{ padding: '8px 10px', color: '#94a3b8', borderRadius: '6px' }}>
                    3. Alquimia y Pociones
                  </div>
                  <div style={{ padding: '8px 10px', color: '#94a3b8', borderRadius: '6px' }}>
                    4. Tratados de Novigrado
                  </div>
                </div>
              </div>

              {/* Columna 2: Editor */}
              <div className="mockup-col" style={{ textAlign: 'left', padding: '24px 32px' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', borderBottom: '1px solid rgba(255,255,255,0.08)', paddingBottom: '12px', marginBottom: '16px' }}>
                  <span style={{ fontSize: '0.9rem', color: '#94a3b8', fontFamily: 'monospace' }}>Hoja 1 de 4 — Guardado automáticamente</span>
                  <span style={{ fontSize: '0.8rem', color: '#10b981', background: 'rgba(16, 185, 129, 0.1)', padding: '2px 8px', borderRadius: '4px' }}>● 100% Local y Privado</span>
                </div>
                <h2 style={{ fontSize: '1.6rem', color: '#fff', marginBottom: '12px', fontWeight: 700 }}>
                  Capítulo I: El Silencio del Bosque
                </h2>
                <p style={{ color: '#cbd5e1', fontSize: '0.95rem', lineHeight: '1.7', marginBottom: '14px' }}>
                  El sendero se extendía bajo la niebla densa del amanecer. Las hojas crujían bajo las botas gastadas mientras el viento mecía las copas de los robles antiguos. En este cuaderno quedan registradas las vivencias, notas al margen y observaciones recopiladas en cada viaje.
                </p>
                <div style={{ padding: '12px 16px', background: 'rgba(255,255,255,0.03)', borderLeft: '3px solid #8b5cf6', borderRadius: '4px', fontStyle: 'italic', color: '#a78bfa', fontSize: '0.9rem' }}>
                  "Un buen cuaderno no es solo papel y tinta; es la memoria que desafía al olvido."
                </div>
              </div>

              {/* Columna 3: Notas y Marcadores */}
              <div className="mockup-col" style={{ textAlign: 'left', background: 'rgba(0,0,0,0.15)' }}>
                <div style={{ fontSize: '0.75rem', fontWeight: 700, color: '#94a3b8', textTransform: 'uppercase', marginBottom: '14px' }}>
                  📌 Marcadores & Notas
                </div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
                  <div style={{ padding: '10px', background: 'rgba(255,255,255,0.04)', borderRadius: '6px', border: '1px solid rgba(255,255,255,0.06)' }}>
                    <div style={{ fontSize: '0.8rem', fontWeight: 600, color: '#f59e0b' }}>🔖 Recordatorio de Investigación</div>
                    <div style={{ fontSize: '0.75rem', color: '#94a3b8', marginTop: '4px' }}>Revisar referencias bibliográficas del tomo antiguo antes de exportar a EPUB.</div>
                  </div>
                  <div style={{ padding: '10px', background: 'rgba(255,255,255,0.04)', borderRadius: '6px', border: '1px solid rgba(255,255,255,0.06)' }}>
                    <div style={{ fontSize: '0.8rem', fontWeight: 600, color: '#8b5cf6' }}>★ Cita Destacada</div>
                    <div style={{ fontSize: '0.75rem', color: '#94a3b8', marginTop: '4px' }}>Incluir en la introducción del documento PDF.</div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* CARACTERÍSTICAS */}
      <section className="features-section" id="caracteristicas">
        <div className="container">
          <div className="section-tag">Diseñado para la concentración</div>
          <h2 className="section-title">Todo lo que necesitas para escribir y organizar</h2>
          <p className="section-subtitle">
            Combina la fluidez del formato Markdown con la estructura tradicional de un cuaderno de hojas y las comodidades de un lector moderno.
          </p>

          <div className="features-grid">
            <div className="glass-panel feature-card">
              <div className="feature-icon">📚</div>
              <h3 className="feature-title">Biblioteca de Cuadernos</h3>
              <p className="feature-text">
                Crea múltiples libros para distintos proyectos. Cada cuaderno tiene sus propias secciones y hojas ordenadas cronológicamente o por temática.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <div className="feature-icon">✍️</div>
              <h3 className="feature-title">Editor Markdown Puro</h3>
              <p className="feature-text">
                Escribe con fluidez sin botones intrusivos. Soporta formato completo, listas, bloques de código, tablas y citas con guardado automático periódico.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <div className="feature-icon">📖</div>
              <h3 className="feature-title">Lectura en Dos Páginas</h3>
              <p className="feature-text">
                Cambia entre visualización tipo libro abierto con sombra de encuadernación, vista de página única centrada o desplazamiento continuo.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <div className="feature-icon">📄</div>
              <h3 className="feature-title">Exportación Multiformato Libre</h3>
              <p className="feature-text">
                Convierte tus hojas o cuadernos completos a <strong>PDF editorial, Microsoft Word (.docx), EPUB para lectores digitales</strong> o Markdown plano sin costo ni restricciones.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <div className="feature-icon">🎨</div>
              <h3 className="feature-title">6 Temas de Color Incluidos</h3>
              <p className="feature-text">
                Modo Oscuro Moderno, Papel Clásico, Sepia Cálido, Obsidiana Púrpura y Zafiro Medianoche, pensados para no fatigar la vista en sesiones prolongadas.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <div className="feature-icon">🚀</div>
              <h3 className="feature-title">Auto-Actualizaciones y Privacidad</h3>
              <p className="feature-text">
                Mantén tu app al día directamente desde GitHub Releases. Tus notas son tuyas: se almacenan localmente en tu disco duro en formato abierto.
              </p>
            </div>
          </div>
        </div>
      </section>

      {/* CÓDIGO ABIERTO & LIBERTAD */}
      <section className="pricing-section" id="opensource">
        <div className="container">
          <div className="section-tag">Software Libre y Abierto</div>
          <h2 className="section-title">Completamente Libre. Sin Paywalls.</h2>
          <p className="section-subtitle">
            WitcherNotes es un proyecto de código abierto desarrollado para la comunidad. Sin suscripciones, sin cuotas mensuales y sin cuentas forzadas.
          </p>

          <div style={{ maxWidth: '750px', margin: '40px auto 0' }}>
            <div className="glass-panel pricing-card featured" style={{ textAlign: 'left', padding: '36px 40px' }}>
              <div className="pricing-badge">100% OPEN SOURCE</div>
              <h3 className="plan-name" style={{ color: '#f59e0b', fontSize: '1.8rem', marginBottom: '8px' }}>
                Edición Completa y Abierta
              </h3>
              <p className="plan-desc" style={{ fontSize: '1.05rem', marginBottom: '24px' }}>
                Todas las funciones están desbloqueadas de fábrica para cualquier usuario.
              </p>
              
              <div className="plan-price" style={{ color: '#10b981', fontSize: '2.5rem', marginBottom: '28px' }}>
                $0 <span>/ gratis para siempre (Licencia MIT)</span>
              </div>

              <ul className="plan-features" style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))', gap: '14px 20px', marginBottom: '32px' }}>
                <li className="active"><span className="check">✓</span> <strong>Cuadernos ilimitados</strong> en la biblioteca</li>
                <li className="active"><span className="check">✓</span> <strong>Exportación completa a PDF</strong> editorial</li>
                <li className="active"><span className="check">✓</span> <strong>Exportación a Microsoft Word (.docx)</strong></li>
                <li className="active"><span className="check">✓</span> <strong>Exportación a E-Book digital (.epub)</strong></li>
                <li className="active"><span className="check">✓</span> <strong>Exportación a Markdown (.md) y TXT</strong></li>
                <li className="active"><span className="check">✓</span> Todos los 6 temas visuales desbloqueados</li>
                <li className="active"><span className="check">✓</span> Modos de lectura en doble página y pergamino</li>
                <li className="active"><span className="check">✓</span> <strong>Sin cuentas ni registros obligatorios</strong></li>
                <li className="active"><span className="check">✓</span> Almacenamiento 100% local en tu disco duro</li>
                <li className="active"><span className="check">✓</span> Código abierto auditable y personalizable</li>
              </ul>

              <div style={{ display: 'flex', gap: '16px', flexWrap: 'wrap' }}>
                <a 
                  href="#descargar" 
                  className="btn btn-primary" 
                  style={{ flex: '1 1 200px', padding: '14px', textAlign: 'center' }}
                >
                  📥 Descargar WitcherNotes
                </a>
                <a 
                  href={GITHUB_URL} 
                  target="_blank" 
                  rel="noreferrer" 
                  className="btn btn-secondary" 
                  style={{ flex: '1 1 200px', padding: '14px', textAlign: 'center' }}
                >
                  ⭐ Ver Repositorio en GitHub
                </a>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* PREGUNTAS FRECUENTES */}
      <section className="faq-section" id="faq" style={{ padding: '60px 0 100px' }}>
        <div className="container" style={{ maxWidth: '800px' }}>
          <div className="section-tag">Dudas comunes</div>
          <h2 className="section-title">Preguntas Frecuentes</h2>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '16px', marginTop: '36px' }}>
            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>¿Es WitcherNotes realmente 100% gratis y de código abierto?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                ¡Sí! WitcherNotes es un proyecto de código abierto publicado bajo la licencia permisiva MIT. No hay versiones de pago, funciones bloqueadas ni muros de suscripción. Todas las herramientas de exportación (PDF, EPUB, DOCX), temas y cuadernos ilimitados están disponibles para todos los usuarios.
              </p>
            </div>

            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>¿Necesito crear una cuenta o registrarme para usar la aplicación?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                No. WitcherNotes no requiere creación de cuentas ni inicio de sesión obligatorio. Puedes instalarla y comenzar a escribir al instante. Todas tus notas se almacenan en tu propio equipo. Si lo deseas, puedes configurar de forma opcional tu propio proyecto de Firebase para sincronización en la nube.
              </p>
            </div>

            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>¿Puedo usar la aplicación sin conexión a internet?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                ¡Totalmente! Es una aplicación de escritorio nativa en C++ y Dear ImGui. Todos tus archivos, notas y libros se guardan localmente en tu ordenador y no dependen de servidores externos para funcionar.
              </p>
            </div>

            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>¿En qué sistemas operativos está disponible?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                Está disponible para Linux mediante formato universal portátil `.AppImage` y para Windows mediante ejecutable directo `.exe`.
              </p>
            </div>
          </div>
        </div>
      </section>

      {/* FOOTER */}
      <footer className="footer">
        <div className="container">
          <div className="footer-content">
            <div className="brand">
              <img src="/witchernotes.png" alt="WitcherNotes Logo" />
              <span>WitcherNotes</span>
            </div>

            <div className="footer-links">
              <a href="#caracteristicas">Características</a>
              <a href="#opensource">Código Abierto</a>
              <a href={GITHUB_URL} target="_blank" rel="noreferrer">GitHub</a>
              <a href={RELEASES_URL} target="_blank" rel="noreferrer">Releases</a>
            </div>
          </div>

          <div className="footer-bottom">
            © {new Date().getFullYear()} WitcherNotes. Software Libre y de Código Abierto bajo Licencia MIT. Desarrollado con C++ y Dear ImGui.
          </div>
        </div>
      </footer>
    </div>
  );
}
