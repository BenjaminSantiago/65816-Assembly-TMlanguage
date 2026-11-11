; SNES hardware register definitions and hover documentation.
; Names and addresses checked against SNESdev on 2026-11-10.
; https://snes.nesdev.org/wiki/PPU_registers
; https://snes.nesdev.org/wiki/MMIO_register_table
; Reference diagrams: SNESdev Wiki (CC0 Public Domain).
; Comment blocks immediately above .EQU definitions also supply VS Code hovers.
; 16-bit CPU addresses require a data bank that maps SNES I/O.

; Control forced blanking and screen brightness. F=1 blanks the screen; B=0-15 selects brightness. $0F displays at full brightness; $8F forces blank while keeping the brightness setting.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; F... BBBB
; |    ||||
; |    ++++- Screen brightness (linear steps from 0 = none to $F = full)
; +--------- Force blanking
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#INIDISP)
.EQU INIDISP      $2100

; OBJ sprite size (S), name secondary select (N), name base address (B). The base selects a 16 KiB VRAM region; b was reserved and is normally 0. The name select chooses the offset of the second 8 KiB of sprite tiles. Also called OBSEL in other references.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; SSSN NbBB
; |||| ||||
; |||| |+++- Name base address (word address = bBB << 13)
; |||+-+---- Name select (word offset = (NN+1) << 12)
; +++------- Object size:
;             0:  8x8  and 16x16
;             1:  8x8  and 32x32
;             2:  8x8  and 64x64
;             3: 16x16 and 32x32
;             4: 16x16 and 64x64
;             5: 32x32 and 64x64
;             6: 16x32 and 32x64
;             7: 16x32 and 32x32
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#OBJSEL)
.EQU OBJSEL       $2101

