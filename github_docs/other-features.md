# Other Features

This page covers a few features just past everyday `@def` macros: vararg
mapping, protocol-like dispatch, and dynamic-array generation.

## Vararg Mapping

`@map_args` is specifically for vararg text.

When a macro captures `args...`, those arguments are available as text. A common
next move is to wrap each captured argument in the same function.

```c
@map_args("a, b, c", to_str)
```

expands conceptually to:

```c
to_str(a), to_str(b), to_str(c)
```

The first argument is the captured vararg text, shown here in quotes to make the
text boundary visible. The final argument is the wrapper.

This is not runtime iteration. It does not walk an array, split a string, or
inspect values. It operates on macro argument text at expansion time.

One common use is a vararg macro:

```c
@def(outf(fmt, args...)) {
    parse_output(
        to_str($fmt),
        (Str[]) { @map_args($args, to_str) },
        $va_count
    )
}
```

Here `$args` is the captured vararg text, and `@map_args` turns each argument
inside that text into a `to_str(...)` expression.

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
writes a C11 `_Generic` dispatch macro.

That turns into code with this shape:

```c
#define to_str(x) \
_Generic((x), \
    CharArr: (Str (*)(CharArr))str_impl_char_array, \
    int: (Str (*)(int))str_impl_int, \
    int*: (Str (*)(int*))str_impl_int_ptr, \
    unsigned long: (Str (*)(unsigned long))str_impl_ulong, \
    char*: (Str (*)(char*))str_impl_char, \
    Str: (Str (*)(Str))str_impl_str, \
    Str*: (Str (*)(Str*))str_impl_str_ptr, \
    StrView: (Str (*)(StrView))str_impl_str_view \
)(x)
```

So if the string definitions are imported, the codebase can grow a generic
`to_str(...)` operation one implementation at a time.

The split is pleasantly boring:

- Mverse collects the `@impl` rows and writes the `_Generic` macro.
- C chooses the implementation from the expression type.
- There is no runtime type system.

The generated macro still follows ordinary C rules. The implementation
functions need to be declared where the generated call is compiled, just as they
would if you wrote the `_Generic` macro by hand.

There is one practical catch: protocol collection happens while Mverse is
processing source and imported headers. If a library contributes `@impl` rows,
those rows need to live in code Mverse sees during the project build. A
precompiled object file can still link normally, but it cannot add new protocol
implementations to the generated `_Generic` macro.

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
