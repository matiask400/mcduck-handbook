"""Validate publication time, branch preservation, and PDF version links."""
from datetime import datetime, timezone
from pathlib import Path
import runpy
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SCRIPT = ROOT / '.github/scripts/build_pages.py'
render_page = runpy.run_path(str(SCRIPT))['render_page']


class BuildPagesTests(unittest.TestCase):
    def test_argentina_date_changes_at_midnight(self):
        instant = datetime(2026, 10, 26, 1, 30, tzinfo=timezone.utc)
        for branch in ('main', 'dev'):
            html = render_page(branch, 'abc1234', 'team/book', updated_at=instant)
            self.assertIn('25/10/2026 22:30', html)
            self.assertIn('datetime="2026-10-25T22:30:00-03:00"', html)
            self.assertIn('<strong>Última actualización<br><time', html)
            self.assertIn('text-align: right', html)
            self.assertIn('abc1234</a>', html)
            self.assertEqual(html.count('McDuck-Handbook.pdf?v=abc1234'), 3)

    def test_unpublished_main_has_no_update_claim(self):
        html = render_page('main', 'abc', 'team/book', pending=True)
        self.assertNotIn('<time ', html)
        self.assertNotIn('<iframe ', html)

    def test_dev_publication_preserves_main(self):
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            site = root / 'site'
            site.mkdir()
            (site / 'index.html').write_text('existing main')
            (site / 'McDuck-Handbook.pdf').write_bytes(b'existing main PDF')
            source = root / 'new.pdf'
            source.write_bytes(b'new dev PDF')
            subprocess.run([sys.executable, str(SCRIPT), '--site', str(site),
                            '--pdf', str(source), '--branch', 'dev', '--sha', 'abc1234',
                            '--repository', 'team/book'], check=True)
            self.assertEqual((site / 'index.html').read_text(), 'existing main')
            self.assertEqual((site / 'McDuck-Handbook.pdf').read_bytes(), b'existing main PDF')
            self.assertEqual((site / 'dev/McDuck-Handbook.pdf').read_bytes(), b'new dev PDF')
            self.assertIn('<time ', (site / 'dev/index.html').read_text())
            self.assertTrue((site / '.nojekyll').exists())


if __name__ == '__main__':
    unittest.main()
