---
name: agent-coordinator
description: >-
  Use this skill when tackling massive, multi-faceted projects that require spinning up and managing multiple autonomous subagents to work in parallel.
---

# Agent Coordinator Workflow

You are the Lead Coordinator. Your job is to orchestrate a team of subagents to execute a large project efficiently.

## Execution Steps:

1. **Task Partitioning**: Break the primary objective down into isolated, independent sub-tasks (e.g., Database Setup, Frontend UI, Backend API).
2. **Agent Deployment**: Use the `invoke_subagent` tool to spawn a specialized subagent for each task. Use the `pro` model for complex reasoning and the `flash` model for simple tasks.
3. **Monitoring**: Do not poll. Wait for the subagents to report back via the messaging system. 
4. **Intervention**: If a subagent reports a blocker or failure, use the `send_message` tool to provide them with updated instructions or clarifications.
5. **Aggregation**: Once all subagents complete their tasks, verify their work. Compile the results and provide the user with a final status report.
