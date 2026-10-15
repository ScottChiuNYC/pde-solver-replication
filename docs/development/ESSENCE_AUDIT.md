# Essence Audit

This file is a human-readable provenance journal, not normative implementation authority.

## Current status

- Run: `icc-pde-replication-20261015-01`
- Backend: GitHub
- Essence: `docs/development/essence.md`
- State: PROPOSED — authorized remediation completed; fresh round 2 follows exact-head integration
- Latest completed formal round: 1, FAIL
- Unresolved owner decisions: none from round 1
- Frozen identity: none
- Skill commit: `ea6453401716d828de21c53b1c33d326647e3499`
- Run/round-1 control: [#1](https://github.com/ScottChiuNYC/pde-solver-replication/issues/1)
- Focused contract PR: [#6](https://github.com/ScottChiuNYC/pde-solver-replication/pull/6)
- Isolation: delivery_only; fresh contexts and checked packets, with broader inherited tools available. No enforced tool sandbox is claimed.

## Round 1 — historical result

- Exact target: `33cab5517a6f51dc32a0872404f7b81a9cf23128`
- Tree: `eeabcd7582ba6004bce12c64c13cc6c92210b9d3`
- Scope: The 1D parabolic PDE solver responsibility in docs/development/essence.md: public C++ interfaces, numerical conventions, boundary and failure behavior, acceptance evidence, and interaction with the established C++/Python Project Skeleton Contract. Audit the contract only; do not implement the solver.
- A: VALID / SEALED / FAIL — [#2](https://github.com/ScottChiuNYC/pde-solver-replication/issues/2)
- B: VALID / SEALED / FAIL — [#3](https://github.com/ScottChiuNYC/pde-solver-replication/issues/3)
- C: VALID / SEALED / FAIL — [#4](https://github.com/ScottChiuNYC/pde-solver-replication/issues/4)
- Reconciliation: [#5](https://github.com/ScottChiuNYC/pde-solver-replication/issues/5), FAIL → WAITING_FOR_OWNER
- Accounting: 25 raw findings; 24 mapped, one dismissed; nine canonical findings, none unaccounted
- Authority-determined repairs: PDE-F01 and PDE-F05
- Owner items: PDE-D01 through PDE-D07; subsequently resolved by the actual owner statement below
- Sealed results and reconciliation are preserved unchanged.

## Owner resolution and remediation

The owner wrote “我接受你全部的建議，請繼續” in the active conversation on 2026-10-15 at 00:16:57 America/New_York. The actual statement, precise accepted alternatives and scoped delegations are durably recorded in [#7](https://github.com/ScottChiuNYC/pde-solver-replication/issues/7#issuecomment-6184308746). The API-authored record is a transcript of this actual human instruction, not a manufactured approval.

Accepted choices: Array1D with unpadded off-diagonals and transactional RHS; at least one interior node; explicit initial/boundary data; finite theta in [0.5,1]; Dirichlet precedence at initial corners; general positive diffusion with checked no-pivot Thomas and an explicitly specified row-relative pivot policy; convergence gates plus the retained fixed heat benchmark. The original absolute-floor pivot proposal was replaced, not adopted.

- Separate Remediator: `/root/remediator_r1`, execution `r1-remediator-e1`
- Input target: round-1 exact authority above
- Authorized findings: PDE-F01 through PDE-F09
- Remediation record and complete mapping: [#8](https://github.com/ScottChiuNYC/pde-solver-replication/issues/8)
- Candidate Essence blob: `4c88b6656bba9adc71907cadaf4de1da8d64f3a9`
- Candidate content SHA-256: `68fdcb4f33f1537db8cd4c78ae48bcbe54128f0046f4a72e51ae687428a9a29f`
- Validation: all nine findings/seven decisions mapped; complete public declaration block passed C++17 syntax checking; normative input contains no audit/decision history references; remote file/tree/PR validation is recorded in #8
- Status: remediated candidate, not PASS; findings await independent re-audit

The next formal round will be bound to the exact main commit after this focused PR passes its required checks and is integrated. Auditors receive the current normative contract and Project Boundary only; owner discussion, prior findings, repair claims, journals and backend responses stay outside their contexts.

## Independence and scope

This run uses only the specified replication repository and pinned skill repository. No other PDE repository or previous PDE convergence results are used. Only contract/journal documentation changes; no solver, acceptance-test, build, package or workflow implementation is added.

## Freeze

No reconciled PASS or freeze exists yet. Freeze requires a new complete independent round, canonical PASS and zero unresolved owner decisions. The owner's acceptance authorizes remediation and continued convergence; it is not itself a PASS.
