"""Exercise Poetry's real archive selection for recursive native inputs."""
from pathlib import Path
import shutil
import subprocess
import tarfile


def test_sdist_contains_nested_native_build_inputs(tmp_path):
    project = Path(__file__).resolve().parents[2]
    copy = tmp_path / "project"
    shutil.copytree(project, copy, ignore=shutil.ignore_patterns(
        ".git", ".dev-tools", ".venv", "build", "dist", "__pycache__",
        ".pytest_cache", "include.consumer-hidden", "*.so", "*.pyd"))
    source = copy / "cpp/src/packaging_probe/nested.cpp"
    header = copy / "cpp/include/pde_software_replication_core/packaging_probe/nested.h"
    source.parent.mkdir(parents=True)
    header.parent.mkdir(parents=True)
    source.write_text("int nested_packaging_probe() { return 23; }\n", encoding="utf-8")
    header.write_text("#pragma once\nint nested_packaging_probe();\n", encoding="utf-8")
    output = tmp_path / "dist"
    subprocess.run(["poetry", "build", "--format", "sdist", "--output", str(output)],
                   cwd=copy, check=True)
    archives = list(output.glob("*.tar.gz"))
    assert len(archives) == 1
    with tarfile.open(archives[0]) as archive:
        paths = {Path(member.name).relative_to(Path(member.name).parts[0]).as_posix()
                 for member in archive.getmembers() if member.isfile()}
    # Discover actual inputs rather than restating pyproject's glob patterns.
    native_inputs = {p.relative_to(copy).as_posix()
                     for directory in ("cpp/include", "cpp/src", "cpp/bindings")
                     for p in (copy / directory).rglob("*")
                     if p.is_file() and p.suffix in {".h", ".cpp"}}
    assert source.relative_to(copy).as_posix() in native_inputs
    assert header.relative_to(copy).as_posix() in native_inputs
    assert not native_inputs - paths, f"Missing native inputs: {sorted(native_inputs - paths)}"
