# Tools

Mverse writes normal generated C, but the best experience is to stay in the
source you authored. The tooling pieces in this repo are about keeping you
there.

## The Output Files

Alongside generated C, Mverse writes two source-facing files:

- `<program>.srcmap`: maps generated C locations back to the original Mverse
  source.
- `<program>.srcnav`: maps macro references to macro definitions.

You can open these files if you are curious, but they are mostly for tools.
They answer questions like:

```text
This instruction came from build/main.c:42.
Where was that in the source file I actually wrote?
```

and:

```text
This is a call to @measure_time.
Where is @measure_time defined?
```

## RAD Debugger

You can debug the generated C with ordinary native debugger behavior: open the
demo executable, use the PDB, and step through the generated `build\` files.

For authored-source debugging, this project has been tested with a modified
build of [RAD Debugger](https://github.com/jamesnolanverran/raddebugger) that
reads Mverse source maps. The broader source-map experiment is described in
[native source maps](https://github.com/jamesnolanverran/native_source_maps).

With the matching RAD build, you can debug at the Mverse level:

- breakpoints in authored source instead of generated `build\` files;
- source locations remapped from generated C back to your original files;
- stepping that skips macro-generated machinery where appropriate; and
- F12 / Ctrl-click style navigation from a macro call to its `@def` or
  `@def_emit` definition.

The debugger still uses normal native debug information from the C compiler.
The `.srcmap` file gives it the missing relationship between generated C and
authored Mverse source.

## VS Code Extension

The repository also includes a VS Code extension in `mverse/`.

It is lightweight editor support, not a language server. It currently provides:

- syntax highlighting for Mverse forms inside `.c` and `.h` files;
- highlighting for `@def`, `@def_emit`, `$body`, `$parameter`, `$va_count`, and
  `%{...}`;
- Go to Definition / Ctrl-click from `@macro` calls to macro definitions;
- indexing for native macro registrations; and
- a generated-output indicator for files under `build\`.

Open a folder containing `build.conf` and the extension activates automatically.