; Tilemap tile size (#), BG3 priority (P), BG mode (M).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; 4321 PMMM
; |||| ||||
; |||| |+++- BG mode (see below)
; |||| +---- Mode 1 BG3 priority (0 = normal, 1 = high)
; |||+------ BG1 character size (0 = 8x8¹, 1 = 16x16)
; ||+------- BG2 character size (0 = 8x8¹, 1 = 16x16)
; |+-------- BG3 character size (0 = 8x8¹, 1 = 16x16)
; +--------- BG4 character size (0 = 8x8, 1 = 16x16)
; ```
;
; ```text
; BG Modes
; Mode| BG bit depth  |Offsets |     Priorities (front -> back)       |                     Notes
;     |BG1 BG2 BG3 BG4|per tile|                                      |
;  0  | 2   2   2   2 |   No   |   S3 1H 2H S2 1L 2L S1 3H 4H S0 3L 4L|
;  1  | 4   4   2     |   No   |   S3 1H 2H S2 1L 2L S1 3H    S0 3L   |BG3 priority = 0
;     |               |        |3H S3 1H 2H S2 1L 2L S1       S0 3L   |BG3 priority = 1
;  2  | 4   4         |  Yes   |   S3 1H    S2 2H    S1 1L    S0 2L   |
;  3  | 8   4         |   No   |   S3 1H    S2 2H    S1 1L    S0 2L   |
;  4  | 8   2         |  Yes   |   S3 1H    S2 2H    S1 1L    S0 2L   |
;  5  | 4   2         |   No   |   S3 1H    S2 2H    S1 1L    S0 2L   |Fixed 16 pixel char width. Forced high-res mode.
;  6  | 4             |  Yes   |   S3 1H    S2       S1 1L    S0      |Fixed 16 pixel char width. Forced high-res mode.
;  7  | 8             |   No   |   S3       S2       S1 1L    S0      |Fixed 8x8 char size.
; 7EXT| 8   7         |   No   |   S3       S2 2H    S1 1L    S0 2L   |Fixed 8x8 char size. BG2 bit 7 acts as priority.
;
; S0, S1, S2, S3 = Sprites, with OAM priority bits set to 0, 1, 2, or 3
; 1L, 2L, 3L, 4L = Background layer with per-tile priority bit set to 0
; 1H, 2H, 3H, 4H = Background layer with per-tile priority bit set to 1
; ```
;
; In modes 5 and 6, characters and offset-per-tile entries are always 16 pixels wide.
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BGMODE)
.EQU BGMODE       $2105

; Tilemap VRAM address (A), vertical tilemap count (Y), horizontal tilemap count (X).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; AAAA AAYX
; |||| ||||
; |||| |||+- Horizontal tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; |||| ||+-- Vertical tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; ++++-++--- Tilemap VRAM address (word address = AAAAAA << 10)
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BGnSC)
.EQU BG1SC        $2107

; Tilemap VRAM address (A), vertical tilemap count (Y), horizontal tilemap count (X).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; AAAA AAYX
; |||| ||||
; |||| |||+- Horizontal tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; |||| ||+-- Vertical tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; ++++-++--- Tilemap VRAM address (word address = AAAAAA << 10)
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BGnSC)
.EQU BG2SC        $2108

; Tilemap VRAM address (A), vertical tilemap count (Y), horizontal tilemap count (X).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; AAAA AAYX
; |||| ||||
; |||| |||+- Horizontal tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; |||| ||+-- Vertical tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; ++++-++--- Tilemap VRAM address (word address = AAAAAA << 10)
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BGnSC)
.EQU BG3SC        $2109

; Tilemap VRAM address (A), vertical tilemap count (Y), horizontal tilemap count (X).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; AAAA AAYX
; |||| ||||
; |||| |||+- Horizontal tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; |||| ||+-- Vertical tilemap count (0 = 1 tilemap, 1 = 2 tilemaps)
; ++++-++--- Tilemap VRAM address (word address = AAAAAA << 10)
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BGnSC)
.EQU BG4SC        $210A

; BG2 CHR base address (B), BG1 CHR base address (A).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; BBBB AAAA
; |||| ||||
; |||| ++++- BG1 CHR word base address (word address = AAAA << 12)
; ++++------ BG2 CHR word base address (word address = BBBB << 12)
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BG12NBA)
.EQU BG12NBA      $210B

; BG4 CHR base address (D), BG3 CHR base address (C).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DDDD CCCC
; |||| ||||
; |||| ++++- BG3 CHR word base address (word address = CCCC << 12)
; ++++------ BG4 CHR word base address (word address = DDDD << 12)
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BG34NBA)
.EQU BG34NBA      $210C

; Set BG1 horizontal scroll with two consecutive byte writes. This address also updates M7HOFS. Scroll registers share internal latches: finish both writes before updating another scroll register.
;
; Access: Write-only, two 8-bit writes to the same address (low byte first)
;
; ```text
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  .... ..XX   XXXX XXXX
;         ||   |||| ||||
;         ++---++++-++++- BGn horizontal scroll
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BGnHOFS)
.EQU BG1HOFS      $210D

; Set BG1 vertical scroll with two consecutive byte writes. This address also updates M7VOFS. Scroll registers share internal latches: finish both writes before updating another scroll register.
;
; Access: Write-only, two 8-bit writes to the same address (low byte first)
;
; ```text
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  .... ..YY   YYYY YYYY
;         ||   |||| ||||
;         ++---++++-++++- BGn vertical scroll
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#BGnVOFS)
.EQU BG1VOFS      $210E

; VRAM address increment mode (M), remapping (R), increment size (I). Bit 7 selects increment after the low-byte port (0) or high-byte port (1); it does not change the CPU transfer width.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; M... RRII
; |    ||||
; |    ||++- Address increment amount:
; |    ||     0: Increment by 1 word
; |    ||     1: Increment by 32 words
; |    ||     2: Increment by 128 words
; |    ||     3: Increment by 128 words
; |    ++--- Address remapping: (VMADD -> Internal)
; |           0: None
; |           1: Remap rrrrrrrr YYYccccc -> rrrrrrrr cccccYYY (2bpp)
; |           2: Remap rrrrrrrY YYcccccP -> rrrrrrrc ccccPYYY (4bpp)
; |           3: Remap rrrrrrYY YcccccPP -> rrrrrrcc cccPPYYY (8bpp)
; +--------- Address increment mode:
;             0: Increment after writing $2118 or reading $2139
;             1: Increment after writing $2119 or reading $213A
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#VMAIN)
.EQU VMAIN        $2115

; VRAM word address.
;
; Access: Write-only, byte of a register pair
;
; ```text
; VMADDH      VMADDL
;   $2117       $2116
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; hHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- VRAM word address
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#VMADD)
.EQU VMADDL       $2116

