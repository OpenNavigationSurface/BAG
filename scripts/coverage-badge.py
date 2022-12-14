import sys
import argparse
import re

from genbadge.utils_badge import Badge, COLORS


COVERAGE_LEVEL_RE_STR = "(\d{1,3}(?:\.\d*){0,1})%"
COVERAGE_LEVEL_RE = re.compile(COVERAGE_LEVEL_RE_STR)


def main():
    parser = argparse.ArgumentParser(
        description='Create coverage badge'
    )
    parser.add_argument('-c', '--coverage_level',
                        help=f"Coverage percentage; String matching pattern: {COVERAGE_LEVEL_RE_STR}")
    parser.add_argument('-o', '--out_path',
                        help='Name of file to write to. Will overwrite an existing file.')
    parser.add_argument('--label', help='Text to use on the left side of coverage badge.',
                        default='Coverage')
    args = parser.parse_args()

    m = COVERAGE_LEVEL_RE.match(args.coverage_level)
    if m is None:
        return (f"Unable to generate coverage badge at {args.out_path} due to error: "
                f"Coverage level '{args.coverage_level}' does not match pattern: '{COVERAGE_LEVEL_RE_STR}'.")

    right_text = m.group(0)

    coverage_level = float(m.group(1))
    if coverage_level < 50.0:
        badge_color = COLORS['red']
    elif coverage_level < 60.0:
        badge_color = COLORS['orange']
    elif coverage_level < 70.0:
        badge_color = COLORS['yellow']
    elif coverage_level < 85.0:
        badge_color = COLORS['yellowgreen']
    else:
        badge_color = COLORS['green']

    badge = Badge(args.label, right_text, badge_color)
    badge.write_to(args.out_path)

    return 0


if __name__ == '__main__':
    sys.exit(main())
