import gzip
import os
import shutil
import subprocess
import sys
from pathlib import Path

Import("env")  # noqa: F821 — provided by PlatformIO

PROJECT_DIR = Path(env["PROJECT_DIR"])  # noqa: F821
INTERFACE_DIR = PROJECT_DIR / "interface"
DIST_DIR = INTERFACE_DIR / "dist"
DATA_DIR = PROJECT_DIR / "data"


def gzip_file(path: Path) -> None:
    gz_path = path.with_suffix(path.suffix + ".gz")
    with path.open("rb") as f_in, gzip.open(gz_path, "wb") as f_out:
        shutil.copyfileobj(f_in, f_out)
    path.unlink()


def find_npm() -> str:
    npm = shutil.which("npm")
    if npm:
        return npm
    candidates = [
        "/opt/homebrew/bin/npm",
        "/usr/local/bin/npm",
        Path.home() / ".volta" / "bin" / "npm",
        Path.home() / ".nvm" / "current" / "bin" / "npm",
    ]
    for path in candidates:
        if Path(path).is_file() and os.access(path, os.X_OK):
            return str(path)
    raise RuntimeError(
        "npm not found. Install Node.js, or ensure npm is on PATH for the "
        "shell that launches PlatformIO (VSCode tasks may not inherit your "
        "interactive shell PATH)."
    )


def build_interface() -> None:
    npm = find_npm()
    print(f"Building interface with npm ({npm})")
    npm_env = os.environ.copy()
    bin_dirs = {str(Path(npm).parent), str(Path(npm).resolve().parent)}
    npm_env["PATH"] = os.pathsep.join([*bin_dirs, npm_env.get("PATH", "")])
    subprocess.check_call([npm, "install"], cwd=INTERFACE_DIR, env=npm_env)
    subprocess.check_call([npm, "run", "build"], cwd=INTERFACE_DIR, env=npm_env)

    if DATA_DIR.exists():
        shutil.rmtree(DATA_DIR)
    print("Copying interface to data directory")
    shutil.copytree(DIST_DIR, DATA_DIR)
    for current, _dirs, files in os.walk(DATA_DIR):
        for name in files:
            gzip_file(Path(current) / name)


targets = set(BUILD_TARGETS) | set(COMMAND_LINE_TARGETS)  # noqa: F821
SKIP_TARGETS = {"clean", "fullclean", "cleanall", "erase"}
TRIGGERING_TARGETS = {"upload", "uploadfs", "buildfs"}
is_clean = "--clean" in sys.argv or any(arg.startswith("FULLCLEAN=") for arg in sys.argv)
if is_clean or SKIP_TARGETS.intersection(targets):
    label = ", ".join(sorted(targets)) or "clean"
    print("Skipping build interface step for target(s): " + label)
elif not targets or TRIGGERING_TARGETS.intersection(targets):
    build_interface()
else:
    print("Skipping build interface step for target(s): " + ", ".join(sorted(targets)))
