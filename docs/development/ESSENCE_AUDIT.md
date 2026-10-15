# Essence Audit

This file is a human-readable provenance journal, not normative implementation authority.

## Current status

- Run: `icc-pde-replication-20261015-01`
- Backend: GitHub
- Essence: `docs/development/essence.md`
- State: FROZEN / IMPLEMENTATION_READY
- Latest completed formal round: 2, reconciled PASS
- Unresolved owner decisions: 0 (all seven round-1 decisions resolved)
- Frozen Essence blob: `4f8c036f9e916fcacce37ee56c72566af9f5b8ed`
- Frozen Essence content SHA-256: `a8ef1e0cff9848b8fce3ee865b9171ae5587b405e525b85974c9faa9d442b436`
- Audited target commit: `23c0e26209bfd3f992f556dbc6ad97822ca2730b`
- Latest canonical PASS: [#13](https://github.com/ScottChiuNYC/pde-solver-replication/issues/13)
- Current run/round-2 control and exact freeze integration identity: [#9](https://github.com/ScottChiuNYC/pde-solver-replication/issues/9)
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
- Historical remediation status: candidate required independent re-audit; remediation itself did not establish PASS.
- Integration: focused PR #6 squash-merged as `23c0e26209bfd3f992f556dbc6ad97822ca2730b`; exact main CI [38682568312](https://github.com/ScottChiuNYC/pde-solver-replication/actions/runs/38682568312) completed successfully on Linux and Windows.

The complete fresh round below was bound to that integrated main commit. Auditors received only checked current normative authority and generic instructions; owner discussion, prior findings, repair claims, journals and backend responses remained outside their contexts.

## Round 2 — final PASS

- Exact target: `23c0e26209bfd3f992f556dbc6ad97822ca2730b`
- Tree: `484d905d4febf8fe78fa5ebd706e6e48386de014`
- Audited Essence blob: `4c88b6656bba9adc71907cadaf4de1da8d64f3a9`
- Scope: The 1D parabolic PDE solver responsibility in docs/development/essence.md: public C++ interfaces, numerical conventions, boundary and failure behavior, acceptance evidence, and interaction with the established C++/Python Project Skeleton Contract. Audit the contract only; do not implement the solver.
- Common input manifest SHA-256: `5dd9f49565b8a4f0f7113074c6311cbdc29592ae18099641c9ed42ab542aa2a1`
- A: VALID / SEALED / PASS — [#10](https://github.com/ScottChiuNYC/pde-solver-replication/issues/10), execution `r2-a-e1`
- B: VALID / SEALED / PASS — [#11](https://github.com/ScottChiuNYC/pde-solver-replication/issues/11), execution `r2-b-e1`
- C: VALID / SEALED / PASS — [#12](https://github.com/ScottChiuNYC/pde-solver-replication/issues/12), execution `r2-c-e1`
- Separate fresh Reconciler: `/root/reconciler_r2`, execution `r2-reconciler-e1`
- Canonical reconciliation: [#13](https://github.com/ScottChiuNYC/pde-solver-replication/issues/13), SEALED PASS → FREEZE
- Canonical result SHA-256: `8628657f4021c01d3e13414a870d750458a363c9b617d08400ff6541741c9496`
- Accounting: zero raw findings, zero canonical substantive findings, zero unresolved owner decisions, zero unaccounted findings
- Completeness: all three fixed slots valid, sealed and unexposed; no invalid executions, exclusions, missing slots or replacements
- Dispatcher verified exact-ref authority, all content/blob identities, packet digests and post-PASS unchanged authority. Complete execution receipts and isolation limitations are preserved in #9–#13.

## Independence and scope

This run uses only the specified replication repository and pinned skill repository. No other PDE repository or previous PDE convergence results are used. Only contract/journal documentation changes; no solver, acceptance-test, build, package or workflow implementation is added.

## Freeze

The exact round-2 authority received a reconciled PASS with zero unresolved owner decisions. Before writeback, main and all five target authority files were re-read and matched the audited target and manifest.

Freeze writeback adds only the marked status/provenance block in the Essence and updates this journal. Removing that one metadata block reproduces the audited Essence byte for byte (SHA-256 `68fdcb4f33f1537db8cd4c78ae48bcbe54128f0046f4a72e51ae687428a9a29f`). No substantive contract text changed.

The frozen Essence blob and content digest are recorded above. Exact freeze integration commit, reviewed diff, head-bound CI and final main CI receipts are recorded in [#9](https://github.com/ScottChiuNYC/pde-solver-replication/issues/9). The next stage receives the frozen Essence together with its existing Project Skeleton Contract; audit history is provenance and adds no implementation requirements.
