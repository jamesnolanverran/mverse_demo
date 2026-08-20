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

## Expansion Recursion

Mverse rejects an active macro invocation that repeats with the same macro,
argument text, and call-site block. The error includes an expansion trace.
Finite nesting remains valid when the arguments or block differ.

Changing recursion is protected by a maximum active expansion depth of 128. A
custom Mverse build can override the limit with
`/DMVERSE_MAX_EXPANSION_DEPTH=<number>` when compiling `mverse.exe`; compiler
flags for the generated target do not alter a prebuilt Mverse executable.

Import deduplication is separate and is not treated as macro recursion.

### Repeated evaluation warning

If an untyped parameter is substituted more than once, Mverse warns at the
macro call when its argument contains an apparent call, assignment, increment,
or decrement. The check ignores strings, character literals, and comments. It
is a useful lexical warning, not a promise that Mverse understands all C side
effects. Typed parameters are evaluated once into generated locals and are not
subject to the warning.

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

- `$parameter`: replace a named parameter. Typed parameters use the same
  spelling; Mverse substitutes their generated `_parameter` local.
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
    printf("%d\n", $value);
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

## External C Types

```c
#include <third_party.h>
@external_type(ThirdPartyType)
```

`@external_type` tells Mverse that an ordinary C include supplies a type needed
by generated protocol or array code. It does not declare, parse, or reproduce
the type; the C compiler checks that the include actually provides it.

The accepted forms are:

```c
@external_type(ThirdPartyType)
@external_type(struct ThirdPartyType)
@external_type(union ThirdPartyType)
```

Register the base name once; pointer uses of that type are then accepted as
well. Enum tags and more complicated declaration spellings are not currently
supported by this macro.

The ordinary path ends there. External aliases, repeated typedef assertions,
wrapper types, and the unresolved `#if` questions are documented separately in
the [C Type Integration fine print](type-integration.md).

## Enum Convenience

With the jlibs root configured as an Mverse and compiler include path:

```c
@import("enum/enum.h")

@enum(Status) {
    STATUS_READY,
    STATUS_BUSY = 10,
    STATUS_DONE = 20,
}
```

This emits `Status` and the header-safe function
`to_str_from_Status(Status)`. Known values return their exact enumerator name;
unknown values return `"<unknown Status>"`.

Each entry is an identifier with an optional C value expression. Values must
resolve uniquely because the generated conversion has one name per value. Flag
formatting and conditional enumerator lists are not currently supported.

Call the generated function directly. Registering an enum in a `_Generic`
protocol can conflict with its compatible C integer type.

## Native Macros Included By This Build

Core Mverse features:

- `@def`
- `@def_emit`
- `@emit`
- `@import`
- `@impl`
- `@emit_protocol`
- `@external_type`

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

- Typed parameter names are generated as `_name` and are not automatically
  unique.
- Substitution is primarily textual.
- `$body` is the supported block mechanism; there are no first-class
  block-valued parameters.
- Type metadata used by protocol and array generation is early and limited.
