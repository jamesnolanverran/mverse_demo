# Demo

These examples show the currently working Mverse language. They are intended to
be run from Windows with `clang-cl` available on your `PATH`.

## Run

From `D:\dev\mverse\demo`:

```bat
cd 01_basics
build.bat

cd ..\02_parsing
build.bat

cd ..\03_foreach
build.bat
```

Each directory contains its own `build.conf`, `main.c`, and `build.bat`. The
batch file invokes the `mverse.exe` in the parent project directory.

## Examples

- `01_basics`: a typed macro with a `$body` block.
- `02_parsing`: a macro that iterates over lines and expands a nested call
  body. It reads the source-map CSV produced by the first demo.
- `03_foreach`: named arguments, default values, and the extra C block needed
  when a macro declares locals. It calls the macro twice to demonstrate why the
  block matters.

Mverse writes generated C and object files under each demo's `build\` folder.
It also writes the executable and the source-map CSV files in the demo
directory.

You can debug the generated C directly with any normal native debugger. If you
want the debugger to show authored Mverse source instead, try the experimental
[RAD Debugger fork](https://github.com/jamesnolanverran/raddebugger), which
reads Mverse `.srcmap` and `.srcnav` files.

The project is pre-alpha. The examples show implemented behavior, but the
language is still changing.
