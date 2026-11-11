# 65816-Assembly-TMlanguage
tmLanguage file for 65816 Assembly

## For Sublime Text
To install
- open up Sublime Text 2
- go to Preferences → Browse Packages (on Mac, it would be Sublime Text 2 → Preferences → Browse Packages)
- copy the 65816ASM__tmLANGUAGE__forSUBLIMEtext' folder from this git repo to that folder
- enjoy your syntax highlighting!

## for VSCode
- open the Command Palette (Ctrl or Cmd + Shift + P)
- type "Developer: Install Extension from Location" 
- select the "vscode__65816ASM" folder
- you may need to re-start VSCode
- enjoy syntax highlighting

### SNES register hover documentation

The VS Code extension also shows register descriptions, access types, bit diagrams,
and links to the SNESdev Wiki when you hover over a register name or a direct
register address. Select **65816 Assembly** as the file's language mode.
After updating an extension installed from this folder, run **Developer: Reload Window**.
Typing `CG` offers names such as `CGADD` and `CGDATA`; typing `PPU` searches the
PPU register category and inserts the chosen register's actual canonical name.
Completion suggestions include the same descriptions and diagrams as hovers.
Use **Trigger Suggest** from the command palette if suggestions do not open automatically.

The documented definitions live in `vscode__65816ASM/data/registers.inc`.
Their uppercase register names and addresses follow the
[PPU register reference](https://snes.nesdev.org/wiki/PPU_registers) and
[complete MMIO register table](https://snes.nesdev.org/wiki/MMIO_register_table).
The initial set covers the 57 register addresses from SUPER ACTION 52's
`src/__inc/registers.inc`. SNESdev's reference diagrams are CC0 Public Domain.

The extension also reads `registers.inc` files in the current workspace, including
unsaved edits. A comment block immediately above an `.EQU` supplies Markdown hover
documentation; use a fenced `text` block for aligned bit diagrams. Current-file
`.EQU` and `.DEFINE` aliases can refer to a numeric value or another symbol.
Symbols are case-sensitive, matching WLA-DX.

```asm
; My palette output port; inherits CGDATA's register documentation.
.EQU PALETTE_OUTPUT $2122
    sta PALETTE_OUTPUT
```

`sta $2121` and `sta CGADD` show CGADD help. `lda #$2121` shows only an immediate
number; `lda #CGADD` retains the symbol documentation and explains that it uses
the value without accessing the register. Raw numbers in comments, strings,
data directives, indexed or indirect operands, and compound address expressions
do not receive register cards. Symbolic operands in expressions retain their
documentation, with a note that the effective address depends on context.
16-bit addresses assume a SNES I/O data bank; explicit 24-bit addresses are
checked against the I/O bank mirrors. This is a documentation provider, not an
assembler or a CPU-state analyzer; macro expansion and arbitrary constant
expressions are not evaluated.

To update the bundled copy from your game's documented register file:

```sh
cd vscode__65816ASM
npm run sync-registers -- /path/to/superaction52__games/src/__inc/registers.inc
npm test
```

Workspace copies take priority over the bundled definitions. These editor
definitions do not add symbols to your assembly program: include `registers.inc`
in the source when you want to use its symbolic constants in assembled code.
