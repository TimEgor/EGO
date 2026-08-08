# AGENTS.md

## General Rules
- Use CRLF for new and modified text files unless the specific file clearly uses another line ending style.
- Do not use `dynamic_cast`; use the project's existing RTTI (`EGO_RTTI`, `ego::rtti`) or explicit interfaces for type checks and casts.
- Do not use `try`/`catch`.
- Put all temporary or disposable work artifacts under VCS-ignored directories unless the project has a dedicated real output location for them. This includes build/configure trees, generated files, logs, experiments, and verification outputs; remove disposable directories after confirming they are no longer needed.
- Do not edit vendored code in `ThirtParty/` unless explicitly requested.

## Project
- The project uses C++20 and CMake.
- Main engine components live in `Ego/Sources/Components`.
- Plugins live in `Ego/Sources/Plugins`.
- Demos and the demo launcher live in `Demo`.
- Shared CMake helpers live in `CMake`; when changing build logic, use the existing functions and structure.

## C++ Style
- Preserve the current code style and use the project's `.clang-format` and `.clang-tidy` as the formatting and C++ checks sources of truth.
- Use `#pragma once` and namespaces `ego`/`ego::*`.
- Do not move logic out of a class into any namespace-level function without a clear need. Keep class-related behavior in the owning class; use namespace-level helpers, including anonymous namespaces, only for clearly utility code or implementation details that are not tied to class logic.
- Do not put implementations of regular non-template functions or methods in headers; prefer source files, especially when the body is more than one line.
- Keep definitions in `.cpp` files in the same order as their corresponding declarations in the `.h` file.
- Naming: classes, structs, unions, enums, enum members, concepts, typedefs/type aliases, global functions, global variables, and constants use `UpperCamelCase`.
- Naming: instance methods, local variables, and lambdas use `lowerCamelCase`.
- Naming: static class/struct methods use `UpperCamelCase`.
- Naming: class/struct fields and union members use `m_lowerCamelCase`.
- Naming: static class/struct fields use `UpperCamelCase`.
- Naming: function and lambda parameters use `_lowerCamelCase`.
- Naming: macros use `UPPER_SNAKE_CASE`.
- Naming: function-like macro parameters use `_UPPER_SNAKE_CASE`.
- Avoid `auto` for simple or obvious types; use it only for complex composite types, such as collection iterators, or when the language/API requires it.
- Do not use raw character pointers or character arrays for named string constants; use `std::string_view` or an explicit string type.
- Use `ComputeVector*` and `ComputeMatrix*` only for short-lived intermediate values during computations. Do not use them for persistent value storage, including class/struct fields; use the corresponding `Vector*` and `Matrix*` types and convert at computation boundaries.
- Structure functions and classes into clear logical blocks. Prefer separating independent steps, responsibilities, and declaration groups instead of keeping large uninterrupted blocks of code.
- Formatting: keep at most one consecutive blank line in code and declarations.
- Formatting: after a scope's closing brace, always insert a blank line before a following statement; omit it only when the next line closes the parent scope.
- Formatting: always insert a blank line before `return`, `continue`, and `break` unless the control-flow statement is the only statement in its scope.
- Formatting: preprocessor directives follow the same indentation rules as regular code; keep empty blocks compact.
- Formatting: simple `case` statements and embedded statements use line breaks instead of staying on one line.
- Formatting: in multiline declarations and calls, wrap after the opening parenthesis and before the closing parenthesis; wrap parameters, arguments, and braced init lists with `chop if long` behavior.
- Formatting: always wrap base clauses; wrap constructor/member initializer lists when long, with a line break after each comma.
- Formatting: put enum members on separate lines except when ReSharper keeps a short enum on one line.
- Formatting: align multiline ternary expressions; do not line break before ternary operator signs.
- Formatting: put `extern "C"`/linkage specification braces on the next line and keep the body indented.
- Do not introduce raw pointers for ownership, object references, or project-level API contracts.
- If a platform, third-party, or C ABI forces a raw pointer or raw handle, keep it isolated at the boundary and do not propagate it as normal project ownership.
- Prefer the project's object pointer types: `ego::IntrusivePointer<T>`/`TypePointer` declared through `EGO_INTRUSIVE_POINTER`, and `ego::SharedPointer<T>`/`TypePointer` plus `ego::WeakPointer<T>`/`TypeWeakPointer` declared through `EGO_POINTER`/`EGO_WEAK_POINTER`.
- Choose deliberately between `IntrusivePointer` and `Pointer`: prefer `IntrusivePointer` when its lifetime model fits, because the project shared pointer type has higher overhead.
- Create intrusive objects through `ego::MakeIntrusive`/`ego::MakeIntrusiveWithDeleter`; direct construction from a freshly allocated object does not bind its destroy function.
- Create shared objects through `ego::MakePointer`/`ego::MakePointerWithDeleter`; keep direct raw-pointer adoption isolated to API and ABI boundaries.
- Use project pointer types where they are already used by the surrounding code.
- If project pointer types do not fit the required ownership, lifetime, performance, or interoperability constraints, use standard C++ ownership tools (`std::unique_ptr`, `std::shared_ptr`, `std::weak_ptr`, references, `std::optional`, `std::reference_wrapper`) according to normal C++ ownership rules.
- For checks and invariants, use the existing macros `EGO_ASSERT`, `EGO_ASSERT_FAIL_MESSAGE`, and `EGO_CHECK_INITIALIZATION`.
- For template implementations, preserve the existing `.h` + `.hpp` pattern.

## Includes
- In source files that have a corresponding header, include that header first, in its own include group, before all other includes.
- Do not use relative include paths that climb above the including file's directory with `..`. A file name from the same directory or a path beginning with a subdirectory of the including file's directory is allowed.
- When adding or changing `#include` directives, keep them grouped with one blank line between groups and between component/module subgroups.
- Include groups are ordered as follows:
  1. Standard library headers.
  2. Ego project headers, grouped by dependency direction and the current component/module hierarchy: lower-level/base components first, then components that depend on them, then the current component, plugin, launcher, or demo. Treat all headers from the same component/module as one subgroup with no blank lines inside it; separate different component/module subgroups with one blank line.
  3. Third-party library headers.
- Keep includes within each group sorted consistently with the surrounding file.
- Do not move third-party includes above standard library or Ego project includes.
- Do not introduce a new include if a forward declaration is already sufficient and consistent with the surrounding code.

## Verification
- Do not add tests for a project or component that has no existing tests.
