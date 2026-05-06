# CS-346 Mini-Compiler (Labs 06–12 integrated)

This repository implements a single end-to-end mini-compiler pipeline:

- Lexer → Parser → AST
- Semantic analysis (type + scope checking)
- Three-Address Code (TAC) generation
- Optimisation passes (at least 3)
- Optional C emission for LLVM IR generation via `clang` (if installed)

It also includes a separate LL(1) tool for FIRST/FOLLOW + parsing table construction.

## Prerequisites (Windows)
- CMake
- MinGW-w64 `g++` (or another C++17 compiler)

Optional (for coursework deliverables):
- `win_flex` and `win_bison` (or MSYS2 `flex`/`bison`) to generate scanners/parsers from files in `grammar/`
- `clang` to produce `.ll` LLVM IR files from emitted C

## Build (CMake)
From PowerShell in the project folder:

```powershell
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```

## Run
### 1) Full compiler pipeline
Compile a sample program and print tokens/AST/TAC/optimisations:

```powershell
./build/compiler.exe --input samples/demo1.ccp --dump-tokens --dump-ast --dump-tac --opt
```

Emit C for Clang:

```powershell
./build/compiler.exe --input samples/demo1.ccp --emit-c out.c
```

If you have Clang installed, generate LLVM IR:

```powershell
clang out.c -S -emit-llvm -o out.ll
clang out.c -S -emit-llvm -O3 -o out_O3.ll
```

### 2) LL(1) FIRST/FOLLOW + table tool

```powershell
./build/ll1_tool.exe --grammar samples/ll1_grammar.txt --table
```

### 3) Postfix evaluator demo

```powershell
"4 8 + 3 *" | ./build/postfix_eval.exe
```

## Input language (mini)
File extension used in this repo: `.ccp`.

Supported constructs:
- Declarations: `int x;`, `float y;`
- Assignment: `x = 3 + 4 * 2;`
- Blocks: `{ ... }` or `begin ... end`
- `if (cond) { ... } else { ... }`
- `while (cond) { ... }`
- `return expr;`
- Expressions: `+ - * / ^`, parentheses, identifiers, integer/float literals, `log(expr)`, `exp(expr)`

## Notes
- The compiler frontend here is hand-written so the project builds even without Flex/Bison.
- Coursework `.l`/`.y` deliverables are provided under `grammar/`.
