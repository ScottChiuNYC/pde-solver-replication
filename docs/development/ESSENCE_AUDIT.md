# Essence Audit

This file is a human-readable provenance journal. It is not normative implementation authority.

## Status

- Essence: `docs/development/essence.md` (unchanged draft)
- Run: `icc-pde-replication-20261015-01`
- Backend: GitHub
- State: **WAITING_FOR_OWNER**
- Latest audited target: `33cab5517a6f51dc32a0872404f7b81a9cf23128`
- Latest round: 1
- Control: [round 1](https://github.com/ScottChiuNYC/pde-solver-replication/issues/1)
- Canonical result: [reconciliation](https://github.com/ScottChiuNYC/pde-solver-replication/issues/5), **FAIL**
- Owner decisions: [decision packet](https://github.com/ScottChiuNYC/pde-solver-replication/issues/7), seven unresolved
- Journal pull request: [PR #6](https://github.com/ScottChiuNYC/pde-solver-replication/pull/6)
- Skill commit: `ea6453401716d828de21c53b1c33d326647e3499`
- Isolation: delivery_only; fresh worker contexts and checked packets, with broader inherited tools still available. No enforced tool sandbox is claimed.

## Round 1

- Target commit: `33cab5517a6f51dc32a0872404f7b81a9cf23128`
- Target tree: `eeabcd7582ba6004bce12c64c13cc6c92210b9d3`
- Scope: The 1D parabolic PDE solver responsibility in docs/development/essence.md: public C++ interfaces, numerical conventions, boundary and failure behavior, acceptance evidence, and interaction with the established C++/Python Project Skeleton Contract. Audit the contract only; do not implement the solver.
- Auditor results:
  - A: VALID / SEALED / FAIL — [raw result](https://github.com/ScottChiuNYC/pde-solver-replication/issues/2)
  - B: VALID / SEALED / FAIL — [raw result](https://github.com/ScottChiuNYC/pde-solver-replication/issues/3)
  - C: VALID / SEALED / FAIL — [raw result](https://github.com/ScottChiuNYC/pde-solver-replication/issues/4)
- Input manifest SHA-256: `0671648cea9aea3a6a80751243e5aca17b2eba60c13e5545f3083e265dadc112`
- Reconciliation SHA-256: `df087b1b663236febf38fe1f3062def6aff2487cfe9a9ed9389ac743596172ee`
- Raw accounting: 25 findings; 24 mapped, one dismissed; none unaccounted
- Canonical findings: PDE-F01 through PDE-F09
- Closed canonical findings: none; no substantive remediation performed
- Dismissed raw finding: B-04; the governing theta equation already determines source weights
- Owner decisions: PDE-D01 through PDE-D07, unresolved; proposed alternatives are not owner resolutions
- Remediation identity: none
- Round verdict: FAIL
- Next state: WAITING_FOR_OWNER

| Canonical finding | Required next input |
| --- | --- |
| PDE-F01 — header mapping and Skeleton integration | Authority-determined repair |
| PDE-F02 — complete public helper API | PDE-D01 |
| PDE-F03 — valid grids and zero-interior support | PDE-D02 |
| PDE-F04 — callback defaults and failures | PDE-D03 |
| PDE-F05 — consistent discrete row | Authority-determined repair |
| PDE-F06 — supported theta methods | PDE-D04 |
| PDE-F07 — initial/Dirichlet corner policy | PDE-D05 |
| PDE-F08 — admitted coefficients and Thomas breakdown | PDE-D06 |
| PDE-F09 — objective acceptance evidence | PDE-D07 |

Source weighting and ordinary per-step Dirichlet time levels are determined by existing equations, not separate owner choices. Complete mappings, questions, alternatives and consequences are preserved in the canonical reconciliation and decision packet.

## Entry and verification

The existing draft governs the first 1D solver responsibility. The Project Skeleton Contract supplies the established repository boundary. No separate design notes were designated for semantic comparison. No substantive draft changes preceded this round. This independent run used only the specified replication repository and skill repository; no other PDE repository or previous convergence results were inspected or used.

The dispatcher verified the exact Git-data commit/tree, exact-ref boundary/Essence blobs and their Git/content hashes. Each Auditor received the same checked authority and scope in a fresh context. All three complete sealed results were mechanically persisted and read back; packet bindings and result validity passed the pinned helper checks. The fresh separate Reconciler accounted for every finding. Its unsealed writeout was interrupted and resumed with unchanged inputs to finish a compact record; no interim status was accepted as canonical evidence. The complete canonical record was then validated, persisted and read back.

The initial journal-only head `a05c638caff62bbed87a56b02693a3a844262ac3` passed both push and PR Build and Test runs on Windows/Linux. These results apply to that initial provenance head only; the PR's checks record later journal revisions. No build, workflow, solver source or tests were changed by this convergence run.

## Owner barrier and resume

Persist the owner's actual resolutions and their durable source in the decision record. Incorporate the selected semantics into current normative authority, complete the two authority-determined repairs and other authorized remediation, then establish a new immutable target and run all three fresh independent Auditor slots. This audit FAIL and the proposed options do not authorize freeze.

## Freeze

- Reconciled PASS target: none
- Frozen contract identity: none
- Frozen status: not established
- Unresolved owner decisions: seven
- Original Essence blob: `05cb221be2affb4f9c876b46ada061ac788d4be9`, unchanged

The candidate and round remain preserved at WAITING_FOR_OWNER. Journal commits are provenance only and do not modify the audited normative files.
