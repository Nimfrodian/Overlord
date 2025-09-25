import subprocess
from pathlib import Path
Import("env")

def generate_coverage(env):
    build_dir = Path(env.subst("$BUILD_DIR"))
    project_dir = Path(env.subst("$PROJECT_DIR"))
    coverage_dir = build_dir / "coverage_report"
    coverage_html = coverage_dir / "coverage.html"

    coverage_dir.mkdir(parents=True, exist_ok=True)

    print(f"Generating coverage report into {coverage_html}")

    subprocess.run([
        "gcovr",
        "-r", str(project_dir),
        str(build_dir),
        "--exclude", r"\.pio/",
        "--html",
        "--html-details",
        "-o", str(coverage_html)
    ], check=True)

generate_coverage(env)