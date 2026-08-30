---
name: Windows C++ Game Dev Constraints
description: Critical guardrails for C++ development on Windows (MSVC limits, Win32 macro sandboxing, CMake policies).
---

# Windows C++ Game Development Guardrails

When working on C++ projects on a Windows OS (especially game engines, graphics, or networking), you MUST adhere to the following invariants:

## 1. Win32 Macro Sandboxing
Windows headers (`<windows.h>`, `<winsock2.h>`) pollute the global namespace with generic macros like `min`, `max`, `Rectangle`, and `CloseWindow`. This causes catastrophic compilation failures when mixed with standard C++ (`<algorithm>`) or graphics libraries (like Raylib).
**Rule**: Before including any Windows headers, you MUST define the following protections:
```cpp
#define NOMINMAX
#define NOGDI
#define NOUSER
```

## 2. MSVC Stack Limits (The 1MB Rule)
Visual Studio's MSVC compiler enforces a strict default thread stack limit of exactly **1 MB**.
**Rule**: NEVER allocate large data structures (game grids, state histories, massive arrays, etc.) on the stack (e.g., `MyState history[1024]`). Anything approaching ~100KB or more MUST be heap-allocated (e.g., using `std::vector`, `std::unique_ptr`, or `new`) to prevent silent `Exit Code 1` crashes.

## 3. CMake Policy Minimums for FetchContent
Fetching older external C libraries (like Raylib) via `FetchContent` on modern CMake/Visual Studio setups often fails due to deprecation warnings being treated as fatal errors by the IDE.
**Rule**: Always force the minimum policy version at the top of the root `CMakeLists.txt` before fetching dependencies:
```cmake
set(CMAKE_POLICY_VERSION_MINIMUM "3.5" CACHE STRING "Minimum CMake policy version" FORCE)
```
