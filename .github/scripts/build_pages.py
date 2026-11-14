"""Actualiza una versión del visor sin borrar la publicada por la otra rama."""

import argparse
from datetime import datetime, timezone
from html import escape
from pathlib import Path
import shutil
from zoneinfo import ZoneInfo


def render_page(branch: str, sha: str, repository: str, *, pending=False,
                updated_at: datetime | None = None) -> str:
    stable = branch == "main"
    title = "(main)" if stable else "(dev)"
    other_url = "dev/" if stable else "../"
    other_label = "Ver (dev)" if stable else "Ver (main)"
    repo_url = f"https://github.com/{escape(repository, quote=True)}"
    pdf_url = f"McDuck-Handbook.pdf?v={escape(sha, quote=True)}"
    release = (
        f'<a href="{repo_url}/releases/latest">Última release</a>' if stable else ""
    )
    updated = ""
    if not pending:
        published = (updated_at or datetime.now(timezone.utc)).astimezone(
            ZoneInfo("America/Argentina/Cordoba"))
        updated = (
            '<p class="updated-at"><strong>Última actualización<br>'
            f'<time datetime="{published.isoformat()}">'
            f'{published:%d/%m/%Y %H:%M}</time> (Argentina)</strong></p>'
        )
    content = (
        '<p>El PDF de (main) estará disponible después de la primera publicación desde esa rama.</p>'
        if pending
        else f"""<p>Commit <a href="{repo_url}/commit/{escape(sha, quote=True)}">{escape(sha[:7])}</a></p>
    <p><a href="{pdf_url}" download>Descargar PDF</a> ·
       <a href="{pdf_url}">Abrir PDF</a></p>
    <iframe src="{pdf_url}" title="McDuck ICPC Handbook — {title}"></iframe>"""
    )
    return f"""<!doctype html>
<html lang="es">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>McDuck ICPC Handbook — {title}</title>
  <style>
    body {{ margin: 0; font: 16px system-ui, sans-serif; background: #f3f4f6; color: #172033; }}
    header, main {{ max-width: 1200px; margin: auto; padding: 16px 24px; }}
    .header-row {{ display: flex; justify-content: space-between; align-items: flex-start; gap: 16px; flex-wrap: wrap; }}
    h1 {{ margin: 0 0 8px; font-size: 1.5rem; }}
    .updated-at {{ margin: 0 0 8px auto; text-align: right; font-size: 0.9rem; }}
    nav {{ display: flex; flex-wrap: wrap; gap: 16px; }}
    a {{ color: #1457b5; }}
    iframe {{ width: 100%; height: 80vh; border: 1px solid #cbd5e1; background: white; }}
  </style>
</head>
<body>
  <header>
    <div class="header-row">
      <h1>McDuck ICPC Handbook — {title}</h1>
      {updated}
    </div>
    <nav><a href="{other_url}">{other_label}</a>{release}</nav>
  </header>
  <main>{content}</main>
</body>
</html>
"""


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--site", type=Path, required=True)
    parser.add_argument("--pdf", type=Path, required=True)
    parser.add_argument("--branch", choices=("main", "dev"), required=True)
    parser.add_argument("--sha", required=True)
    parser.add_argument("--repository", required=True)
    args = parser.parse_args()

    target = args.site if args.branch == "main" else args.site / "dev"
    target.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(args.pdf, target / "McDuck-Handbook.pdf")
    (target / "index.html").write_text(
        render_page(args.branch, args.sha, args.repository), encoding="utf-8"
    )
    (args.site / ".nojekyll").touch()
    if not (args.site / "index.html").exists():
        (args.site / "index.html").write_text(
            render_page("main", args.sha, args.repository, pending=True), encoding="utf-8"
        )


if __name__ == "__main__":
    main()
