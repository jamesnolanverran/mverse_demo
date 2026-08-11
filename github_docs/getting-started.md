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
`clang-cl` compiled.

You can debug that generated C like ordinary native code. To debug the authored
Mverse source instead, use the experimental
[RAD Debugger fork](https://github.com/jamesnolanverran/raddebugger), which
reads the source-map files Mverse writes.

## Run The Other Demos

From `D:\dev\mverse\demo`:

```bat
cd 02_parsing
build.bat

cd ..\03_foreach
build.bat
```

The demos are intentionally small:

- `01_basics`: a typed macro with a `$body`.
- `02_parsing`: a macro that walks lines of text and runs a call-site body for
  each line.
- `03_foreach`: default arguments, named arguments, and the extra C block used
  by statement-like macros.

## The Command

The command-line interface is deliberately tiny:

```text
mverse.exe              build ./build.conf
mverse.exe --help       show usage
mverse.exe --version    show version information
```

Run `mverse.exe` from a directory that contains `build.conf`.

## Build Configuration

A minimal `build.conf` looks like this:

```text
source_files = main.c
compiler_flags = -DDEBUG -Zi
include_path =
disable_warnings =
link_flags =
executable = demo.exe
project_dir = .
```

The most important fields are:

| Key | Purpose |
| --- | --- |
| `source_files` | C and header files Mverse should process. |
| `compiler_flags` | Flags passed to `clang-cl`. |
| `include_path` | Include flags passed to `clang-cl`. |
| `link_flags` | Linker flags. |
| `executable` | Output executable name. |
| `mverse_include_paths` | Search paths for `@import`. |
| `precompiled_objs` | Object files to include during linking. |

Paths are currently interpreted from the directory where Mverse is run. Mverse
does not search parent directories for `build.conf`.

## Generated Files

Mverse writes generated C and object files under `build\`.

It also writes:

- `<program>.srcmap`, mapping generated C locations back to authored source.
- `<program>.srcnav`, mapping macro calls to macro definitions for navigation
  tools.

Generated files are useful to inspect. They are also build output, so edit the
source files outside `build\` and run Mverse again.

For debugger and editor support built around these files, see
[Tools](tools.md).
