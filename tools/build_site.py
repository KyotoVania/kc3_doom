import argparse
import html
from html.parser import HTMLParser
from pathlib import Path
import re
import shutil
from urllib.parse import quote, unquote, urlsplit


class Presentation(HTMLParser):
    def __init__(self):
        super().__init__()
        self.ids = set()
        self.links = []

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        if "id" in attrs:
            if attrs["id"] in self.ids:
                raise ValueError("Duplicate id: " + attrs["id"])
            self.ids.add(attrs["id"])
        if "href" in attrs:
            self.links.append(attrs["href"])
        if "src" in attrs and not attrs["src"].startswith("data:"):
            raise ValueError("Presentation must be self-contained")


def prepare(root, repository=None, revision=None):
    if bool(repository) != bool(revision):
        raise ValueError("Repository and revision must be supplied together")
    if repository and not re.fullmatch(r"[A-Za-z0-9_.-]+/[A-Za-z0-9_.-]+", repository):
        raise ValueError("Invalid GitHub repository")
    if revision and not re.fullmatch(r"[a-fA-F0-9]{40}", revision):
        raise ValueError("Revision must be a full commit SHA")
    root = root.resolve()
    source = (root / "presentation-code.html").read_text(encoding="utf-8")
    parser = Presentation()
    parser.feed(source)
    files = set()
    replacements = {}
    for link in parser.links:
        url = urlsplit(link)
        if url.scheme or url.netloc:
            if url.scheme != "https":
                raise ValueError("Unsupported external link: " + link)
            continue
        if not url.path:
            if url.fragment and unquote(url.fragment) not in parser.ids:
                raise ValueError("Missing anchor: " + link)
            continue
        path = Path(unquote(url.path))
        allowed = (
            path.as_posix() in {"README.md", "CONTRIBUTING.md", "HANDOFF.md", "AGENTS.md", "PLAN.md", "LICENSE", "NOTICE"}
            or (path.parts[0] == "src" and path.suffix in {".c", ".h"})
            or (path.parts[0] == "kc3" and path.suffix == ".kc3")
            or (path.parts[0] == "tasks" and path.suffix == ".md")
        )
        target = (root / path).resolve()
        if not allowed or ".." in path.parts or not target.is_relative_to(root):
            raise ValueError("Disallowed local link: " + link)
        if not target.is_file():
            raise ValueError("Missing local file: " + link)
        files.add(path)
        if repository:
            destination = f"https://github.com/{repository}/blob/{revision}/{quote(path.as_posix())}"
            if url.fragment:
                destination += "#" + url.fragment
            replacements[link] = destination
    source = re.sub(
        r'\bhref=([\"\x27])(.*?)\1',
        lambda match: 'href="' + html.escape(replacements.get(html.unescape(match[2]), html.unescape(match[2])), quote=True) + '"',
        source,
    )
    return source, files


def build(root, output, repository=None, revision=None):
    source, files = prepare(root, repository, revision)
    output.mkdir(parents=True, exist_ok=False)
    (output / "index.html").write_text(source, encoding="utf-8")
    (output / ".nojekyll").touch()
    for name in ("LICENSE", "NOTICE"):
        shutil.copyfile(root / name, output / name)
    if not repository:
        for path in sorted(files):
            target = output / path
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(root / path, target)
    print(f"Site ready: {output} ({len(files)} source links checked)")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path, default=Path("_site"))
    parser.add_argument("--repository")
    parser.add_argument("--revision")
    args = parser.parse_args()
    build(Path(__file__).resolve().parents[1], args.output, args.repository, args.revision)
