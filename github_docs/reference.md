# Reference

This page is the compact version: what the current pre-alpha build does, what
syntax it accepts, and where the edges are.

## Build Flow

Mverse is currently Windows-only and builds generated C with `clang-cl`.

Run Mverse from a directory containing `build.conf`:

```text
mverse.exe
```

The current flow is:

1. Read `build.conf`.
2. Read configured source files and imported Mverse headers.
3. Collect macro definitions and type/protocol information.
4. Expand headers and source files.
5. Write generated files under `build\`.
6. Write `<program>.srcmap` and `<program>.srcnav`.
7. Compile generated C files with `clang-cl` and link the executable.

`build.conf` is required in the current directory. Paths are currently
interpreted relative to the directory where Mverse is run.

## Macro Definitions

```c
@def(name(arg, other:char*, count:int=3)) {
    /* body */
}
```

Supported parameter forms:

- `name`: textual parameter.
- `name:type`: typed parameter, emitted as a local named `_name`.
- `name=default`: defaulted parameter.
- `name:type=default`: typed parameter with a default.
- `args...`: final trailing vararg parameter.

Defaults must follow required parameters. A trailing vararg parameter captures
remaining arguments and exposes `$va_count`.

A macro may declare up to 32 fixed parameters. Definitions that exceed that
limit are rejected.

## Macro Calls

Supported call forms:

```c
@name(arg1, arg2);
@name(arg1, arg2) { /* body */ }
@name { /* body */ }
```

A call must have an argument list, a block body, or both.

Named arguments use `name=value` and must come after positional arguments.
Unknown and duplicate names are errors. Vararg macros do not currently combine
with named call arguments.

## Substitutions

Supported substitutions:

- `$parameter`: replace a named parameter.
- `$body`: replace the call-site block.
- `$va_count`: number of captured variable arguments.
- `$vararg_name`: captured variable-argument text.

Substitution is mostly textual. It is not a full C-aware rewrite.

## Expression And Statement Shape

Mverse does not classify macros as expressions or statements. It expands text,
then C decides whether that text is valid in context.

Untyped expression-shaped macros can be used in expressions:

```c
@def(add(a, b)) {
    (($a) + ($b))
}
```

Typed parameters generate local declarations, so typed macros are
statement-shaped:

```c
@def(show(value:int)) {
    printf("%d\n", _value);
}
```

That expands with a C block containing a generated local.

## Emitters

Native `@emit` form:

```c
@emit(emit_function, element_type, wrapper, extra_wrapper...) {
    literal text %{expression}
}
```

`@def_emit` defines a user macro and marks it as an emitter, which allows nested
emitter macros to compose.

Inside an emitter body, literal text is captured as output text and
`%{expression}` inserts a C expression. Its closing `}` is recognized only
after nested C braces have closed; strings, character literals, and comments
are also ignored while scanning. Nested interpolations are not supported.

## Imports

```c
@import("path/to/file.h")
```

Mverse searches configured `mverse_include_paths` and current source paths.
Imported files are processed during macro collection. Generated imported
headers are written under `build\` and included from generated C.

## Native Macros Included By This Build

Core Mverse features:

- `@def`
- `@def_emit`
- `@emit`
- `@import`
- `@impl`
- `@emit_protocol`

Other included features:

- `@map_args`
- `@impl_darr`
- `@emit_darr_h`
- `@emit_darr_c`

Native callbacks are registered by the current executable. A different Mverse
host can register a different set.

## Current Limitations

Mverse is not a hygienic macro system or complete C parser. Current limitations
include:

- No recursion or cycle detection for macros.
- Typed parameter names are generated as `_name` and are not automatically
  unique.
- Substitution is primarily textual.
- `$body` is the supported block mechanism; there are no first-class
  block-valued parameters.
- Type metadata used by protocol and array generation is early and limited.
