#!/usr/bin/env python3
import html
import os
import re
from pathlib import Path


def read_tsv(path: Path, columns: int) -> dict[str, list[str]]:
    rows: dict[str, list[str]] = {}
    if not path.is_file():
        return rows
    for line in path.read_text(errors="replace").splitlines():
        fields = line.split("\t")
        if len(fields) == columns:
            rows[fields[0]] = fields[1:]
    return rows


repo_root = Path(__file__).resolve().parents[2]
summary_path = Path(os.environ["GITHUB_STEP_SUMMARY"])
unit_results = read_tsv(Path(os.environ["UNIT_TEST_RESULTS_FILE"]), 5)
no_guard_unit_results = read_tsv(Path(os.environ["NO_GUARD_UNIT_TEST_RESULTS_FILE"]), 5)
native_unit_results = read_tsv(Path(os.environ["NATIVE_UNIT_TEST_RESULTS_FILE"]), 5)
example_results = read_tsv(Path(os.environ["EXAMPLE_RESULTS_FILE"]), 2)
unit_output_path = Path(os.environ["UNIT_TEST_OUTPUT_FILE"])
no_guard_unit_output_path = Path(os.environ["NO_GUARD_UNIT_TEST_OUTPUT_FILE"])
native_unit_output_path = Path(os.environ["NATIVE_UNIT_TEST_OUTPUT_FILE"])

unit_names = sorted({
    path.stem.removeprefix("test_")
    for path in (repo_root / "tests" / "unit").glob("test_*.cpp")
    if path.name not in {"test_unit.cpp", "test_support.cpp"}
} | set(native_unit_results) | {f"no_guards/{name}" for name in no_guard_unit_results})
unit_results.update(native_unit_results)
unit_results.update({f"no_guards/{name}": result for name, result in no_guard_unit_results.items()})
example_names = sorted(
    re.findall(r"^\[env:(ESP32[^]]*)\]$", (repo_root / "platformio.ini").read_text(), re.MULTILINE)
)
overall_unit_state = "OK" if unit_names and all(unit_results.get(name, ["FALSE"] * 4)[3] == "OK" for name in unit_names) else "FALSE"

lines = [
    "## Unit tests",
    "",
    f"**Overall result: {overall_unit_state}**",
    "",
    "| Test | Compiled | Executed | Correct result | Status |",
    "|---|---:|---:|---:|---:|",
]
for name in unit_names:
    compiled, executed, correct, state = unit_results.get(name, ["FALSE"] * 4)
    lines.append(f"| `{name}` | {compiled} | {executed} | {correct} | {state} |")

if unit_output_path.is_file():
    unit_output = unit_output_path.read_text(errors="replace").strip()
else:
    unit_output = "(no serial output produced)"
if native_unit_output_path.is_file():
    native_unit_output = native_unit_output_path.read_text(errors="replace").strip()
else:
    native_unit_output = "(no native output produced)"
if no_guard_unit_output_path.is_file():
    no_guard_unit_output = no_guard_unit_output_path.read_text(errors="replace").strip()
else:
    no_guard_unit_output = "(no guard-disabled serial output produced)"
lines.extend([
    "",
    "### Unit test results",
    "",
    "<details><summary>QEMU serial output</summary>",
    "",
    f"<pre>{html.escape(unit_output)}</pre>",
    "",
    "</details>",
    "",
    "<details><summary>QEMU serial output (amount guards disabled)</summary>",
    "",
    f"<pre>{html.escape(no_guard_unit_output)}</pre>",
    "",
    "</details>",
    "",
    "<details><summary>Native output</summary>",
    "",
    f"<pre>{html.escape(native_unit_output)}</pre>",
    "",
    "</details>",
    "",
    "## Examples",
    "",
    "| Example | Compiled |",
    "|---|---:|",
])
for name in example_names:
    lines.append(f"| `{name}` | {example_results.get(name, ['FALSE'])[0]} |")
if not example_names:
    lines.append("| _(no examples discovered)_ | FALSE |")

with summary_path.open("a") as summary:
    summary.write("\n".join(lines) + "\n")
