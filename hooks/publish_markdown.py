"""Publish each page's Markdown source next to its rendered HTML.

A page built at `labs/lab01-pwm-motors/index.html` also gets
`labs/lab01-pwm-motors/index.md`, so the page-actions menu can fetch, copy,
open, or download the real source rather than trying to reconstruct Markdown
from the DOM.
"""

from pathlib import Path


def on_post_build(config, **kwargs):
    docs = Path(config["docs_dir"])
    site = Path(config["site_dir"])

    exclude = set()
    for pattern in (config.get("exclude_docs") or "").split("\n"):
        pattern = pattern.strip()
        if pattern:
            exclude.add(pattern)

    published = 0
    for source in sorted(docs.rglob("*.md")):
        rel = source.relative_to(docs)
        if rel.as_posix() in exclude:
            continue

        # Mirror MkDocs' directory URLs: foo.md -> foo/index.md,
        # and an existing index.md stays where it is.
        parent = rel.parent if rel.name == "index.md" else rel.parent / rel.stem
        target = site / parent / "index.md"

        target.parent.mkdir(parents=True, exist_ok=True)
        target.write_text(source.read_text(encoding="utf-8"), encoding="utf-8")
        published += 1

    print(f"INFO    -  Published {published} Markdown sources alongside the HTML")