; VRAM word address.
;
; Access: Write-only, byte of a register pair
;
; ```text
; VMADDH      VMADDL
;   $2117       $2116
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; hHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- VRAM word address
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#VMADD)
.EQU VMADDH       $2117

; VRAM data write. Increments VMADD after write according to VMAIN setting.
;
; Access: Write-only, byte of a register pair
;
; ```text
; VMDATAH     VMDATAL
;   $2119       $2118
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- VRAM data word
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#VMDATA)
.EQU VMDATAL      $2118

; VRAM data write. Increments VMADD after write according to VMAIN setting.
;
; Access: Write-only, byte of a register pair
;
; ```text
; VMDATAH     VMDATAL
;   $2119       $2118
; 7  bit  0   7  bit  0
; ---- ----   ---- ----
; HHHH HHHH   LLLL LLLL
; |||| ||||   |||| ||||
; ++++-++++---++++-++++- VRAM data word
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#VMDATA)
.EQU VMDATAH      $2119

; Select the CGRAM palette entry (0-255). Setting CGADD resets the low/high-byte latch. Entry 0 supplies the backdrop color.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; AAAA AAAA
; |||| ||||
; ++++-++++- CGRAM word address
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#CGADD)
.EQU CGADD        $2121

; Write a 15-bit RGB color to the selected palette entry: low byte first, then high byte, both to $2122. After the high byte, the palette index advances. Each color component is 0-31. Access CGRAM during vblank, hblank, or forced blank.
;
; Access: Write-only, two 8-bit writes to the same address (low byte first)
;
; ```text
; 15  bit  8   7  bit  0
;  ---- ----   ---- ----
;  .BBB BBGG   GGGR RRRR
;   ||| ||||   |||| ||||
;   ||| ||||   |||+-++++- Red component
;   ||| ||++---+++------- Green component
;   +++-++--------------- Blue component
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#CGDATA)
.EQU CGDATA       $2122

; Enable main-screen layers: bits 0-3 enable BG1-BG4 and bit 4 enables OBJ sprites.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; ...O 4321
;    | ||||
;    | |||+- Enable BG1 on main screen
;    | ||+-- Enable BG2 on main screen
;    | |+--- Enable BG3 on main screen
;    | +---- Enable BG4 on main screen
;    +------ Enable OBJ on main screen
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#TM)
.EQU TM           $212C

; Fixed color channel select (BGR) and value (C).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; BGRC CCCC
; |||| ||||
; |||+-++++- Color value
; ||+------- Write color value to red channel
; |+-------- Write color value to green channel
; +--------- Write color value to blue channel
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#COLDATA)
.EQU COLDATA      $2132

; VRAM data read. Increments VMADD after read according to VMAIN setting.
;
; Access: Read-only, byte of a register pair
;
; ```text
; VMDATAHREAD VMDATALREAD
;    $213A       $2139
;  7  bit  0   7  bit  0
;  ---- ----   ---- ----
;  HHHH HHHH   LLLL LLLL
;  |||| ||||   |||| ||||
;  ++++-++++---++++-++++- VRAM data word from vram_latch
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#VMDATAREAD)
.EQU VMDATALREAD  $2139

; VRAM data read. Increments VMADD after read according to VMAIN setting.
;
; Access: Read-only, byte of a register pair
;
; ```text
; VMDATAHREAD VMDATALREAD
;    $213A       $2139
;  7  bit  0   7  bit  0
;  ---- ----   ---- ----
;  HHHH HHHH   LLLL LLLL
;  |||| ||||   |||| ||||
;  ++++-++++---++++-++++- VRAM data word from vram_latch
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#VMDATAREAD)
.EQU VMDATAHREAD  $213A

