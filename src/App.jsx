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
            <a href="#features">Features</a>
            <a href="#workspace">Workspace</a>
            <a href="#opensource">Open Source</a>
            <a href="#faq">FAQ</a>
          </nav>

          <div className="nav-cta">
            <a href={GITHUB_URL} target="_blank" rel="noreferrer" className="btn btn-secondary" style={{ padding: '8px 16px', fontSize: '0.85rem' }}>
              GitHub
            </a>
            <a href="#download" className="btn btn-primary" style={{ padding: '8px 16px', fontSize: '0.85rem' }}>
              Download
            </a>
          </div>
        </div>
      </header>

      {/* HERO SECTION */}
      <section className="hero">
        <div className="container">
          <div className="hero-pill">
            <span>100% Free and Open Source under MIT License</span>
          </div>

          <h1 className="hero-title">
            The modern desktop workspace for <br />
            <span className="gradient-text">notes, documentation, and reading.</span>
          </h1>

          <p className="hero-desc">
            Organize your knowledge into structured notebooks, write distraction-free in Markdown,
            and enjoy an editorial dual-page reading mode with instant export to PDF, EPUB, DOCX, and Markdown.
            No accounts required, no subscriptions, and strictly offline-first.
          </p>

          <div className="hero-actions" id="download">
            <a 
              href={RELEASES_URL} 
              target="_blank" 
              rel="noreferrer" 
              className="btn btn-primary"
            >
              <span>Download for Linux (.AppImage)</span>
            </a>

            <a 
              href={RELEASES_URL} 
              target="_blank" 
              rel="noreferrer" 
              className="btn btn-secondary"
            >
              <span>Download for Windows (.exe)</span>
            </a>

            <a 
              href={GITHUB_URL} 
              target="_blank" 
              rel="noreferrer" 
              className="btn btn-secondary"
            >
              <span>View Source on GitHub</span>
            </a>
          </div>

          <div className="hero-platforms">
            <span>Native C++17 & Dear ImGui</span>
            <span>•</span>
            <span>Ultra lightweight (~10 MB)</span>
            <span>•</span>
            <span>100% Offline & Private</span>
            <span>•</span>
            <span>Permissive MIT License</span>
          </div>

          {/* WORKSPACE PREVIEW MOCKUP */}
          <div className="app-preview" id="workspace">
            <div className="mockup-header">
              <div className="mockup-dot dot-red"></div>
              <div className="mockup-dot dot-yellow"></div>
              <div className="mockup-dot dot-green"></div>
              <span className="mockup-title">WitcherNotes — Engineering & System Architecture</span>
            </div>

            <div className="mockup-body">
              {/* Column 1: Document Outline */}
              <div className="mockup-col" style={{ textAlign: 'left', background: 'rgba(0,0,0,0.2)' }}>
                <div style={{ fontSize: '0.75rem', fontWeight: 700, color: '#f59e0b', textTransform: 'uppercase', marginBottom: '14px', letterSpacing: '0.05em' }}>
                  Document Outline
                </div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '6px', fontSize: '0.85rem' }}>
                  <div style={{ padding: '8px 10px', background: 'rgba(245, 158, 11, 0.15)', borderRadius: '6px', borderLeft: '3px solid #f59e0b', color: '#fff', fontWeight: 600 }}>
                    1. Architecture Overview
                  </div>
                  <div style={{ padding: '8px 10px', color: '#94a3b8', borderRadius: '6px' }}>
                    2. Data Models & Serialization
                  </div>
                  <div style={{ padding: '8px 10px', color: '#94a3b8', borderRadius: '6px' }}>
                    3. Rendering Engine & ImGui
                  </div>
                  <div style={{ padding: '8px 10px', color: '#94a3b8', borderRadius: '6px' }}>
                    4. Export Pipeline
                  </div>
                </div>
              </div>

              {/* Column 2: Editor */}
              <div className="mockup-col" style={{ textAlign: 'left', padding: '24px 32px' }}>
                <div style={{ display: 'flex', justifyContent: 'space-between', borderBottom: '1px solid rgba(255,255,255,0.08)', paddingBottom: '12px', marginBottom: '16px' }}>
                  <span style={{ fontSize: '0.9rem', color: '#94a3b8', fontFamily: 'monospace' }}>Page 1 of 4 — Automatically saved</span>
                  <span style={{ fontSize: '0.8rem', color: '#10b981', background: 'rgba(16, 185, 129, 0.1)', padding: '2px 8px', borderRadius: '4px' }}>Local & Private</span>
                </div>
                <h2 style={{ fontSize: '1.6rem', color: '#fff', marginBottom: '12px', fontWeight: 700 }}>
                  Section 1: Architecture Overview
                </h2>
                <p style={{ color: '#cbd5e1', fontSize: '0.95rem', lineHeight: '1.7', marginBottom: '14px' }}>
                  WitcherNotes is engineered as a zero-latency native desktop environment. Built in modern C++17 with an OpenGL 3 render backend and Dear ImGui, it provides instantaneous startup times and zero web view runtime overhead.
                </p>
                <div style={{ padding: '12px 16px', background: 'rgba(255,255,255,0.03)', borderLeft: '3px solid #8b5cf6', borderRadius: '4px', fontStyle: 'italic', color: '#a78bfa', fontSize: '0.9rem' }}>
                  "Software should be dependable, offline-first, and respect the user's data autonomy."
                </div>
              </div>

              {/* Column 3: Bookmarks & Marginalia */}
              <div className="mockup-col" style={{ textAlign: 'left', background: 'rgba(0,0,0,0.15)' }}>
                <div style={{ fontSize: '0.75rem', fontWeight: 700, color: '#94a3b8', textTransform: 'uppercase', marginBottom: '14px' }}>
                  Bookmarks & Marginalia
                </div>
                <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
                  <div style={{ padding: '10px', background: 'rgba(255,255,255,0.04)', borderRadius: '6px', border: '1px solid rgba(255,255,255,0.06)' }}>
                    <div style={{ fontSize: '0.8rem', fontWeight: 600, color: '#f59e0b' }}>Key Reference</div>
                    <div style={{ fontSize: '0.75rem', color: '#94a3b8', marginTop: '4px' }}>Verify schema consistency before running batch PDF export.</div>
                  </div>
                  <div style={{ padding: '10px', background: 'rgba(255,255,255,0.04)', borderRadius: '6px', border: '1px solid rgba(255,255,255,0.06)' }}>
                    <div style={{ fontSize: '0.8rem', fontWeight: 600, color: '#8b5cf6' }}>Note</div>
                    <div style={{ fontSize: '0.75rem', color: '#94a3b8', marginTop: '4px' }}>Included in section documentation and EPUB export.</div>
                  </div>
                </div>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* FEATURES */}
      <section className="features-section" id="features">
        <div className="container">
          <div className="section-tag">Designed for Focus and Speed</div>
          <h2 className="section-title">Everything you need to write and document</h2>
          <p className="section-subtitle">
            Combines the speed of pure Markdown with the structured hierarchy of notebooks and the reading comfort of an e-reader.
          </p>

          <div className="features-grid">
            <div className="glass-panel feature-card">
              <h3 className="feature-title">Notebook Library</h3>
              <p className="feature-text">
                Manage multiple projects in dedicated notebooks. Each notebook supports structured sections and pages organized chronologically or by topic.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <h3 className="feature-title">Distraction-Free Markdown</h3>
              <p className="feature-text">
                Write cleanly without clunky toolbars. Supports full Markdown formatting, bullet lists, code blocks, tables, and blockquotes with periodic auto-save.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <h3 className="feature-title">Dual-Page Reading Mode</h3>
              <p className="feature-text">
                Switch seamlessly between an editorial two-page book spread with spine shadows, a centered single-page layout, or continuous scrolling.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <h3 className="feature-title">Multi-Format Publishing</h3>
              <p className="feature-text">
                Export single pages or entire notebooks into <strong>clean PDF documents, Microsoft Word (.docx), EPUB e-books</strong>, or portable Markdown files.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <h3 className="feature-title">6 Curated Themes</h3>
              <p className="feature-text">
                Includes Modern Dark, Classic Paper, Warm Sepia, Midnight Purple, Sapphire Blue, and Minimal Light designed to minimize eye strain during long sessions.
              </p>
            </div>

            <div className="glass-panel feature-card">
              <h3 className="feature-title">Local-First Privacy</h3>
              <p className="feature-text">
                Your notes belong strictly to you. All data is saved on your local storage in open JSON format, completely independent of cloud servers.
              </p>
            </div>
          </div>
        </div>
      </section>

      {/* OPEN SOURCE SECTION */}
      <section className="pricing-section" id="opensource">
        <div className="container">
          <div className="section-tag">Free & Open Source</div>
          <h2 className="section-title">Completely Free. Zero Paywalls.</h2>
          <p className="section-subtitle">
            WitcherNotes is an open-source project built for the developer and writer community. No monthly fees, no feature tiers, and no forced logins.
          </p>

          <div style={{ maxWidth: '750px', margin: '40px auto 0' }}>
            <div className="glass-panel pricing-card featured" style={{ textAlign: 'left', padding: '36px 40px' }}>
              <div className="pricing-badge">100% OPEN SOURCE</div>
              <h3 className="plan-name" style={{ color: '#f59e0b', fontSize: '1.8rem', marginBottom: '8px' }}>
                Full & Unrestricted Edition
              </h3>
              <p className="plan-desc" style={{ fontSize: '1.05rem', marginBottom: '24px' }}>
                Every capability is included out of the box for every user.
              </p>
              
              <div className="plan-price" style={{ color: '#10b981', fontSize: '2.5rem', marginBottom: '28px' }}>
                $0 <span>/ free forever (MIT License)</span>
              </div>

              <ul className="plan-features" style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(260px, 1fr))', gap: '14px 20px', marginBottom: '32px' }}>
                <li className="active"><span className="check">✓</span> <strong>Unlimited notebooks</strong> in library</li>
                <li className="active"><span className="check">✓</span> <strong>Editorial PDF export</strong></li>
                <li className="active"><span className="check">✓</span> <strong>Microsoft Word (.docx) export</strong></li>
                <li className="active"><span className="check">✓</span> <strong>Digital e-book (.epub) export</strong></li>
                <li className="active"><span className="check">✓</span> <strong>Markdown (.md) and plain text export</strong></li>
                <li className="active"><span className="check">✓</span> All 6 color themes unlocked</li>
                <li className="active"><span className="check">✓</span> Dual-page book spread & continuous scroll</li>
                <li className="active"><span className="check">✓</span> <strong>No accounts or sign-ups required</strong></li>
                <li className="active"><span className="check">✓</span> 100% local storage on your machine</li>
                <li className="active"><span className="check">✓</span> Auditable and customizable open source code</li>
              </ul>

              <div style={{ display: 'flex', gap: '16px', flexWrap: 'wrap' }}>
                <a 
                  href="#download" 
                  className="btn btn-primary" 
                  style={{ flex: '1 1 200px', padding: '14px', textAlign: 'center' }}
                >
                  Download WitcherNotes
                </a>
                <a 
                  href={GITHUB_URL} 
                  target="_blank" 
                  rel="noreferrer" 
                  className="btn btn-secondary" 
                  style={{ flex: '1 1 200px', padding: '14px', textAlign: 'center' }}
                >
                  View GitHub Repository
                </a>
              </div>
            </div>
          </div>
        </div>
      </section>

      {/* FAQ SECTION */}
      <section className="faq-section" id="faq" style={{ padding: '60px 0 100px' }}>
        <div className="container" style={{ maxWidth: '800px' }}>
          <div className="section-tag">Common Questions</div>
          <h2 className="section-title">Frequently Asked Questions</h2>

          <div style={{ display: 'flex', flexDirection: 'column', gap: '16px', marginTop: '36px' }}>
            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>Is WitcherNotes truly 100% free and open source?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                Yes. WitcherNotes is an open-source project published under the permissive MIT License. There are no paid tiers, no locked features, and no subscription models. Full export capabilities (PDF, EPUB, DOCX), themes, and unlimited notebooks are available to everyone.
              </p>
            </div>

            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>Do I need to create an account or sign in?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                No. WitcherNotes does not require any account creation or login. You can download and immediately start writing. All your documents and notes stay on your local disk.
              </p>
            </div>

            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>Can I use the app completely offline?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                Yes, WitcherNotes is built as a native C++ and Dear ImGui desktop application. All files, notebooks, and configuration data reside locally on your machine and never require an internet connection to operate.
              </p>
            </div>

            <div className="glass-panel" style={{ padding: '24px' }}>
              <h4 style={{ fontSize: '1.1rem', marginBottom: '8px', color: '#fff' }}>What operating systems are supported?</h4>
              <p style={{ color: '#94a3b8', fontSize: '0.95rem' }}>
                WitcherNotes is available for Linux as a portable standalone `.AppImage` and for Windows as a native `.exe` binary.
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
              <a href="#features">Features</a>
              <a href="#opensource">Open Source</a>
              <a href={GITHUB_URL} target="_blank" rel="noreferrer">GitHub</a>
              <a href={RELEASES_URL} target="_blank" rel="noreferrer">Releases</a>
            </div>
          </div>

          <div className="footer-bottom">
            © {new Date().getFullYear()} WitcherNotes. Free and Open Source under the MIT License. Built with C++17 and Dear ImGui.
          </div>
        </div>
      </footer>
    </div>
  );
}
