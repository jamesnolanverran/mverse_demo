# Mverse

Mverse is a small experimental macro system for C.

You write C, add `@` macros where a bit of repetition or shape would help, and
Mverse expands those macros into ordinary C before compiling with `clang-cl`.

Mverse is currently Windows-only. The demos expect `clang-cl` to be available
on your `PATH`.

Start with this:

```c
@def(measure_time(label:char*)) {
    clock_t _start = clock();
    $body
    clock_t _end = clock();
    printf("%s took %f seconds\n", $label,
           (double)(_end - _start) / CLOCKS_PER_SEC);
}

@measure_time("heavy work") {
    // This block replaces $body in measure_time.
    do_heavy_math();
}
```

`@def` creates a macro. `$body` is replaced by the block you pass at the call
site. Parameters are referenced with `$name`, typed or not. Because
`label:char*` is typed, its argument is evaluated once and `$label` becomes a
generated local named `_label`.

Mverse expands the call above into ordinary C with this shape:

```c
{
    char *_label = "heavy work";
    clock_t _start = clock();

    // This block replaces $body in measure_time.
    do_heavy_math();

    clock_t _end = clock();
    printf("%s took %f seconds\n", _label,
           (double)(_end - _start) / CLOCKS_PER_SEC);
}
```

The generated output is ordinary C and can be inspected under the demo's
`build` directory.

## Demo

The demo folder contains four small examples of the currently working Mverse
language:

- `01_basics`: a typed macro with a `$body` block.
- `02_parsing`: a macro that iterates over lines and expands a nested call-site
  body.
- `03_foreach`: named arguments, default values, and the extra C block needed
  when a macro declares locals.
- `04_protocols`: register implementations for ordinary C types and select the
  correct function with an `@` protocol call.

From `D:\dev\mverse\demo`, run them from any Windows shell where `clang-cl` is
available:

```bat
cd 01_basics
build.bat

cd ..\02_parsing
build.bat

cd ..\03_foreach
build.bat

cd ..\04_protocols
build.bat
```

Each demo writes generated C under its own `build\` folder, plus source-map
files for debugger and editor tooling.

You can debug the expanded C with any normal native debugger by opening the
demo executable and its generated `build\main.c`. To debug the authored Mverse
source instead, use the experimental
[RAD Debugger fork](https://github.com/jamesnolanverran/raddebugger), which
reads the `.srcmap` and `.srcnav` files. The source-map experiment is described
in [native source maps](https://github.com/jamesnolanverran/native_source_maps).

For example, after building `01_basics`:

```bat
D:\dev\raddebugger\build\raddbg.exe --source-map:basics.srcmap basics.exe
```

Put `--source-map` before the executable name. RAD will use the matching
`basics.srcnav` file automatically when it is next to `basics.srcmap`.

## Where To Go

- [Getting Started](github_docs/getting-started.md): run the demos and see what
  files Mverse writes.
- [Macros By Example](github_docs/macros-by-example.md): learn `@def`, `$body`,
  parameters, defaults, named arguments, and varargs.
- [Patterns](github_docs/patterns.md): expression macros, statement macros,
  scoping, and the important extra-braces rule.
- [Emitters](github_docs/emitters.md): build output-oriented mini templates
  with `@emit`, `@def_emit`, and `%{...}` interpolation.
- [Other Features](github_docs/other-features.md): enum declarations,
  `@map_args`, protocol dispatch, and the current dynamic-array generator.
- [Tools](github_docs/tools.md): RAD Debugger support, the VS Code extension,
  `.srcmap`, and `.srcnav`.
- [Reference](github_docs/reference.md): the compact rules and current
  limitations.
- [C Type Integration](github_docs/type-integration.md): fine print and escape
  hatches for external types, aliases, wrappers, and conditional declarations.

Mverse is pre-alpha software. There will be bugs. Feel free to play with it and
let me know what you think.

If there is enough interest we will continue development.
