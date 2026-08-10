# Emitters

Some macros want to act like a tiny templating language. They generate
formatted text, HTML, code fragments, or other output that is awkward to build
out of escaped C strings.

That is what `@emit` is for.

## A Tiny Output Macro

First, imagine an ordinary C helper:

```c
void print_strings(Str *items, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        fwrite(items[i].str, 1, items[i].len, stdout);
    }
}
```

Now define a friendly Mverse wrapper:

```c
@def_emit(printc(prefix)) {
    @emit(print_strings, Str, to_str) {
        %{$prefix}$body
    }
}
```

Use it like this:

```c
const char *name = "Mverse";

@printc("message: ") {
    Hello, %{name}!
}
```

You can write quotes and punctuation naturally:

```c
@printc("message: ") {
    "Hello %{name}", she said wearily.
}
```

No backslash maze. The text stays readable, and syntax highlighters often do a
reasonable job because the body still looks like the thing you are producing.

Conceptually, the first call becomes ordinary C:

```c
print_strings((Str[]) {
    to_str("message: "),
    to_str("Hello, "),
    to_str(name),
    to_str("!\n")
}, 4);
```

The source reads like the output you wanted, while the generated C still calls
plain C functions. It feels like a small template, but it compiles as ordinary
C.

## Literal Text And Interpolation

Inside an `@emit` body:

- Write literal text as itself.
- Use `%{expression}` to insert a C expression.
- Let the wrapper, such as `to_str`, convert each piece before output.

The native form is:

```c
@emit(emit_function, element_type, wrapper, extra_wrapper...) {
    literal text %{expression}
}
```

The interpolation scanner is intentionally simple right now. It stops at the
first `}`, so keep `%{...}` expressions free of nested braces.

## Why `@def_emit` Exists

`@def_emit` defines a macro and marks it as an emitter.

That marker matters when emitters are nested. It tells Mverse that a macro is
part of the output template, not plain text inside it.

For example, an HTML-shaped layer can be built out of smaller emitters:

```c
@def_emit(html()) {
    @emit(write_segments, Str, to_str, html_encode) {
        $body
    }
}

@def_emit(page()) {
    @html {
        <div class="page-container">
            $body
        </div>
    }
}

@def_emit(h1(extra=)) {
    @html {
        <h1 class="$extra">$body</h1>
    }
}
```

Then application code can keep the same shape as the output:

```c
@page {
    @h1 { Notes }
}
```

The expansion path is:

```text
@page
  -> @html
      -> @emit
          -> write_segments(Str[], count)
```

That is the emitter pattern: define one low-level output macro, then build a
small vocabulary on top of it.
