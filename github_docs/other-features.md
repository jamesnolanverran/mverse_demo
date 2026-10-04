# Other Features

This page covers a few features just past everyday `@def` macros: enum
declarations, vararg mapping, protocol dispatch, and generic containers.

## Enums With String Names

Import the enum convenience from jlibs, then declare the enum once:

```c
@import("enum/enum.h")

@enum(Color) {
    COLOR_RED,
    COLOR_GREEN = 10,
    COLOR_BLUE = 20,
}
```

This emits the ordinary C enum and a header-safe conversion function:

```c
Str name = to_str_from_Color(COLOR_GREEN);
```

The returned text is the exact enumerator name, such as `"COLOR_GREEN"`. An
unknown value returns `"<unknown Color>"`.

Enumerator values may be implicit or use ordinary C integer constant
expressions. The expressions remain C: Mverse preserves them and the compiler
validates and evaluates them. A trailing comma and comments are allowed.

Mverse enums require unique values. The generated conversion has one result
for each value, so aliases such as two names both equal to `7` are rejected by
the C compiler. If aliases are important, use an ordinary C enum and define the
desired conversion behavior yourself.

The generated function is `static inline`, so an `@enum` declaration may live
in a header included by several translation units.

The converter is intentionally called directly. Adding an enum to a shared
`_Generic` protocol table can conflict with the integer type that C considers
compatible with that enum. Direct `to_str_from_Color(value)` calls avoid that
ambiguity.

`enum/enum.h` imports the `Str` definitions it needs. Give the jlibs root to
both Mverse and the compiler, for example:

```bat
mverse.exe --expand --target colors.exe -I path\to\jlibs main.c
clang-cl /Ipath\to\jlibs build\main.c /Fe:colors.exe
```

Combined flag formatting and conditional enumerator lists are not part of this
initial helper.

## Vararg Mapping

`@map_args` is specifically for vararg text.

When a macro captures `args...`, those arguments are available as text. A common
next move is to map each captured argument through the same operation.

```c
@map_args(a, b, c, encode_arg)
```

expands conceptually to:

```c
encode_arg(a), encode_arg(b), encode_arg(c)
```

The final argument is the mapper name. A bare name always produces ordinary C
calls as shown above. Prefixing it with `@` explicitly requests a registered
Mverse macro. For example, `@map_args(6, make_value(), @to_str)` applies the
visibility-aware `@to_str(...)` protocol call to both rvalues.

This is not runtime iteration. It does not walk an array, split a string, or
inspect values. It operates on macro argument text at expansion time.

For example, a vararg macro can apply a normal conversion function to every
argument:

```c
@def(write_all(args...)) {
    write_values(
        (Value[]) { @map_args($args, encode_arg) },
        $va_count
    );
}
```

Here `$args` is the captured vararg text, and `@map_args` turns each argument
inside that text into an `encode_arg(...)` expression.

## Protocol Dispatch

Mverse includes a protocol-like mechanism, inspired by Elixir protocols.

The useful version looks like this:

```c
@impl(to_str, int, str_impl_int)
@impl(to_str, int*, str_impl_int_ptr)
@impl(to_str, unsigned long, str_impl_ulong)
@impl(to_str, char*, str_impl_char)
@impl(to_str, Str, str_impl_str)
@impl(to_str, Str*, str_impl_str_ptr)
@impl(to_str, StrView, str_impl_str_view)
```

Each `@impl` says: for this operation, this C type is handled by this C
function. The first `@impl` for a protocol introduces its callable
`@to_str(...)` macro; each later `@impl` adds another implementation:

```c
int number = 42;
Str text = @to_str(number);
```

If the string definitions are imported, the codebase can grow the operation one
implementation at a time.

The split is pleasantly boring:

- Mverse collects the `@impl` rows and writes the C11 type-selection code.
- C chooses the implementation from the expression type.
- There is no runtime type system.

Native protocol calls select directly on the expression, so named variables,
literals, and function results all work. At each call, Mverse includes only
the implementations visible through imports that appear before it. An
unrelated by-value struct registered elsewhere therefore does not need to be
complete at this call site.

The generated dispatcher names each implementation function directly, so the
selected call has the real type of that function. C checks the selected call
in its actual use context; implementations may return different types. The
functions must be declared where the generated protocol code is compiled, just
as with ordinary C function calls.

Named views and explicit authored-type selection use the same call syntax:

```c
Str normal = @to_str(size);
Str width = @to_str(size, view=width);
Str exact = @to_str(code, as=ErrorCode);
```

Protocol calls use the Mverse spelling, such as `@to_str(value)`.

The string library keeps protocol conversion and template collection distinct:
`@to_str(value)` is protocol dispatch, while `@to_string(result){...}` is the
body emitter. Mverse rejects macro, protocol, and emitter name collisions.

There is one practical catch: protocol collection happens while Mverse is
processing source and imported headers. If a library contributes `@impl` rows,
those rows need to live in code Mverse sees during the project build. A
precompiled object file can still link normally, but it cannot add new protocol
implementations to the generated `_Generic` macro.

If a protocol uses a type declared only by a traditional C include, register
that spelling explicitly instead of asking Mverse to parse the external include
tree:

```c
#include <third_party.h>
@external_type(ThirdPartyType)
@impl(to_str, ThirdPartyType, third_party_to_str)
```

The include remains responsible for the declaration. Mverse merely accepts the
spelling and leaves validation to the C compiler.

For external aliases and conditional declarations, see the
[C Type Integration fine print](type-integration.md).

## Generic Containers

Generics let a library supply a primary type and its functions once,
then specialize them for different element types. Mverse handles the repeated
names and operation dispatch; the library supplies the ordinary C structure
and algorithms.

For example, after importing a header that specializes the `box` generic from
the [Generics guide](generics.md) for `int`:

```c
BoxInt box = {0};
@box_set(&box, 42);
int value = @box_get(&box);
```

`BoxInt` is a normal C struct, and the operations select typed functions such
as `box_int_set`. Library authors use `%{T}` for the element type,
`%{Self}` for the primary type, and `@fn`/`@type` to name functions and
supporting types.

The [Generics guide](generics.md) walks through a complete
small generic, its specialization, and the implementation anchor.
