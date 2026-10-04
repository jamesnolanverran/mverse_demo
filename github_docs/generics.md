# Generics

A container usually has one main type, a few supporting types, and a set of
functions that work with them. A Mverse generic lets you write that code once,
then specialize it for the element types you need.

The result is ordinary typed C: structs with fields you can access directly,
functions you can call by name, and convenient `@` calls that select the right
specialization. The aim is to keep the library code familiar and let Mverse
handle the repeated names and dispatch.

## A Small Generic

Here is a complete generic for a box that holds one value. Save it as `box.h`:

```c
#ifndef BOX_H
#define BOX_H

@def_generic(box) {
    @generic_operation(set)
    @generic_operation(get)

    @def_generic_emit(declaration) {
        typedef struct %{Self} {
            %{T} value;
        } %{Self};

        void @fn(set)(%{Self} *box, %{T} value);
        %{T} @fn(get)(%{Self} *box);
    }

    @def_generic_emit(definition) {
        void @fn(set)(%{Self} *box, %{T} value) {
            box->value = value;
        }

        %{T} @fn(get)(%{Self} *box) {
            return box->value;
        }
    }
}

#endif
```

The declaration template supplies the struct and function declarations. The
definition template supplies the function bodies. There is no separate list
of naming prefixes to keep in sync: `box` supplies the conventions, and the
operations name only the work they do.

### `T`, `Self`, And The Naming Helpers

For a specialization of `box` with `int`:

| Spelling | Meaning | Generated text |
| --- | --- | --- |
| `%{T}` | The supplied element type | `int` |
| `%{Self}` | The primary type for this specialization | `BoxInt` |
| `@fn(set)` | A function belonging to this specialization | `box_int_set` |
| `@type(Iterator)` | An auxiliary type belonging to this specialization | `BoxIntIterator` |

Mverse supplies the name of `Self`, not its fields or memory layout. Those
come from the C you write in the template. Here it holds one value; an array
can hold a buffer and a length, and a list can hold a pointer to its first node.

The naming helpers produce identifiers. They do not declare a type or define
a function on their own. `@fn` can also name private helper functions that do
not need a public operation.

These are generation-time templates: `%{T}` and `%{Self}` insert type
spellings, not runtime C expressions. That is different from `%{expression}`
in an ordinary [output emitter](emitters.md).

## Use The Generic

Declare specializations in a header, after importing the generic definition.
For this example, save the following as `types.h`:

```c
#ifndef TYPES_H
#define TYPES_H

@import("box.h")

@impl_box(int)

#endif
```

Then use the resulting type in `main.c`:

```c
@import("types.h")

int main(void) {
    BoxInt box = {0};
    @box_set(&box, 42);
    return @box_get(&box) == 42 ? 0 : 1;
}
```

`@box_set` selects the specialized function using the type of `&box`. You can
also call `box_int_set(&box, 42)` directly or read `box.value` like any other
C struct field.

One source file supplies the implementation anchor. Save this as `box.c`:

```c
@import("box.h")

@box_impl()
```

Build `main.c` and `box.c` as one expansion target. With `mverse.exe` and
`clang-cl` on your `PATH`, run these commands from the example directory:

```bat
mverse.exe --expand --target build\box.exe main.c box.c
clang-cl /nologo /Fe:build\box.exe build\main.c build\box.c
build\box.exe
```

Mverse collects the specializations across the target and emits their function
bodies at the anchor. You still compile and link the generated C normally.

### Selecting From An Element Pointer

Most operations select a function from a pointer to `Self`. An operation can
instead declare that it receives an element pointer:

```c
@generic_operation(
    from_fixed,
    selector=element_pointer,
    options=(as)
)
```

Such an operation accepts both `T *` and `const T *` sources. When compatible
typedef aliases make automatic selection ambiguous, `as=` names the exact
element specialization to use. For a generic `values` that declares the
operation above, that call reads:

```c
Values values = @values_from_fixed(source, 3, as=int);
```

`as=` belongs to operations that declare it; it is not an option on every
generic call.

## A Few Practical Rules

- Import the generic definition before specializing it, using its operations,
  or placing its anchor. The `box` example keeps the definition in `box.h`;
  a definition can also live in its own file.
- Put specializations at file scope in a header, after the element type is
  declared. Callers import that header before using the generated type.
- A call sees specializations introduced earlier in its own import stream.
  Adding a separate source file to the target does not make its imports
  visible everywhere.
- Each generic with specializations needs exactly one implementation anchor
  in the expansion target.
- Use `name=` when a specialization needs a particular public name, such as
  `@impl_box(int, name=BoxCount)`. Otherwise, the generic supplies the name.
- When templates need a shared runtime header, declare it with
  `reference_header="path/to/runtime.h"` and include
  `"%{reference_header_rel}"` in the declaration template. Mverse computes
  the generated header's relative include path.

Generated declarations and implementations live under `build/mverse_gen`.
For the box example, look for `box_int.h` and `box_impl.inc`. The owning
header includes the declarations, and the anchor includes the implementation.

Generics currently specialize one element type and dispatch through a wrapper
pointer or an element pointer. C remains responsible for checking the
resulting types and function calls.
