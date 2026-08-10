# Macros By Example

## A Macro With A Body

Define a macro with `@def`:

```c
@def(measure_time(label:char*)) {
    clock_t _start = clock();
    $body
    clock_t _end = clock();
    printf("%s took %f seconds\n", _label,
           (double)(_end - _start) / CLOCKS_PER_SEC);
}
```

Call it like this:

```c
@measure_time("Heavy Math Loop") {
    do_heavy_math();
}
```

The call-site block is substituted where the macro body says `$body`.

Because `label` is typed as `char*`, Mverse creates a local variable named
`_label` in the generated C. That means the expression passed as `label` is
evaluated once.

Conceptually, the expansion has this shape:

```c
{
    char* _label = "Heavy Math Loop";
    clock_t _start = clock();
    do_heavy_math();
    clock_t _end = clock();
    printf("%s took %f seconds\n", _label,
           (double)(_end - _start) / CLOCKS_PER_SEC);
}
```

That tiny example shows three core ideas:

- `@def(...) { ... }` defines a macro.
- `$body` inserts the block from the call site.
- A typed parameter like `label:char*` becomes a generated local named
  `_label`.

## Plain Text Parameters

Parameters do not have to be typed:

```c
@def(log_value(value)) {
    printf("value = %d\n", $value);
}

@log_value(42);
```

Here `$value` is replaced with the argument text:

```c
printf("value = %d\n", 42);
```

This is simple and flexible. It also means Mverse is not type-checking the
argument. The C compiler checks the generated C after expansion.

## One Macro Can Call Another

Macro expansion is recursive:

```c
@def(log_value(value)) {
    printf("value = %d\n", $value);
}

@def(log_twice(value)) {
    @log_value($value);
    @log_value($value);
}

@log_twice(42);
```

Mverse expands `@log_twice`, then expands the `@log_value` calls inside it.

## Defaults

A parameter can have a default:

```c
@def(foreach(type, arr, item_name=item, index_name=__mv_i)) {
    /* ... */
}
```

Typed parameters can have defaults too:

```c
@def(retry(count:int=3)) {
    for (int _i = 0; _i < _count; ++_i) {
        $body
    }
}
```

Now the call site can stay quiet when the defaults are fine:

```c
@foreach(int, &numbers) {
    printf("%d\n", *item);
}
```

Or it can name the parts that should read better locally:

```c
@foreach(int, &numbers, item_name=value, index_name=i) {
    printf("numbers[%zu] = %d\n", i, *value);
}
```

Defaults must come after required parameters.

## Named Arguments

Named arguments use `name=value`:

```c
@foreach(int, &numbers, item_name=value, index_name=i) {
    printf("%d\n", *value);
}
```

Named arguments must come after positional arguments. Once a call uses a named
argument, the rest of the arguments must be named too.

## Variable Arguments

A final parameter ending in `...` captures the remaining arguments:

```c
@def(report(args...)) {
    printf("received %d arguments\n", $va_count);
}

@report("a", "b", "c");
```

Inside the macro:

- `$args` is the captured argument text.
- `$va_count` is the number of captured arguments.

Varargs are especially useful with helpers such as `@map_args`, where you want
to transform captured argument text into generated C expressions.

## Imports

Use `@import` to load Mverse definitions from another file:

```c
@import("path/to/file.h")
```

Imported files are processed during macro collection. Their macro definitions
become available to later source files, and generated imported headers are
written under `build\`.
