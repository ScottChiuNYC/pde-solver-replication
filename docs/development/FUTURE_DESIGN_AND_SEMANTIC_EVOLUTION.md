# Follow-up TODO — PDE design and semantic evolution

Status: **planning notes only**, 2026-10-16. Do not change the frozen normative Essence or the merged PDE implementation as part of this note.

## Where we are

Two independent ICC audit/convergence runs began from substantially the same original 1D PDE Essence draft, but produced **different, internally frozen contracts**, largely because the owner chose differently when asked similar design questions. Those choices were made primarily while testing the Skill, not after a production-focused design review. Neither frozen contract should be treated as the definitive long-term PDE design.

- **Original PDE solver**: [pde-solver PR #1](https://github.com/ScottChiuNYC/pde-solver/pull/1), unmerged; frozen contract at `docs/model/ESSENCE_CONTRACT.md` (commit `f16541acce45ec310e220e4bab7437bc45033118`). Fixed CN theta=0.5, private Thomas helper, IC/BC corner compatibility, unified callback errors.
- **Replication**: [this repository's frozen Essence](essence.md), blob `4f8c036f9e916fcacce37ee56c72566af9f5b8ed`; implementation merged in [PR #15](https://github.com/ScottChiuNYC/pde-solver-replication/pull/15). Theta in [0.5,1], public Thomas solver, boundary precedence at corners, original callback exceptions propagated, stronger numerical/failure acceptance.
- These are **historical alternatives, not a single linear V1→V2 evolution**. Preserve their provenance and owner-decision records. The original *draft* is the intended starting point for a future production-focused redesign, not either frozen branch.

## Priority / next actions

1. **Now:** E2E test the second Skill, Contract-to-Code Convergence, using the existing replication Frozen Essence + Project Skeleton Contract as immutable inputs in a **new clean repository**. No prior implementation, tests, or review reports should be exposed to its agents. Validate independent pre-code acceptance assets, implementation, independent review, supplemental final verification and handoffs. Do not reopen PDE yet.
2. **After Skill 2 stabilizes:** return to the **original PDE Essence draft**, reopen design deliberation and revisit owner choices thoughtfully for a usable long-lived library (including theta scope, corner policy, public/private Thomas responsibility, exception behavior, acceptance breadth). Do **not** simply splice preferred clauses from two frozen contracts or rewrite frozen historical artifacts. Create a newly audited/frozen successor with clear provenance.
3. **Then:** use the PDE project to test **incremental development**, including semantic change impact on downstream requirements, tests, APIs and implementation.

## Future idea: Semantic Version Control (not yet designed or implemented)

Git provides immutable byte-level history; add a lightweight semantic layer *when needed*: stable requirement/decision IDs, owner rationale and alternatives, assumptions, semantic changes between frozen baselines, and dependency/impact links to downstream contracts, tests, APIs, consumers and code. A change to an early decision may invalidate later stages; do not silently rewrite old baselines or assume linear history. Support explicit branching/reconvergence, traceable supersession, and targeted revalidation. First probe existing Git/agent capabilities before building infrastructure.

Key principle: independent audits and faithful AI implementation improve consistency but cannot guarantee optimal **owner design decisions**. Decisions should be explainable and revisable; fast alternate implementations reduce time cost, though compute/token and downstream migration costs remain. Existing ICC owner-decision options, tradeoffs and recommendations are adequate for now; **do not add a new owner-decision gate solely because of this observation**.

This note is non-normative handoff context; the current frozen contract remains authoritative for the upcoming Skill 2 test.
