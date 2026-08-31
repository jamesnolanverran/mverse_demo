# Getting Started

The fastest way to understand Mverse is to run a demo and open the generated C.

## Requirements

Mverse currently expects:

- Windows.
- `clang-cl` available on your `PATH`.

The repository can be browsed from WSL, but the demo build scripts are Windows
batch files.

## Run The First Demo

From a Windows shell where `clang-cl` is available:

```bat
cd D:\dev\mverse\demo\01_basics
build.bat
```

From WSL, that same folder is:

```text
/mnt/d/dev/mverse/demo/01_basics
```

The first demo defines a timing macro and calls it with a block:

```c
@measure_time("Heavy Math Loop") {
    printf("  Inside the block!\n");
    do_heavy_math();
}
```

After the build, look in `build\main.c`. That file is the generated C that
`clang-cl` compiled. There is no hidden runtime or special object format: the
interesting part becomes C you can read with the same tools you already use.

You can debug that generated C like ordinary native code. To debug the authored
Mverse source instead, use the experimental
[RAD Debugger fork](https://github.com/jamesnolanverran/raddebugger), which
reads the source-map files Mverse writes.

For `01_basics`, that looks like:

```bat
D:\dev\raddebugger\build\raddbg.exe --source-map:basics.srcmap basics.exe
```

## Run The Other Demos

From `D:\dev\mverse\demo`:

```bat
cd 02_parsing
build.bat

cd ..\03_foreach
build.bat

cd ..\04_protocols
build.bat
```

The demos are intentionally small:

- `01_basics`: a typed macro with a `$body`.
- `02_parsing`: a macro that walks lines of text and runs a call-site body for
  each line.
- `03_foreach`: default arguments, named arguments, and the extra C block used
  by statement-like macros.
- `04_protocols`: attach behavior to ordinary C types and let an `@` call select
  the right implementation.

## How The Demo Build Works

You can treat `build.bat` as the whole interface while you are learning Mverse.
It expands the source, compiles the generated C, and runs the result.

<details>
<summary>See the commands and build responsibilities</summary>

The command-line interface stays deliberately small:

```text
mverse.exe --expand --target <output.exe> [-I <mverse-import-path> ...] <source.c>...
mverse.exe --remap-diagnostics --target <output.exe> <diagnostic-log>
mverse.exe --help
mverse.exe --version
```

For example, the first half of `01_basics\build.bat` is essentially:

```bat
..\mverse.exe --expand --target basics.exe main.c
clang-cl build\main.c /Fe:basics.exe
```

`--target` tells Mverse the name of the output the surrounding build will
produce. Mverse uses that name for `basics.srcmap` and `basics.srcnav`; it does
not create the executable itself. Positional arguments are authored source
files. Repeatable `-I` options add search paths for Mverse `@import` files.

The compiler command is intentionally outside Mverse. Your script or build
system owns compiler flags, libraries, linking, incremental builds, and the
compiler's exit status. If compilation fails, the demo captures the compiler
output and asks `--remap-diagnostics` to translate generated locations back to
the authored source.

</details>

## Generated Files

Mverse writes generated C under `build\`. The demo's compiler command keeps its
object file and diagnostic log there too.

It also writes:

- `<program>.srcmap`, mapping generated C locations back to authored source.
- `<program>.srcnav`, mapping macro calls to macro definitions for navigation
  tools.

Generated files are useful to inspect. They are also build output, so edit the
source files outside `build\` and run Mverse again.

For the precise command-line contract, see [Reference](reference.md). For
debugger and editor support built around these files, see [Tools](tools.md).
