# Other Features

This page covers a few features just past everyday `@def` macros: enum
declarations, vararg mapping, protocol-like dispatch, and dynamic-array
generation.

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

`enum/enum.h` imports the `Str` definitions it needs. Configure the jlibs root
as both an Mverse import path and a compiler include path, for example:

```text
mverse_include_paths = path/to/jlibs
include_path = /Ipath/to/jlibs
```

Combined flag formatting and conditional enumerator lists are not part of this
initial helper.

## Vararg Mapping

`@map_args` is specifically for vararg text.

When a macro captures `args...`, those arguments are available as text. A common
next move is to wrap each captured argument in the same ordinary C function.

```c
@map_args("a, b, c", encode_arg)
```

expands conceptually to:

```c
encode_arg(a), encode_arg(b), encode_arg(c)
```

The first argument is the captured vararg text, shown here in quotes to make the
text boundary visible. The final argument is the wrapper.

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

@emit_protocol(Str, to_str)
```

Each `@impl` says: for this operation, this C type is handled by this C
function. `@emit_protocol` collects the implementations Mverse has seen and
creates the `@to_str(...)` protocol call:

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

The current implementation uses the address of the argument for type selection.
Automatic protocol calls therefore require an addressable lvalue, normally a
named variable. Character-array lvalues, including string literals, retain the
usual string conversion behavior. This lets the generated table safely mention
forward-declared structs through pointers. Import-scoped expansion at each call
site remains a possible future design if the lvalue requirement or global
tables prove limiting in practice.

The implementation functions must be declared where the generated call is
compiled, just as with an ordinary C function call.

Named views and explicit authored-type selection use the same call syntax:

```c
Str normal = @to_str(size);
Str width = @to_str(size, view=width);
Str exact = @to_str(code, as=ErrorCode);
```

Prefer the `@` spelling in Mverse code, particularly when using `as=` or
`view=`. The generated ordinary `to_str(value)` C spelling also works for
automatic default dispatch and has the same lvalue requirement.

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

## Dynamic Arrays

The dynamic-array generator is built on the same collection-and-generation
idea.

The current macros are:

```c
@impl_darr(MyType)
@emit_darr_h()
@emit_darr_c()
```

`@impl_darr(MyType)` registers a type. The emit macros generate type-specific
array declarations and implementations.

The generated API includes `_Generic` dispatch for operations such as:

- `da_init`
- `da_push`
- `da_pop`
- `da_peek`
- `da_free`
- `from_fixed`

For example, registering `int` produces an `IntArr`-style type and matching
functions behind the generic `da_*` macros.
