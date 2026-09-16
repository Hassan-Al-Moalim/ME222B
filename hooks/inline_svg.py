"""Inline an SVG file into a page.

Write `{{ svg assets/name.svg }}` on its own line. Inlining rather than linking
is what lets the drawings use the site's CSS variables, so they follow the
light/dark toggle.

The substitution happens *after* Markdown conversion, not before. Markdown's
block parser mangles raw SVG: it closes the tag at the first blank line, wraps
element groups in <p>, and — because SVG is conventionally indented — turns the
body into a code block. Injecting into the rendered HTML sidesteps all of that.

Paths resolve against `docs_dir`, which MkDocs gives us absolutely, so nothing
depends on the working directory mkdocs was launched from.
"""

import re
from pathlib import Path

# Markdown wraps a lone marker in a paragraph; match it with or without.
MARKER = re.compile(
    r"(?:<p>\s*)?\{\{\s*svg\s+(?P<path>[^\s}]+)\s*\}\}(?:\s*</p>)?"
)


def on_page_content(html, page, config, files, **kwargs):
    if "{{" not in html:
        return html

    docs = Path(config["docs_dir"]).resolve()

    def replace(match):
        rel = match.group("path")
        target = (docs / rel).resolve()

        if docs not in target.parents:
            raise ValueError(f"{page.file.src_path}: svg path escapes docs_dir: {rel}")
        if not target.is_file():
            raise FileNotFoundError(f"{page.file.src_path}: no such svg: {rel}")

        return target.read_text(encoding="utf-8").strip()

    return MARKER.sub(replace, html)