; Interlace field (F), counter latch value (L), NTSC/PAL (M), PPU2 version (V).
;
; Access: Read-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; FLxM VVVV
; |||| ||||
; |||| ++++- PPU2 version
; |||+------ 0: 262 or 525i lines = 60Hz, 1: 312 or 625i lines = 50Hz (PPU2 pin 30)
; ||+------- PPU2 open bus
; |+-------- Counter latch value
; +--------- Interlace field
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/PPU_registers#STAT78)
.EQU STAT78       $213F

; Data to/from APU.
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DDDD DDDD
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#APUIOn)
.EQU APUIO0       $2140

; Data to/from APU.
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DDDD DDDD
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#APUIOn)
.EQU APUIO1       $2141

; Data to/from APU.
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DDDD DDDD
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#APUIOn)
.EQU APUIO2       $2142

; Data to/from APU.
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DDDD DDDD
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#APUIOn)
.EQU APUIO3       $2143

; Vblank NMI enable (N), timer IRQ mode (VH), joypad auto-read enable (J).
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; N.VH ...J
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#NMITIMEN)
.EQU NMITIMEN     $4200

; Unsigned multiplication factor A.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DDDD DDDD
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#WRMPYA)
.EQU WRMPYA       $4202

; Unsigned multiplication factor B, starts 8-cycle multiplication.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DDDD DDDD
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#WRMPYB)
.EQU WRMPYB       $4203

; H counter target for timer IRQ.
;
; Access: Write-only, byte of a register pair
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#HTIME)
.EQU HTIMEL       $4207

; DMA enable.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; 7654 3210
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#MDMAEN)
.EQU MDMAEN       $420B

; HDMA enable.
;
; Access: Write-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; 7654 3210
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#HDMAEN)
.EQU HDMAEN       $420C

; Vblank NMI flag (N), CPU version (V).
;
; Access: Read-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; N... VVVV
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#RDNMI)
.EQU RDNMI        $4210

; Vblank flag (V), hblank flag (H), joypad auto-read in-progress flag (J).
;
; Access: Read-only, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; VH.. ...J
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#HVBJOY)
.EQU HVBJOY       $4212

; Unsigned product or unsigned remainder.
;
; Access: Read-only, byte of a register pair
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#RDMPY)
.EQU RDMPYL       $4216

; Unsigned product or unsigned remainder.
;
; Access: Read-only, byte of a register pair
;
; ```text
; 7  bit  0
; ---- ----
; HHHH HHHH
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#RDMPY)
.EQU RDMPYH       $4217

; 16-bit joypad auto-read result (first read high to last read low).
;
; Access: Read-only, byte of a register pair
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#JOY1)
.EQU JOY1L        $4218

; 16-bit joypad auto-read result (first read high to last read low).
;
; Access: Read-only, byte of a register pair
;
; ```text
; 7  bit  0
; ---- ----
; HHHH HHHH
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#JOY1)
.EQU JOY1H        $4219

; 16-bit joypad auto-read result (first read high to last read low).
;
; Access: Read-only, byte of a register pair
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#JOY2)
.EQU JOY2L        $421A

