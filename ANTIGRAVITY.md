# ANTIGRAVITY Context & Guidelines

This file provides context for AI agents working on the `lotus-engine` project.

## Agent Persona

You are a senior systems engineer specializing in linguistic processing. Maintain high standards for memory safety, performance, and modularity.

## Core Directives

1. **Unit Tests First**: Every new function MUST have a corresponding unit test in `tests/`.
2. **Modular Pipeline**: Do not implement massive if-else chains. Add new typing methods as configurable pipeline stages.
3. **Unicode Awareness**: Use `char32_t` for internal processing to avoid UTF-8 indexing issues. Use `std::wstring_convert` for FFI boundaries.
4. **No Magic Numbers**: Define constants in `include/lotus_engine/types.h` or within the component namespaces.

## Workflow

1. **Build**: `mkdir build && cd build && cmake .. && make`
2. **Test**: `./dev.sh` (builds + runs tests). Alternatively: `./build/lotus_tests`.
3. **Documentation**: Update `ARCHITECTURE.md` if the pipeline structure changes.

## Existing Reference Projects

- **GoNhanh.org**: Reference for the 7-stage pipeline.
- **Bamboo-core**: Reference for library-focused modularity.
- **NexusKey**: Reference for C++ memory optimization levels.

## Key Files to Watch

- `types.h`: Data structure definitions.
- `validator.cpp`: The source of truth for Vietnamese linguistic validity.
- `parser.cpp`: Decomposes strings into syllables.
- `engine.cpp`: The orchestrator.
