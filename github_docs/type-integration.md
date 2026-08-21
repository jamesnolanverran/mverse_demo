# C Type Integration: Fine Print

Most Mverse code does not need this page. Declare a type in an imported header,
register its implementation, and let Mverse generate the protocol or array
code:

```c
typedef struct Point {
    int x;
    int y;
} Point;

@impl(to_str, Point, point_to_str)
```

The details below matter when a type comes from a traditional C include, when
several typedef names refer to one C type, or when the C preprocessor controls a
declaration.

## Traditional includes and `@external_type`

Mverse examines project headers reached through `@import`, but it does not try
to recursively understand every system and third-party header. A normal
`#include` remains C compiler territory:

```c
#include <third_party.h>

@external_type(ThirdPartyType)
@impl(to_str, ThirdPartyType, third_party_to_str)
```

`@external_type` means:

- ordinary C code supplies this type;
- Mverse may use the spelling in generated code;
- Mverse must not emit a replacement declaration; and
- the C compiler validates that the type exists and is complete wherever the
  generated code requires a complete type.

It does not include a header automatically and does not tell Mverse anything
about fields, size, alignment, aliases, or compatibility with another type.

The current accepted forms are:

```c
@external_type(ThirdPartyType)
@external_type(struct ThirdPartyRecord)
@external_type(union ThirdPartyValue)
```

Register the base type once. Pointer uses of that type then work as well.
External enum tags and more complicated registration spellings are not
currently supported.

This is particularly useful for an anonymous typedef from an external header.
Mverse cannot safely recreate its declaration, but it does not need to when the
real header is already included:

```c
// third_party.h contains: typedef struct { ... } ThirdPartyValue;
#include <third_party.h>
@external_type(ThirdPartyValue)
```

## External typedef aliases and `_Generic`

C typedefs are aliases, not distinct runtime types:

```c
typedef int ExtErrCode;
```

`ExtErrCode` and `int` are compatible. A C `_Generic` expression cannot contain
one association for `int` and another for `ExtErrCode`; the compiler rejects the
duplicate compatible associations rather than selecting the first one.

When Mverse parsed the typedef itself, it knows the two registrations belong to
one compatibility family. It can make automatic dispatch require an explicit
selection:

```c
@to_str(code, as=ExtErrCode)
@to_str(number, as=int)
```

An opaque `@external_type(ExtErrCode)` does not reveal that relationship. If a
protocol registers both `int` and the external alias in the same view,
automatic `_Generic` dispatch can therefore produce a compiler error. An
explicit Mverse call with `as=` selects its exact registration directly.

For a simple external alias, C itself provides a useful assertion.

## Repeating a typedef as an assertion

C11 permits a typedef name to be repeated in the same scope when it denotes the
same, non-variably-modified type. A wrapper header can use that rule to tell both
Mverse and the compiler what an external alias must mean:

```c
#include <third_party.h>

// The external header is expected to define ExtErrCode as int.
typedef int ExtErrCode;

@impl(to_str, int, int_to_str)
@impl(to_str, ExtErrCode, error_code_to_str)
```

Mverse sees the ordinary typedef and learns that `ExtErrCode` belongs to the
`int` compatibility family. The compiler verifies the assertion. If a later
version of the dependency changes `ExtErrCode` to `long`, compilation stops
instead of silently using incorrect dispatch.

In this situation, use the repeated typedef instead of `@external_type`; it
provides stronger information. Do not attempt to reproduce an anonymous struct
or another declaration whose identity cannot be repeated.

## Creating a genuinely distinct type

Another typedef never creates a distinct C type. If automatic dispatch must
distinguish two values without `as=`, a struct wrapper creates a nominally
different type:

```c
typedef struct WrappedExtErrCode {
    ExtErrCode value;
} WrappedExtErrCode;
```

This changes storage and call sites, so use it only when the domain distinction
should be represented in the C type itself. Protocol `view=` selects behavior;
it does not make a typedef nominally distinct. See
[`other-features.md`](other-features.md) for the normal protocol rules.

## Conditional declarations and `#if`

Mverse currently expands before the C preprocessor and does not evaluate the
active state of ordinary `#if`, `#elif`, or `#else` branches. Conditional type
declarations are therefore not currently safe sources of inferred facts for
protocol or array generation:

```c
#if USE_SMALL_IDS
typedef int Id;
#else
typedef long Id;
#endif
```

Likewise, do not expect an `@impl` inside a C `#if` to be conditionally collected
by Mverse. The C preprocessor has not selected the branch when Mverse performs
its collection pass.

A type hidden inside a traditionally included third-party header is different:
Mverse can treat it as opaque with `@external_type`, while the compiler processes
that header normally. The selected build configuration must still make the type
available wherever generated code uses it.

The complete conditional design has not been chosen. Possible future directions
include tracking unknown C preprocessor regions and failing clearly, adding
Mverse-owned conditionals driven by explicit build values, or asking Clang to
preprocess a separate analysis input with the exact compile configuration.
None of these approaches is promised yet.

For now, keep C-preprocessor-controlled type identities out of protocol and
array registration, or place the dependency behind an opaque external-type
boundary.

## Practical order of escape hatches

When Mverse cannot find or safely use a type declaration, prefer:

1. Put a normal project-owned declaration in a header reached through
   `@import`.
2. For a type owned by a traditional include, use `@external_type`.
3. For a simple external alias whose underlying type Mverse must know, repeat
   the typedef as a compiler-checked assertion.
4. Use `as=Type` when compatible aliases intentionally have different protocol
   implementations.
5. Use a struct wrapper only when the C type itself should be distinct.

If none of these expresses the real contract, stop rather than inventing a
declaration that merely happens to compile.