; 16-bit joypad auto-read result (first read high to last read low).
;
; Access: Read-only, byte of a register pair
;
; ```text
; 7  bit  0
; ---- ----
; HHHH HHHH
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/MMIO_registers#JOY2)
.EQU JOY2H        $421B

; Direction (D), indirect HDMA (I), address increment mode (A), transfer pattern (P).
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DI.A APPP
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DMAPn)
.EQU DMAP0        $4300

; B-bus address.
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; AAAA AAAA
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#BBADn)
.EQU BBAD0        $4301

; DMA source address / HDMA table start address.
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#A1TnL)
.EQU A1T0L        $4302

; DMA source address / HDMA table start address.
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; HHHH HHHH
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#A1TnH)
.EQU A1T0H        $4303

; DMA source address / HDMA table start address.
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; BBBB BBBB
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#A1Bn)
.EQU A1B0         $4304

; DMA byte count (H:L) / HDMA indirect table address (B:H:L).
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DASnL)
.EQU DAS0L        $4305

; DMA byte count (H:L) / HDMA indirect table address (B:H:L).
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; HHHH HHHH
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DASnH)
.EQU DAS0H        $4306

; DMA byte count (H:L) / HDMA indirect table address (B:H:L).
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; BBBB BBBB
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DASBn)
.EQU DASB0        $4307

; Direction (D), indirect HDMA (I), address increment mode (A), transfer pattern (P).
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; DI.A APPP
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DMAPn)
.EQU DMAP1        $4310

; B-bus address.
;
; Access: Read/write, 8-bit
;
; ```text
; 7  bit  0
; ---- ----
; AAAA AAAA
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#BBADn)
.EQU BBAD1        $4311

; DMA source address / HDMA table start address.
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#A1TnL)
.EQU A1T1L        $4312

; DMA source address / HDMA table start address.
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; HHHH HHHH
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#A1TnH)
.EQU A1T1H        $4313

; DMA source address / HDMA table start address.
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; BBBB BBBB
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#A1Bn)
.EQU A1B1         $4314

; DMA byte count (H:L) / HDMA indirect table address (B:H:L).
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; LLLL LLLL
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DASnL)
.EQU DAS1L        $4315

; DMA byte count (H:L) / HDMA indirect table address (B:H:L).
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; HHHH HHHH
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DASnH)
.EQU DAS1H        $4316

; DMA byte count (H:L) / HDMA indirect table address (B:H:L).
;
; Access: Read/write, byte of a three-register group
;
; ```text
; 7  bit  0
; ---- ----
; BBBB BBBB
; ```
;
; [SNESdev reference](https://snes.nesdev.org/wiki/DMA_registers#DASBn)
.EQU DASB1        $4317

; Project-defined bit masks for INIDISP (these are values, not registers).
.EQU ENABLE_SCREEN	$00
.EQU BLANK_SCREEN 	$80
.EQU BRIGHTNESS_0	$00
.EQU BRIGHTNESS_1	$01
.EQU BRIGHTNESS_2	$02
.EQU BRIGHTNESS_3	$03
.EQU BRIGHTNESS_4	$04
.EQU BRIGHTNESS_5	$05
.EQU BRIGHTNESS_6	$06
.EQU BRIGHTNESS_7	$07
.EQU BRIGHTNESS_8	$08
.EQU BRIGHTNESS_9	$09
.EQU BRIGHTNESS_10	$0A
.EQU BRIGHTNESS_11	$0B
.EQU BRIGHTNESS_12	$0C
.EQU BRIGHTNESS_13	$0D
.EQU BRIGHTNESS_14	$0E
.EQU BRIGHTNESS_15	$0F

; VMAIN bit 7: increment after low-byte port (0) or high-byte port (1).
; Legacy mask names below do not select CPU access width.
.EQU VRAM_BYTE_ACCESS	$00
.EQU VRAM_WORD_ACCESS	$80

; TM layer masks: BG0_ENABLE refers to hardware BG1, etc.
.EQU BG0_ENABLE 	$01
.EQU BG1_ENABLE 	$02
.EQU BG2_ENABLE 	$04
.EQU BG3_ENABLE 	$08
.EQU OBJ_ENABLE 	$10

; NMITIMEN automatic joypad reading
.EQU JOY_MANUAL_READ	$00
.EQU JOY_AUTO_READ	$01

; HVBJOY status bit 0: 0 = idle, 1 = auto-read in progress (read-only).
.EQU JOY_DISABLE	$00
.EQU JOY_ENABLE		$01

; For STAT78
.EQU PPU_VERSION	$0F
.EQU PPU_MODE		$10
.EQU PPU_FIELD		$40
