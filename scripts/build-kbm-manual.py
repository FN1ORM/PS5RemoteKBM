#!/usr/bin/env python3
"""Render our restricted, trusted Markdown manual to standalone offline HTML.

No packages, fonts, scripts, network requests or credentials are needed to read it.
The supported syntax is deliberately limited to the constructs used in this file.
"""
from pathlib import Path
import html
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "docs/kbm/user-manual.md"
DESTINATION = SOURCE.with_suffix(".html")


def inline(text):
    saved = []

    def keep(markup):
        saved.append(markup)
        return f"\x00{len(saved)-1}\x00"

    # Escape source text before introducing generated markup.
    text = html.escape(text)
    text = re.sub(r"`([^`]+)`", lambda m: keep(f"<code>{m[1]}</code>"), text)
    text = re.sub(r"\[([^\]]+)\]\(([^)]+)\)", lambda m: keep(f'<a href="{m[2]}">{m[1]}</a>'), text)
    text = re.sub(r"\*\*([^*]+)\*\*", r"<strong>\1</strong>", text)
    return re.sub(r"\x00(\d+)\x00", lambda m: saved[int(m[1])], text)


def render(source):
    lines = source.splitlines()
    out, toc, paragraph = [], [], []
    index = 0

    def flush():
        if paragraph:
            out.append("<p>" + inline(" ".join(paragraph)) + "</p>")
            paragraph.clear()

    while index < len(lines):
        line = lines[index]
        if line.startswith("```"):
            flush()
            code = []
            index += 1
            while index < len(lines) and not lines[index].startswith("```"):
                code.append(lines[index])
                index += 1
            out.append("<pre><code>" + html.escape("\n".join(code)) + "</code></pre>")
        elif line.startswith("#"):
            flush()
            heading = re.match(r"(#+) (.*)", line)
            level, title = len(heading[1]), heading[2]
            slug = re.sub(r"[^a-z0-9]+", "-", title.lower()).strip("-")
            out.append(f'<h{level} id="{slug}">{inline(title)}</h{level}>')
            if level == 2:
                toc.append(f'<a href="#{slug}">{html.escape(title)}</a>')
        elif line.startswith("|"):
            flush()
            rows = []
            while index < len(lines) and lines[index].startswith("|"):
                cells = [v.strip() for v in lines[index].strip().strip("|").split("|")]
                if not all(re.fullmatch(r":?-+:?", cell) for cell in cells):
                    tag = "th" if not rows else "td"
                    rows.append("<tr>" + "".join(f"<{tag}>{inline(cell)}</{tag}>" for cell in cells) + "</tr>")
                index += 1
            out.append('<div class="table"><table>' + "".join(rows) + "</table></div>")
            continue
        elif re.match(r"\d+\. ", line) or line.startswith("- "):
            flush()
            ordered = bool(re.match(r"\d+\. ", line))
            pattern = r"\d+\. (.*)" if ordered else r"- (.*)"
            items = []
            while index < len(lines) and (match := re.fullmatch(pattern, lines[index])):
                items.append("<li>" + inline(match[1]) + "</li>")
                index += 1
            tag = "ol" if ordered else "ul"
            out.append(f"<{tag}>" + "".join(items) + f"</{tag}>")
            continue
        elif not line.strip():
            flush()
        else:
            paragraph.append(line)
        index += 1
    flush()
    return "\n".join(out), "\n".join(toc)


if __name__ == "__main__":
    body, navigation = render(SOURCE.read_text(encoding="utf-8"))
    page = """<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>PS5 Remote KBM — User Manual</title>
<style>
:root{color-scheme:dark;--bg:#0c1320;--panel:#142238;--text:#e5edf7;--muted:#a9bdd4;--line:#30445d;--accent:#6bc4ff}
*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:16px/1.65 system-ui,Segoe UI,sans-serif}
a{color:var(--accent);text-underline-offset:3px}header{padding:22px 32px;border-bottom:1px solid var(--line);background:var(--panel)}
header strong{font-size:22px}header p{margin:4px 0;color:var(--muted)}.layout{display:grid;grid-template-columns:300px minmax(0,1fr);max-width:1520px;margin:auto}
nav{position:sticky;top:0;height:100vh;overflow:auto;padding:24px 22px;border-right:1px solid var(--line)}nav a{display:block;padding:6px 0;font-size:14px;text-decoration:none}nav a:hover{text-decoration:underline}
main{padding:20px 42px 70px;max-width:1100px;min-width:0}h1{font-size:34px;line-height:1.2}h2{font-size:24px;line-height:1.35;margin-top:52px;padding-top:20px;border-top:1px solid var(--line);scroll-margin-top:16px}
p,li{overflow-wrap:anywhere}li{padding-left:5px;margin:9px 0}strong{color:#fff}code{font:14px/1.6 Consolas,monospace;background:var(--panel);padding:2px 5px;border-radius:4px}
pre{padding:18px;background:var(--panel);border:1px solid var(--line);border-radius:10px;overflow:auto}pre code{padding:0}.table{overflow:auto;border:1px solid var(--line);border-radius:8px;margin:22px 0}
table{border-collapse:collapse;width:100%;min-width:430px;font-size:15px}th,td{padding:12px 14px;border-bottom:1px solid var(--line);text-align:left;vertical-align:top}th{background:var(--panel)}tr:last-child td{border-bottom:0}
.back{position:fixed;right:16px;bottom:12px;background:var(--panel);padding:7px 12px;border:1px solid var(--line);border-radius:8px;font-size:13px}
@media(max-width:900px){.layout{display:block}nav{position:static;height:auto;border-bottom:1px solid var(--line);border-right:0}nav a{display:inline-block;margin-right:16px}main{padding:12px 22px 60px}h1{font-size:28px}header{padding:20px 22px}}
@media print{body{color:#111;background:#fff;font-size:11pt}header,nav,.back{display:none}.layout{display:block}main{padding:0;max-width:none}h1,h2,strong{color:#111}h2{break-after:avoid}pre,code,th{background:#f0f3f6;color:#111}a{color:#164974}table{min-width:0}tr,pre{break-inside:avoid}}
</style></head><body id="top"><header><strong>PS5 Remote KBM</strong><p>Offline user manual · F3 in the app · Find with Ctrl+F · Print or save as PDF from your browser</p></header>
<div class="layout"><nav aria-label="Contents"><strong>Contents</strong>\nNAVIGATION\n</nav><main>BODY</main></div>
<a class="back" href="#top">Back to top ↑</a></body></html>
""".replace("NAVIGATION", navigation).replace("BODY", body)
    DESTINATION.write_text(page, encoding="utf-8")
    print(f"Built {DESTINATION.name}: {len(navigation.splitlines())} sections, {len(page):,} characters")
