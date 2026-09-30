from pathlib import Path
import sys
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / "tools"))
from build_site import build, prepare


class SiteTest(unittest.TestCase):
    def test_branch_publication(self):
        with tempfile.TemporaryDirectory() as directory:
            base = Path(directory)
            remote = base / "remote.git"
            repo = base / "repo"
            repo.mkdir()

            def git(*args):
                return subprocess.check_output(["git", *args], cwd=repo, stderr=subprocess.PIPE).decode().strip()

            git("init", "--bare", str(remote))
            git("init", "-b", "master")
            git("config", "user.name", "Site test")
            git("config", "user.email", "test@example.invalid")
            (repo / "game.txt").write_text("game source")
            git("add", "game.txt")
            git("commit", "-m", "initial")
            git("remote", "add", "origin", str(remote))
            git("push", "origin", "master")
            original = git("rev-parse", "HEAD")
            site = base / "site"
            build(ROOT, site, "example/test", original)

            def publish():
                return subprocess.run(["sh", str(ROOT / "tools/publish_site.sh"), str(site)], cwd=repo, capture_output=True, text=True)

            first = publish()
            self.assertEqual(first.returncode, 0, first.stderr)
            first_head = git("rev-parse", "FETCH_HEAD")
            self.assertEqual(git("rev-list", "--count", first_head), "1")
            self.assertEqual(set(git("ls-tree", "--name-only", first_head).splitlines()), {"index.html", ".nojekyll", "LICENSE", "NOTICE"})
            self.assertEqual(publish().returncode, 0)
            self.assertEqual(git("rev-parse", "FETCH_HEAD"), first_head)
            with (site / "index.html").open("a") as handle:
                handle.write("\n")
            self.assertEqual(publish().returncode, 0)
            self.assertEqual(git("rev-parse", "FETCH_HEAD^"), first_head)
            self.assertEqual(git("rev-parse", "HEAD"), original)
            self.assertEqual(git("status", "--porcelain"), "")
            (site / "secret.txt").write_text("must not be published")
            self.assertNotEqual(publish().returncode, 0)

    def test_presentation(self):
        source, files = prepare(ROOT)
        self.assertIn('lang="fr"', source)
        self.assertIn(Path("src/engine.c"), files)
        self.assertIn(Path("LICENSE"), files)
        self.assertIn(Path("NOTICE"), files)

    def test_published_links_and_payload(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "site"
            build(ROOT, output, "example/test", "a" * 40)
            source = (output / "index.html").read_text()
            self.assertIn("https://github.com/example/test/blob/" + "a" * 40 + "/src/engine.c", source)
            self.assertIn('href="#pitch"', source)
            self.assertEqual({p.name for p in output.iterdir()}, {"index.html", ".nojekyll", "LICENSE", "NOTICE"})

    def test_local_preview(self):
        with tempfile.TemporaryDirectory() as directory:
            output = Path(directory) / "site"
            build(ROOT, output)
            self.assertEqual((output / "src/engine.c").read_bytes(), (ROOT / "src/engine.c").read_bytes())
            self.assertFalse((output / "config.mk").exists())
            self.assertFalse((output / "tests/out").exists())
            with self.assertRaises(FileExistsError):
                build(ROOT, output)

    def test_invalid_links(self):
        for markup in (
            '<a href="#missing">bad</a>',
            '<p id="x"></p><p id="x"></p>',
            '<a href="src/missing.c">bad</a>',
            '<a href="../private.md">bad</a>',
            '<a href="config.mk">bad</a>',
            '<a href="javascript:alert(1)">bad</a>',
            '<script src="https://example.com/script.js"></script>',
        ):
            with self.subTest(markup=markup), tempfile.TemporaryDirectory() as directory:
                root = Path(directory)
                (root / "presentation-code.html").write_text(markup)
                with self.assertRaises(ValueError):
                    prepare(root)

    def test_invalid_repository_metadata(self):
        for repository, revision in (("example/test", None), ("bad/path/name", "a" * 40), ("example/test", "main")):
            with self.subTest(repository=repository, revision=revision):
                with self.assertRaises(ValueError):
                    prepare(ROOT, repository, revision)


if __name__ == "__main__":
    unittest.main()
