# AGENTS.md

## General Rules
- Use CRLF for new and modified text files unless the specific file clearly uses another line ending style.
- Do not use `dynamic_cast`; use the project's existing RTTI (`EGO_RTTI`, `ego::rtti`) or explicit interfaces for type checks and casts.
- Put all temporary or disposable work artifacts under VCS-ignored directories unless the project has a dedicated real output location for them. This includes build/configure trees, generated files, logs, experiments, and verification outputs; remove disposable directories after confirming they are no longer needed.
- Do not edit vendored code in `ThirtParty/` unless explicitly requested.

## Project
- The project uses C++20 and CMake.
- Main engine components live in `Ego/Sources/Components`.
- Plugins live in `Ego/Sources/Plugins`.
- Demos and the launcher live in `Demo`.
- Shared CMake helpers live in `CMake`; when changing build logic, use the existing functions and structure.

## C++ Style
- Preserve the current code style and the settings from `Utils/ReSharper.DotSettings`.
- Use `#pragma once` and namespaces `ego`/`ego::*`.
- Naming: classes, structs, unions, enums, enum members, concepts, typedefs/type aliases, global functions, global variables, and constants use `UpperCamelCase`.
- Naming: instance methods, local variables, and lambdas use `lowerCamelCase`.
- Naming: static class/struct methods use `UpperCamelCase`.
- Naming: class/struct fields and union members use `m_lowerCamelCase`.
- Naming: static class/struct fields use `UpperCamelCase`.
- Naming: function and lambda parameters use `_lowerCamelCase`.
- Formatting: keep at most one consecutive blank line in code and declarations.
- Formatting: preprocessor directives use normal indentation; keep empty blocks compact.
- Formatting: simple `case` statements and embedded statements use line breaks instead of staying on one line.
- Formatting: in multiline declarations and calls, wrap after the opening parenthesis and before the closing parenthesis; wrap parameters, arguments, and braced init lists with `chop if long` behavior.
- Formatting: always wrap base clauses; wrap constructor/member initializer lists when long, with a line break after each comma.
- Formatting: put enum members on separate lines except when ReSharper keeps a short enum on one line.
- Formatting: align multiline ternary expressions; do not line break before ternary operator signs.
- Formatting: put `extern "C"`/linkage specification braces on the next line and keep the body indented.
- Do not introduce raw pointers for ownership, object references, or project-level API contracts.
- If a platform, third-party, or C ABI forces a raw pointer or raw handle, keep it isolated at the boundary and do not propagate it as normal project ownership.
- Prefer the project's object reference types: `ego::Reference<T>`/`TypeReference` declared through `EGO_REFERENCE`, and `ego::SharedPointer<T>`/`TypePointer` plus `ego::WeakPointer<T>`/`TypeWeakPointer` declared through `EGO_POINTER`/`EGO_WEAK_POINTER`.
- Choose deliberately between `Reference` and `Pointer`: prefer `Reference` when its lifetime model fits, because the project pointer type has higher overhead.
- Use project pointer/reference types where they are already used by the surrounding code.
- If project pointer/reference types do not fit the required ownership, lifetime, performance, or interoperability constraints, use standard C++ ownership tools (`std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`, references, `std::optional`, `std::reference_wrapper`) according to normal C++ ownership rules.
- For checks and invariants, use the existing macros `EGO_ASSERT`, `EGO_ASSERT_FAIL_MESSAGE`, and `EGO_CHECK_INITIALIZATION`.
- For template implementations, preserve the existing `.h` + `.hpp` pattern.

## Includes
- When adding or changing `#include` directives, keep them grouped with one blank line between groups.
- Include groups are ordered as follows:
  1. Standard library headers.
  2. Project headers, grouped by dependency direction: lower-level/base modules first, then modules that depend on them. Apply this consistently across components, plugins, launchers, and demos.
  3. Third-party library headers.
- Keep includes within each group sorted consistently with the surrounding file.
- Do not move third-party includes above standard library or Ego project includes.
- Do not introduce a new include if a forward declaration is already sufficient and consistent with the surrounding code.

## Verification
- For C++ or CMake changes, when appropriate, verify CMake configuration/build with the project's existing commands.
- If build or tests cannot be run, state the reason clearly in the final response.
