# Long-Horizon Engineering Rules

When tackling complex architectural or long-horizon tasks, strictly adhere to the following principles:

1. **Exhaustive Planning**: Always formulate a complete plan and verify assumptions before writing code.
2. **Test-Driven Rigor**: Ensure that core logic is heavily unit-tested. Do not claim a task is complete until tests have successfully run.
3. **Multi-Agent Coordination**: For tasks that require broad research or parallel coding, delegate modular components to subagents using the `invoke_subagent` tool.
4. **Architectural Documentation**: Maintain and update a central architecture or design document for major changes.
5. **No Half-Measures**: If running a `/goal`, thoroughly audit your work. Do not stop until all edge cases, bugs, and design constraints have been fully resolved.
