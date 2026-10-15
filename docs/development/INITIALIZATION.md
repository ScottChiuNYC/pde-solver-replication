# Project initialization

This repository was initialized as a Python package with a C++ core using
Copier. This step includes only the template's sample code and infrastructure;
no PDE functionality or numerical requirements were added.

## Template provenance

- Source: <https://github.com/ScottChiuNYC/copier-pypackage-cpp>
- Default branch at resolution: `master`
- Exact revision: `67630a4d60c57ad89142cebd99b5c2e6da01f17a`
- Revision type: **untagged commit**, explicitly selected for this experiment
- Copier: `9.19.1`, as documented by the template

The copy command used the full immutable SHA with `--vcs-ref`:

```bash
copier copy --defaults \
  --vcs-ref 67630a4d60c57ad89142cebd99b5c2e6da01f17a \
  --data 'project_name=PDE Software Replication' \
  --data project_slug=pde-solver-replication \
  --data module_name=pde_software_replication \
  --data 'description=Infrastructure skeleton for the PDE software replication experiment.' \
  https://github.com/ScottChiuNYC/copier-pypackage-cpp.git \
  pde-solver-replication
```

Copier generated `.copier-answers.yml` with its normal abbreviated `_commit`
value, `67630a4`, which resolves to the full SHA recorded above. Its source
remains the upstream template URL so future Copier updates are possible.
Keep this answers file committed. For an update, commit local changes first,
review the intended template revision, and pass that revision explicitly to
`copier update --vcs-ref <revision>`.

## Project values

| Setting | Value |
| --- | --- |
| Project name | `PDE Software Replication` |
| Repository/project slug | `pde-solver-replication` |
| Python package | `pde_software_replication` |
| Initial version | `0.1.0` |
| CMake package | `pde_software_replication_core` |
| CMake consumer target | `pde_software_replication_core::core` |
| vcpkg port | `pde-software-replication-core` |
| Python requirement | `>=3.12` |
| C++ standard | `17` |
| Minimum CMake version | `3.25` |

Other values use the template's documented defaults, including its author
metadata placeholders (`Your Name <you@example.com>`). Python/C++ bindings,
CMake, local vcpkg ports, tests, packaging scripts, and the Windows/Linux
GitHub Actions workflow are retained.

## Initialization validation

Validated on Linux x86_64 with Python 3.12.14 and GCC 13.3.0:

```bash
python scripts/bootstrap.py
python scripts/validate.py --wheel
```

Both commands exited successfully. The bootstrap installed the template's
pinned tools and vcpkg baseline
`00807e89724847a866033a2f042c38fafae3c534`.

- CMake configure/build and the C++ CTest target passed (1/1).
- Python tests passed (3/3).
- The source distribution and native wheel built successfully; the wheel was
  rebuilt from the source distribution, installed into a clean virtual
  environment, and imported outside the source checkout.
- The local vcpkg port passed independent C++ consumer checks in Release and
  Debug, with the install prefix relocated and source headers hidden.
- Validation confirmed that `pkgconf` was not acquired or installed.
- Static checks confirmed the source revision, package metadata, required
  integration/test files, Python syntax, root JSON files, and rendered GitHub
  Actions expressions.

Windows could not be executed locally; the preserved GitHub Actions workflow
runs the same bootstrap and validation commands on `ubuntu-latest` and
`windows-2022`. Live CI results are available in the repository's Actions tab.

Only source files, template infrastructure, and these provenance notes are
committed. Tool environments, native build products, caches, and distributions
remain excluded by the template's `.gitignore`.
