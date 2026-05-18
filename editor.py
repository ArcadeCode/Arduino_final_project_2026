#!/usr/bin/env python3
"""
Pac-Man Level Editor for Arduino
Éditeur de niveaux Pac-Man compatible avec la structure Cell[][] Arduino

TODO:
= Phase 1 - Basic cleanup =
- English comments.
- Shard editor.py into different editor/ files.
- Add versioning to `.json` levels.

= Phase 2 - Declaring rules =
- Add rules to force having max 4 Ghosts, 1 Pacman, 1 Fruit.
- Add rule to have maximum 255 Pac-gum + Super pac-gum by level
- Add the pac-gums counter to the level file

= Phase 3 - Improving UX =
- Add a "preview screen" where some ghosts and pacman cells is replaced by their Sprites
    - For remain, each entity is 4 cells large
    - Sprites will be store into "./editor/assets/"
- When ctrl+c or other force stop execution signal is send, save current work under ./levels/ and give the user the possibility to reload his work if an error occurred.
- Add Scroll click to move the grid without using scroll bars.
- Add version used to exporter C++ ready strings.
- Add version upgrade system.

= Phase 4 - Assembling multiples levels =
- Add a `.levels.json` who regroup each level of the game under one file.
- Add the rule to accept max 10 levels into a level set.
- Add a custom select menu to choose what level to edit and the order of levels.
- Add a possibility to import or export individual levels to/from the level set.
- Give the possibility to export the game file to `levels.hpp` with the level loader function ready.

= Phase 5 - Complex editing =
- Add parameter to set the facing of Pacman and the Ghosts.
- Add parameter to set the SCATTER mode target.
- Add parameter to set timing between AI modes.
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import json
import re
from dataclasses import dataclass, field
from typing import List, Optional
from enum import IntEnum

# ─── Constantes ──────────────────────────────────────────────────────────────

GRID_W = 28   # GAME_GRID_X_AXIS_LEN  (colonnes, axe horizontal)
GRID_H = 36   # GAME_GRID_Y_AXIS_LEN  (lignes,   axe vertical)

CELL_SIZE_MIN     =  6   # px minimum par cellule
CELL_SIZE_DEFAULT = 18
CELL_SIZE_MAX     = 48   # px maximum par cellule
ZOOM_STEP         =  2   # incrément de zoom

# ─── Enums identiques au C++ ─────────────────────────────────────────────────

class BG(IntEnum):
    EMPTY    = 0  # BG_EMPTY
    WALL     = 1  # BG_WALL
    GUM      = 2  # BG_GUM
    ENERGIZE = 3  # BG_ENERGIZE

class ENT(IntEnum):
    EMPTY        = 0  # ENT_EMPTY
    PACMAN       = 1  # ENT_PACMAN
    BLUE_GHOST   = 2  # ENT_BLUE_GHOST
    RED_GHOST    = 3  # ENT_RED_GHOST
    PINK_GHOST   = 4  # ENT_PINK_GHOST
    ORANGE_GHOST = 5  # ENT_ORANGE_GHOST
    FRUIT        = 6  # ENT_FRUIT

# ─── Modèle Cell ─────────────────────────────────────────────────────────────

class Cell:
    """
    Représentation d'une cellule identique au C++.
    bit:  7 6 5 4 3 2 1 0
          - - - E E E B B
    """
    def __init__(self, data: int = 0):
        self.data = data & 0xFF

    def get_bg(self) -> BG:
        return BG(self.data & 0b00000011)

    def set_bg(self, bg: BG):
        self.data = (self.data & 0b11111100) | (int(bg) & 0b00000011)

    def get_ent(self) -> ENT:
        return ENT((self.data >> 2) & 0b00000111)

    def set_ent(self, ent: ENT):
        self.data = (self.data & 0b11100011) | ((int(ent) & 0b00000111) << 2)

    def to_char(self) -> str:
        ent = self.get_ent()
        if ent == ENT.PACMAN:       return 'P'
        if ent == ENT.RED_GHOST:    return 'R'
        if ent == ENT.BLUE_GHOST:   return 'B'
        if ent == ENT.PINK_GHOST:   return 'K'
        if ent == ENT.ORANGE_GHOST: return 'O'
        if ent == ENT.FRUIT:        return 'F'
        bg = self.get_bg()
        if bg == BG.WALL:     return '#'
        if bg == BG.GUM:      return '.'
        if bg == BG.ENERGIZE: return '*'
        return ' '

    def copy(self):
        return Cell(self.data)

# ─── Grille ──────────────────────────────────────────────────────────────────

def empty_grid() -> List[List[Cell]]:
    """Crée une grille vide GRID_W × GRID_H, indexée [x][y]
    x = colonne (0..GRID_W-1), y = ligne (0..GRID_H-1)"""
    return [[Cell() for _ in range(GRID_H)] for _ in range(GRID_W)]

# ─── Conversion C++ ──────────────────────────────────────────────────────────

_BYTES_PER_ROW = GRID_W // 4  # 7 bytes per row: 4 cells × 2 bits = 1 byte


def _count_dots(grid: List[List[Cell]]) -> int:
    """Return the total number of collectible cells (BG_GUM + BG_ENERGIZE)."""
    count = 0
    for x in range(GRID_W):
        for y in range(GRID_H):
            if grid[x][y].get_bg() in (BG.GUM, BG.ENERGIZE):
                count += 1
    return count


def _pack_bg_row(grid: List[List[Cell]], y: int) -> List[str]:
    """
    Pack one grid row into _BYTES_PER_ROW hex byte strings.

    4 consecutive cells are packed into one uint8_t (LSB = leftmost cell):
        byte = bg[x+0] | bg[x+1]<<2 | bg[x+2]<<4 | bg[x+3]<<6
    """
    result = []
    for bx in range(_BYTES_PER_ROW):
        byte_val = 0
        for bit in range(4):
            x = bx * 4 + bit
            byte_val |= (int(grid[x][y].get_bg()) & 0x03) << (bit * 2)
        result.append(f"0x{byte_val:02X}")
    return result


def grid_to_cpp(grid: List[List[Cell]], var_name: str = "grid") -> str:
    """
    Convert the editor grid to a levels.hpp C++ snippet (format v2).

    Background layout — format v2
    ──────────────────────────────
    Each cell background needs 2 bits (BG_EMPTY=0 … BG_ENERGIZE=3).
    4 consecutive cells in the same row are packed into one uint8_t (LSB first):

        byte = (bg[x+0] & 0x03)
             | (bg[x+1] & 0x03) << 2
             | (bg[x+2] & 0x03) << 4
             | (bg[x+3] & 0x03) << 6

    One row  = 28 cells = 7 bytes.
    Full grid = 36 rows × 7 bytes = 252 bytes stored in Flash (PROGMEM).
    (vs 1008 bytes of SRAM used by the previous setBackground() approach)

    The LEVEL_BG_READ() macro decodes a single cell on the fly:
        CellBackgroundType bg = LEVEL_BG_READ(LEVEL_0_BG, x, y);

    Convention C++ (types.hpp): grid[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN]
    → first index = y (row), second index = x (column).
    Python storage is grid[x][y]; we transpose here.
    GridPosition is {x, y} in C++.
    """

    # ── collect entity positions ──────────────────────────────────────────────
    pacman_pos = None
    ghost_positions = {
        ENT.BLUE_GHOST:   None,
        ENT.RED_GHOST:    None,
        ENT.PINK_GHOST:   None,
        ENT.ORANGE_GHOST: None,
    }

    for y in range(GRID_H):
        for x in range(GRID_W):
            ent = grid[x][y].get_ent()
            if ent == ENT.PACMAN:
                pacman_pos = (x, y)
            elif ent in ghost_positions:
                ghost_positions[ent] = (x, y)

    dots = _count_dots(grid)

    # ── build output ──────────────────────────────────────────────────────────
    lines: List[str] = []

    lines += [
        "/*",
        " * levels.hpp — Pac-Man level data stored in Flash (PROGMEM).",
        " *",
        " * Background layout — format v2",
        " * ──────────────────────────────",
        " * Each cell background needs 2 bits:",
        " *   BG_EMPTY=0, BG_WALL=1, BG_GUM=2, BG_ENERGIZE=3",
        " *",
        " * 4 consecutive cells in the same row are packed into one uint8_t (LSB first):",
        " *",
        " *   byte = (bg[x+0] & 0x03)",
        " *        | (bg[x+1] & 0x03) << 2",
        " *        | (bg[x+2] & 0x03) << 4",
        " *        | (bg[x+3] & 0x03) << 6",
        " *",
        f" * One row  = {GRID_W} cells = {_BYTES_PER_ROW} bytes.",
        f" * Full grid = {GRID_H} rows x {_BYTES_PER_ROW} bytes = {GRID_H * _BYTES_PER_ROW} bytes in Flash.",
        " * (vs 1008 bytes of SRAM used by the previous Cell grid[][] approach)",
        " *",
        " * Decoding a single cell — use the LEVEL_BG_READ() macro below.",
        " *",
        " * Generated by Pac-Man Level Editor v2.",
        " */",
        "",
        "#pragma once",
        '#include "types.hpp"',
        "",
        "// Total collectible dots (BG_GUM + BG_ENERGIZE) for level 0.",
        f"#define LEVEL_0_TOTAL_DOTS {dots}u",
        "",
        "// Level 0 background — packed 4 cells/byte, indexed [y][x >> 2], LSB = leftmost cell.",
        f"static const uint8_t LEVEL_0_BG[{GRID_H}][{_BYTES_PER_ROW}] PROGMEM = {{",
    ]

    for y in range(GRID_H):
        row_bytes = _pack_bg_row(grid, y)
        comma = "," if y < GRID_H - 1 else " "
        lines.append(f"    {{{', '.join(row_bytes)}}}{comma} // y={y:2d}")

    lines += [
        "};",
        "",
        "/**",
        " * @brief Read a cell background from a PROGMEM level array.",
        " *",
        " * @param level_bg  The LEVEL_x_BG array (e.g. LEVEL_0_BG).",
        " * @param x         Column (0..GAME_GRID_X_AXIS_LEN-1).",
        " * @param y         Row    (0..GAME_GRID_Y_AXIS_LEN-1).",
        " * @return CellBackgroundType",
        " */",
        "#define LEVEL_BG_READ(level_bg, x, y) \\",
        "    ((CellBackgroundType)(                         \\",
        "        (pgm_read_byte(&(level_bg)[(y)][(x) >> 2]) \\",
        "         >> (((x) & 0x03) << 1)) & 0x03))",
        "",
        "/**",
        " * @brief Initialise a GameState for the given level.",
        " *",
        " * The background is left in Flash (PROGMEM) — this function only sets",
        " * entity positions and resets counters.  state.grid[][] is no longer used.",
        " *",
        " * Read the background at runtime with LEVEL_BG_READ(), for example in",
        " * Screen::print_frame() and isWalkable().",
        " */",
        "void loadLevel(GameState& state, uint8_t level) {",
        "    state.tick             = 0;",
        "    state.level            = level;",
        "    state.modePhase        = 0;",
        "    state.lastModeChangeMs = 0;",
        "",
        "    switch (level) {",
        "        default: // unknown level — fall through to 0",
        "        case 0: {",
    ]

    if pacman_pos:
        lines.append(
            f"            state.pacmanPosition      = {{{pacman_pos[0]}, {pacman_pos[1]}}};"
        )

    ghost_setters = {
        ENT.BLUE_GHOST:   "state.blueGhostPosition  ",
        ENT.RED_GHOST:    "state.redGhostPosition   ",
        ENT.PINK_GHOST:   "state.pinkGhostPosition  ",
        ENT.ORANGE_GHOST: "state.orangeGhostPosition",
    }
    for ent, setter in ghost_setters.items():
        pos = ghost_positions[ent]
        if pos:
            lines.append(f"            {setter} = {{{pos[0]}, {pos[1]}}};")

    lines += [
        "            state.totalDots           = LEVEL_0_TOTAL_DOTS;",
        "            state.remainingDots       = LEVEL_0_TOTAL_DOTS;",
        "            break;",
        "        }",
        "    }",
        "}",
    ]

    return "\n".join(lines) + "\n"


def grid_to_raw_array(grid: List[List[Cell]]) -> str:
    """Convertit la grille en tableau C++ brut.

    Convention C++ (types.hpp) : Cell grid[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN]
    → le tableau exporté est donc [GRID_H][GRID_W], première dimension = y (ligne).

    En mémoire Python la grille est grid[x][y], on transpose ici à l'export.
    """
    lines = [
        "// Niveau généré par Pac-Man Level Editor",
        f"// Taille : {GRID_H}×{GRID_W} — Indexé [y][x] comme grid[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN]",
        "",
        f"const uint8_t level_data[{GRID_H}][{GRID_W}] = {{",
    ]
    for y in range(GRID_H):
        row_vals = ", ".join(f"0x{grid[x][y].data:02X}" for x in range(GRID_W))
        comma = "," if y < GRID_H - 1 else ""
        lines.append(f"    {{{row_vals}}}{comma}  // y={y}")
    lines.append("};")
    lines.append("")
    lines.append("// Pour charger :")
    lines.append("// for(int y=0;y<GAME_GRID_Y_AXIS_LEN;y++)")
    lines.append("//   for(int x=0;x<GAME_GRID_X_AXIS_LEN;x++)")
    lines.append("//     state.grid[y][x].data = level_data[y][x];")
    return "\n".join(lines)


def grid_to_ascii(grid: List[List[Cell]]) -> str:
    """Représentation ASCII de la grille.
    On itère sur les lignes (y) en externe pour que chaque ligne de texte
    corresponde à une rangée horizontale de l'écran."""
    rows = []
    for y in range(GRID_H):                      # y = ligne (vertical)
        row = "".join(grid[x][y].to_char() for x in range(GRID_W))  # x = colonne (horizontal)
        rows.append(row)
    return "\n".join(rows)


def ascii_to_grid(text: str) -> Optional[List[List[Cell]]]:
    """Parse une grille ASCII → Cell[x][y]
    Chaque ligne de texte = une rangée y, chaque caractère = une colonne x."""
    char_map = {
        '#': (BG.WALL,     ENT.EMPTY),
        '.': (BG.GUM,      ENT.EMPTY),
        '*': (BG.ENERGIZE, ENT.EMPTY),
        ' ': (BG.EMPTY,    ENT.EMPTY),
        'P': (BG.EMPTY,    ENT.PACMAN),
        'R': (BG.EMPTY,    ENT.RED_GHOST),
        'B': (BG.EMPTY,    ENT.BLUE_GHOST),
        'K': (BG.EMPTY,    ENT.PINK_GHOST),
        'O': (BG.EMPTY,    ENT.ORANGE_GHOST),
        'F': (BG.EMPTY,    ENT.FRUIT),
    }
    lines = text.split('\n')
    while len(lines) < GRID_H:
        lines.append('')
    grid = empty_grid()
    for y, line in enumerate(lines[:GRID_H]):         # y = index de ligne
        for x, ch in enumerate(line[:GRID_W]):        # x = index de colonne
            if ch in char_map:
                bg, ent = char_map[ch]
                grid[x][y].set_bg(bg)
                grid[x][y].set_ent(ent)
    return grid


def raw_array_to_grid(text: str) -> Optional[List[List[Cell]]]:
    """Parse un tableau C++ brut exporté par grid_to_raw_array().

    Le tableau source est [GRID_H][GRID_W] (dimension externe = y),
    on le recharge en mémoire Python comme grid[x][y].
    """
    # 1. Supprimer les commentaires C++ (// ...) — évite de parser les indices y=N
    stripped = re.sub(r'//[^\n]*', '', text)
    # 2. Supprimer les lignes de déclaration C (const uint8_t ..., for(...), etc.)
    #    qui contiennent des nombres parasites comme les dimensions [36][28].
    stripped = re.sub(r'(const\s+\w+.*?=\s*\{|for\s*\(.*?\)|#\w+[^\n]*)', '', stripped)
    hex_vals = re.findall(r'0[xX][0-9a-fA-F]+', stripped)  # uniquement hex 0x.. pour éviter tout entier parasite
    vals = [int(v, 16) for v in hex_vals]
    if len(vals) < GRID_W * GRID_H:
        return None
    grid = empty_grid()
    idx = 0
    for y in range(GRID_H):      # dimension externe du tableau C++ = y
        for x in range(GRID_W):  # dimension interne = x
            grid[x][y] = Cell(vals[idx])
            idx += 1
    return grid


def bg_cpp_name(bg: BG) -> str:
    return {BG.WALL: "BG_WALL", BG.GUM: "BG_GUM",
            BG.ENERGIZE: "BG_ENERGIZE", BG.EMPTY: "BG_EMPTY"}[bg]

def ent_cpp_name(ent: ENT) -> str:
    return {ENT.PACMAN: "ENT_PACMAN", ENT.BLUE_GHOST: "ENT_BLUE_GHOST",
            ENT.RED_GHOST: "ENT_RED_GHOST", ENT.PINK_GHOST: "ENT_PINK_GHOST",
            ENT.ORANGE_GHOST: "ENT_ORANGE_GHOST", ENT.FRUIT: "ENT_FRUIT",
            ENT.EMPTY: "ENT_EMPTY"}[ent]

# ─── Palette visuelle ─────────────────────────────────────────────────────────

CELL_COLORS = {
    # (bg, ent) → (fill, text, label)
    (BG.EMPTY,    ENT.EMPTY):        ("#0A0A1A", "",  "Vide"),
    (BG.WALL,     ENT.EMPTY):        ("#1A3A8F", "",  "Mur"),
    (BG.GUM,      ENT.EMPTY):        ("#1A1A2E", "·", "Pac-gum"),
    (BG.ENERGIZE, ENT.EMPTY):        ("#1A1A2E", "●", "Super gum"),
    (BG.EMPTY,    ENT.PACMAN):       ("#FFD700", "P", "Pac-Man"),
    (BG.EMPTY,    ENT.RED_GHOST):    ("#FF3333", "R", "Fantôme Rouge"),
    (BG.EMPTY,    ENT.BLUE_GHOST):   ("#33CCFF", "B", "Fantôme Bleu"),
    (BG.EMPTY,    ENT.PINK_GHOST):   ("#FF88CC", "K", "Fantôme Rose"),
    (BG.EMPTY,    ENT.ORANGE_GHOST): ("#FF8800", "O", "Fantôme Orange"),
    (BG.EMPTY,    ENT.FRUIT):        ("#FF4488", "F", "Fruit"),
}

def cell_visual(cell: Cell):
    key = (cell.get_bg(), cell.get_ent())
    return CELL_COLORS.get(key, ("#333", "?", "?"))

# ─── Palette items ────────────────────────────────────────────────────────────

PALETTE_ITEMS = [
    ("Vide",          BG.EMPTY,    ENT.EMPTY),
    ("Mur",           BG.WALL,     ENT.EMPTY),
    ("Pac-gum",       BG.GUM,      ENT.EMPTY),
    ("Super pac-gum", BG.ENERGIZE, ENT.EMPTY),
    ("Pac-Man",       BG.EMPTY,    ENT.PACMAN),
    ("Fantôme Rouge", BG.EMPTY,    ENT.RED_GHOST),
    ("Fantôme Bleu",  BG.EMPTY,    ENT.BLUE_GHOST),
    ("Fantôme Rose",  BG.EMPTY,    ENT.PINK_GHOST),
    ("Fantôme Orange",BG.EMPTY,    ENT.ORANGE_GHOST),
    ("Fruit",         BG.EMPTY,    ENT.FRUIT),
]

# ─── Application principale ───────────────────────────────────────────────────

class PacManEditor(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Pac-Man Level Editor — Arduino")
        self.configure(bg="#0A0A1A")
        self.resizable(True, True)
        self.minsize(700, 500)

        self.cell_size: int = CELL_SIZE_DEFAULT

        self.grid_data: List[List[Cell]] = empty_grid()
        self.selected_bg  = BG.WALL
        self.selected_ent = ENT.EMPTY
        self.painting     = False
        self.erasing      = False  # clic droit = effacer
        self.history: List[List[List[Cell]]] = []  # undo stack

        self._build_ui()
        self._refresh_canvas()

    # ── Construction UI ──────────────────────────────────────────────────────

    def _build_ui(self):
        # Titre
        title_bar = tk.Frame(self, bg="#0A0A1A")
        title_bar.pack(fill=tk.X, padx=10, pady=(8, 0))
        tk.Label(title_bar, text="PAC-MAN", font=("Courier", 20, "bold"),
                 fg="#FFD700", bg="#0A0A1A").pack(side=tk.LEFT)
        tk.Label(title_bar, text="Level Editor", font=("Courier", 12),
                 fg="#4488FF", bg="#0A0A1A").pack(side=tk.LEFT, padx=(8, 0))

        # Barre outils
        toolbar = tk.Frame(self, bg="#111130", pady=4)
        toolbar.pack(fill=tk.X, padx=0, pady=(6, 0))
        btn_style = dict(font=("Courier", 9, "bold"), bg="#1A1A40",
                         fg="#CCCCFF", activebackground="#3333AA",
                         activeforeground="#FFFFFF", relief=tk.FLAT,
                         bd=0, padx=10, pady=4, cursor="hand2")
        for label, cmd in [
            ("🗋 Nouveau",     self._new_level),
            ("📂 Ouvrir JSON", self._load_json),
            ("💾 Sauver JSON", self._save_json),
            ("↩ Annuler",      self._undo),
            ("🧹 Tout effacer", self._clear_all),
        ]:
            tk.Button(toolbar, text=label, command=cmd, **btn_style).pack(
                side=tk.LEFT, padx=3)

        # Séparateur + contrôles zoom
        tk.Label(toolbar, text="│", fg="#333366", bg="#111130",
                 font=("Courier", 14)).pack(side=tk.LEFT, padx=4)
        tk.Button(toolbar, text="🔍−", command=self._zoom_out,
                  **btn_style).pack(side=tk.LEFT, padx=2)
        self.zoom_label = tk.Label(toolbar, text=f"{CELL_SIZE_DEFAULT}px",
                                   font=("Courier", 9, "bold"),
                                   fg="#FFD700", bg="#111130", width=5)
        self.zoom_label.pack(side=tk.LEFT)
        tk.Button(toolbar, text="🔍+", command=self._zoom_in,
                  **btn_style).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="⟳", command=self._zoom_reset,
                  **btn_style).pack(side=tk.LEFT, padx=2)

        # Corps principal
        body = tk.Frame(self, bg="#0A0A1A")
        body.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        # Panneau gauche : palette + stats
        left = tk.Frame(body, bg="#0A0A1A", width=160)
        left.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 8))
        left.pack_propagate(False)
        self._build_palette(left)
        self._build_stats(left)

        # Canvas de la grille (scrollable, expansible)
        canvas_outer = tk.Frame(body, bg="#111130", bd=2, relief=tk.SUNKEN)
        canvas_outer.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        canvas_outer.rowconfigure(0, weight=1)
        canvas_outer.columnconfigure(0, weight=1)

        self.canvas = tk.Canvas(
            canvas_outer,
            bg="#0A0A1A", highlightthickness=0, cursor="crosshair"
        )
        self.canvas.grid(row=0, column=0, sticky="nsew")

        vbar = tk.Scrollbar(canvas_outer, orient=tk.VERTICAL,
                            command=self.canvas.yview,
                            bg="#111130", troughcolor="#080818")
        vbar.grid(row=0, column=1, sticky="ns")
        hbar = tk.Scrollbar(canvas_outer, orient=tk.HORIZONTAL,
                            command=self.canvas.xview,
                            bg="#111130", troughcolor="#080818")
        hbar.grid(row=1, column=0, sticky="ew")
        self.canvas.configure(xscrollcommand=hbar.set, yscrollcommand=vbar.set)

        # Bindings souris
        self.canvas.bind("<Button-1>",        self._on_press)
        self.canvas.bind("<B1-Motion>",       self._on_drag)
        self.canvas.bind("<ButtonRelease-1>", self._on_release)
        self.canvas.bind("<Button-3>",        self._on_rclick)
        self.canvas.bind("<B3-Motion>",       self._on_rdrag)
        self.canvas.bind("<ButtonRelease-3>", self._on_release)
        self.canvas.bind("<Motion>",          self._on_hover)
        # Zoom Ctrl+Molette
        self.canvas.bind("<Control-MouseWheel>", self._on_ctrl_wheel)   # Windows/Mac
        self.canvas.bind("<Control-Button-4>",   lambda e: self._zoom_in())   # Linux ↑
        self.canvas.bind("<Control-Button-5>",   lambda e: self._zoom_out())  # Linux ↓
        # Scroll sans Ctrl
        self.canvas.bind("<MouseWheel>",  self._on_mousewheel_scroll)
        self.canvas.bind("<Button-4>",    lambda e: self.canvas.yview_scroll(-1, "units"))
        self.canvas.bind("<Button-5>",    lambda e: self.canvas.yview_scroll( 1, "units"))

        # Panneau droite : export/import
        right = tk.Frame(body, bg="#0A0A1A", width=320)
        right.pack(side=tk.LEFT, fill=tk.Y, padx=(8, 0))
        right.pack_propagate(False)
        self._build_export_panel(right)

    def _build_palette(self, parent):
        tk.Label(parent, text="PALETTE", font=("Courier", 9, "bold"),
                 fg="#4488FF", bg="#0A0A1A").pack(anchor=tk.W, pady=(0, 4))

        self.palette_buttons = []
        self.palette_var = tk.IntVar(value=1)  # Mur sélectionné par défaut

        for i, (label, bg, ent) in enumerate(PALETTE_ITEMS):
            fill, sym, _ = CELL_COLORS.get((bg, ent), ("#333", "?", "?"))
            frame = tk.Frame(parent, bg="#0A0A1A")
            frame.pack(fill=tk.X, pady=1)

            btn = tk.Radiobutton(
                frame, variable=self.palette_var, value=i,
                text=f" {sym or '·'} {label}",
                font=("Courier", 9), fg="#DDDDFF", bg="#0A0A1A",
                selectcolor=fill, activebackground="#0A0A1A",
                activeforeground="#FFFFFF",
                indicatoron=False, relief=tk.FLAT,
                bd=0, padx=6, pady=3,
                command=lambda idx=i: self._select_palette(idx),
                anchor=tk.W, width=18, cursor="hand2"
            )
            btn.configure(
                highlightbackground=fill,
                highlightcolor=fill,
            )
            btn.pack(fill=tk.X)
            self.palette_buttons.append(btn)

        self._select_palette(1)  # Mur par défaut

    def _build_stats(self, parent):
        tk.Label(parent, text="", bg="#0A0A1A").pack(pady=4)
        tk.Label(parent, text="STATISTIQUES", font=("Courier", 9, "bold"),
                 fg="#4488FF", bg="#0A0A1A").pack(anchor=tk.W)
        self.stat_labels = {}
        for key in ["Murs", "Pac-gums", "Super gums", "Entités"]:
            f = tk.Frame(parent, bg="#0A0A1A")
            f.pack(fill=tk.X, pady=1)
            tk.Label(f, text=key + ":", font=("Courier", 8),
                     fg="#888888", bg="#0A0A1A", width=10, anchor=tk.W).pack(side=tk.LEFT)
            lbl = tk.Label(f, text="0", font=("Courier", 8, "bold"),
                           fg="#FFD700", bg="#0A0A1A")
            lbl.pack(side=tk.LEFT)
            self.stat_labels[key] = lbl

        # Info cellule survolée
        tk.Label(parent, text="", bg="#0A0A1A").pack(pady=2)
        self.hover_label = tk.Label(parent, text="[x=-, y=-]",
                                    font=("Courier", 8), fg="#555588",
                                    bg="#0A0A1A", wraplength=150, justify=tk.LEFT)
        self.hover_label.pack(anchor=tk.W)

    def _build_export_panel(self, parent):
        notebook = ttk.Notebook(parent)
        notebook.pack(fill=tk.BOTH, expand=True)

        style = ttk.Style()
        style.theme_use("default")
        style.configure("TNotebook", background="#0A0A1A", borderwidth=0)
        style.configure("TNotebook.Tab", background="#111130", foreground="#AAAACC",
                        font=("Courier", 8, "bold"), padding=[8, 4])
        style.map("TNotebook.Tab", background=[("selected", "#1A1A50")],
                  foreground=[("selected", "#FFD700")])

        ta_style = dict(bg="#080818", fg="#AAFFAA", font=("Courier", 8),
                        insertbackground="#AAFFAA", relief=tk.FLAT,
                        wrap=tk.NONE, bd=0)

        # Tab ASCII
        tab_ascii = tk.Frame(notebook, bg="#0A0A1A")
        notebook.add(tab_ascii, text="ASCII")
        tk.Button(tab_ascii, text="↑ Exporter vers grille",
                  command=self._export_ascii,
                  **self._btn_style()).pack(fill=tk.X, padx=4, pady=(4, 2))
        tk.Button(tab_ascii, text="↓ Importer depuis texte",
                  command=self._import_ascii,
                  **self._btn_style()).pack(fill=tk.X, padx=4, pady=(0, 4))
        self.ascii_text = scrolledtext.ScrolledText(tab_ascii, **ta_style)
        self.ascii_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # Tab C++ setters
        tab_cpp = tk.Frame(notebook, bg="#0A0A1A")
        notebook.add(tab_cpp, text="C++ PROGMEM")
        tk.Button(tab_cpp, text="↑ Générer levels.hpp (PROGMEM)",
                  command=self._export_cpp,
                  **self._btn_style()).pack(fill=tk.X, padx=4, pady=(4, 2))
        tk.Button(tab_cpp, text="📋 Copier",
                  command=lambda: self._copy_text(self.cpp_text),
                  **self._btn_style()).pack(fill=tk.X, padx=4, pady=(0, 4))
        self.cpp_text = scrolledtext.ScrolledText(tab_cpp, **ta_style)
        self.cpp_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # Tab tableau brut
        tab_raw = tk.Frame(notebook, bg="#0A0A1A")
        notebook.add(tab_raw, text="Tableau brut")
        tk.Button(tab_raw, text="↑ Générer tableau uint8_t",
                  command=self._export_raw,
                  **self._btn_style()).pack(fill=tk.X, padx=4, pady=(4, 2))
        tk.Button(tab_raw, text="↓ Importer tableau brut",
                  command=self._import_raw,
                  **self._btn_style()).pack(fill=tk.X, padx=4, pady=(0, 2))
        tk.Button(tab_raw, text="📋 Copier",
                  command=lambda: self._copy_text(self.raw_text),
                  **self._btn_style()).pack(fill=tk.X, padx=4, pady=(0, 4))
        self.raw_text = scrolledtext.ScrolledText(tab_raw, **ta_style)
        self.raw_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

    def _btn_style(self):
        return dict(font=("Courier", 8, "bold"), bg="#1A1A50", fg="#CCCCFF",
                    activebackground="#3333AA", activeforeground="#FFFFFF",
                    relief=tk.FLAT, bd=0, pady=4, cursor="hand2")

    # ── Palette ──────────────────────────────────────────────────────────────

    def _select_palette(self, idx: int):
        self.palette_var.set(idx)
        label, bg, ent = PALETTE_ITEMS[idx]
        self.selected_bg  = bg
        self.selected_ent = ent
        for i, (btn) in enumerate(self.palette_buttons):
            _, bg_i, ent_i = PALETTE_ITEMS[i]
            fill, _, _ = CELL_COLORS.get((bg_i, ent_i), ("#333", "?", "?"))
            if i == idx:
                btn.configure(fg="#FFD700", bg=fill, relief=tk.SUNKEN)
            else:
                btn.configure(fg="#AAAACC", bg="#0A0A1A", relief=tk.FLAT)

    # ── Dessin canvas ────────────────────────────────────────────────────────

    def _refresh_canvas(self):
        self.canvas.delete("all")
        cs = self.cell_size
        # La zone de défilement couvre GRID_W colonnes × GRID_H lignes
        total_w = GRID_W * cs   # largeur  = nb colonnes × taille cellule
        total_h = GRID_H * cs   # hauteur  = nb lignes   × taille cellule
        self.canvas.configure(scrollregion=(0, 0, total_w, total_h))

        for x in range(GRID_W):           # x = colonne → position horizontale
            for y in range(GRID_H):       # y = ligne   → position verticale
                cell = self.grid_data[x][y]
                fill, sym, _ = cell_visual(cell)
                x0 = x * cs   # pixel gauche  (dépend de la colonne x)
                y0 = y * cs   # pixel haut    (dépend de la ligne   y)
                x1 = x0 + cs
                y1 = y0 + cs
                self.canvas.create_rectangle(x0, y0, x1, y1,
                                             fill=fill, outline="#0D0D20", width=1)
                if sym and cs >= 10:
                    self.canvas.create_text(
                        x0 + cs // 2, y0 + cs // 2,
                        text=sym, fill="#FFFFFF",
                        font=("Courier", max(7, cs - 8), "bold")
                    )

        # Grille de guidage légère (tous les 4 blocs)
        for gx in range(0, GRID_W + 1, 4):
            self.canvas.create_line(gx * cs, 0, gx * cs, total_h,
                                    fill="#1A1A35", width=1)
        for gy in range(0, GRID_H + 1, 4):
            self.canvas.create_line(0, gy * cs, total_w, gy * cs,
                                    fill="#1A1A35", width=1)
        self._update_stats()

    def _redraw_cell(self, x: int, y: int):
        """Redessine une seule cellule à la position (x=colonne, y=ligne)."""
        cs = self.cell_size
        cell = self.grid_data[x][y]
        fill, sym, _ = cell_visual(cell)
        x0 = x * cs   # pixel gauche  (colonne x)
        y0 = y * cs   # pixel haut    (ligne   y)
        x1 = x0 + cs
        y1 = y0 + cs
        self.canvas.create_rectangle(x0, y0, x1, y1,
                                     fill=fill, outline="#0D0D20", width=1)
        if sym and cs >= 10:
            self.canvas.create_text(
                x0 + cs // 2, y0 + cs // 2, text=sym,
                fill="#FFFFFF", font=("Courier", max(7, cs - 8), "bold")
            )

    # ── Événements souris ────────────────────────────────────────────────────

    def _canvas_to_cell(self, event):
        """Convertit les coordonnées pixel du canvas en (x=colonne, y=ligne)."""
        cx = self.canvas.canvasx(event.x)   # pixel horizontal → colonne x
        cy = self.canvas.canvasy(event.y)   # pixel vertical   → ligne   y
        x = int(cx) // self.cell_size
        y = int(cy) // self.cell_size
        if 0 <= x < GRID_W and 0 <= y < GRID_H:
            return x, y
        return None, None

    def _on_mousewheel_scroll(self, event):
        """Scroll vertical sans Ctrl (Windows/Mac)"""
        delta = -1 if event.delta > 0 else 1
        self.canvas.yview_scroll(delta, "units")

    # ── Zoom ─────────────────────────────────────────────────────────────────

    def _zoom_in(self, event=None):
        if self.cell_size < CELL_SIZE_MAX:
            self.cell_size = min(self.cell_size + ZOOM_STEP, CELL_SIZE_MAX)
            self._apply_zoom()

    def _zoom_out(self, event=None):
        if self.cell_size > CELL_SIZE_MIN:
            self.cell_size = max(self.cell_size - ZOOM_STEP, CELL_SIZE_MIN)
            self._apply_zoom()

    def _zoom_reset(self, event=None):
        self.cell_size = CELL_SIZE_DEFAULT
        self._apply_zoom()

    def _on_ctrl_wheel(self, event):
        if event.delta > 0:
            self._zoom_in()
        else:
            self._zoom_out()

    def _apply_zoom(self):
        self.zoom_label.configure(text=f"{self.cell_size}px")
        self._refresh_canvas()

    def _on_press(self, event):
        self._push_history()
        self.painting = True
        x, y = self._canvas_to_cell(event)
        if x is not None:
            self._paint(x, y)

    def _on_drag(self, event):
        if self.painting:
            x, y = self._canvas_to_cell(event)
            if x is not None:
                self._paint(x, y)

    def _on_rclick(self, event):
        self._push_history()
        self.erasing = True
        x, y = self._canvas_to_cell(event)
        if x is not None:
            self._erase(x, y)

    def _on_rdrag(self, event):
        if self.erasing:
            x, y = self._canvas_to_cell(event)
            if x is not None:
                self._erase(x, y)

    def _on_release(self, event):
        self.painting = False
        self.erasing  = False

    def _on_hover(self, event):
        x, y = self._canvas_to_cell(event)
        if x is None:
            self.hover_label.configure(text="[x=-, y=-]")
            return
        cell = self.grid_data[x][y]
        _, _, name = cell_visual(cell)
        self.hover_label.configure(
            text=f"x={x}, y={y}\n{name}\ndata=0x{cell.data:02X}"
        )

    def _paint(self, x: int, y: int):
        cell = self.grid_data[x][y]
        cell.set_bg(self.selected_bg)
        cell.set_ent(self.selected_ent)
        self._redraw_cell(x, y)
        self._update_stats()

    def _erase(self, x: int, y: int):
        self.grid_data[x][y] = Cell()
        self._redraw_cell(x, y)
        self._update_stats()

    # ── Stats ────────────────────────────────────────────────────────────────

    def _update_stats(self):
        walls = gums = energize = entities = 0
        for x in range(GRID_W):
            for y in range(GRID_H):
                c = self.grid_data[x][y]
                bg, ent = c.get_bg(), c.get_ent()
                if bg == BG.WALL:     walls    += 1
                if bg == BG.GUM:      gums     += 1
                if bg == BG.ENERGIZE: energize += 1
                if ent != ENT.EMPTY:  entities += 1
        self.stat_labels["Murs"].configure(text=str(walls))
        self.stat_labels["Pac-gums"].configure(text=str(gums))
        self.stat_labels["Super gums"].configure(text=str(energize))
        self.stat_labels["Entités"].configure(text=str(entities))

    # ── Historique (undo) ────────────────────────────────────────────────────

    def _push_history(self):
        snapshot = [[self.grid_data[x][y].copy() for y in range(GRID_H)]
                    for x in range(GRID_W)]
        self.history.append(snapshot)
        if len(self.history) > 50:
            self.history.pop(0)

    def _undo(self):
        if not self.history:
            return
        self.grid_data = self.history.pop()
        self._refresh_canvas()

    # ── Actions toolbar ──────────────────────────────────────────────────────

    def _new_level(self):
        if messagebox.askyesno("Nouveau niveau", "Créer un nouveau niveau vide ?"):
            self._push_history()
            self.grid_data = empty_grid()
            self._refresh_canvas()

    def _clear_all(self):
        if messagebox.askyesno("Effacer", "Effacer toute la grille ?"):
            self._push_history()
            self.grid_data = empty_grid()
            self._refresh_canvas()

    def _save_json(self):
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON", "*.json"), ("Tous", "*.*")],
            title="Sauvegarder le niveau"
        )
        if not path:
            return
        # Sérialisation format v1 : grid[y][x], première dimension = y (ligne).
        # Cohérent avec types.hpp : Cell grid[GAME_GRID_Y_AXIS_LEN][GAME_GRID_X_AXIS_LEN].
        data = [[self.grid_data[x][y].data for x in range(GRID_W)]
                for y in range(GRID_H)]
        with open(path, "w") as f:
            json.dump({"version": 1, "width": GRID_W, "height": GRID_H, "grid": data}, f)
        messagebox.showinfo("Sauvegardé", f"Niveau sauvegardé :\n{path}")

    def _load_json(self):
        path = filedialog.askopenfilename(
            filetypes=[("JSON", "*.json"), ("Tous", "*.*")],
            title="Ouvrir un niveau"
        )
        if not path:
            return
        try:
            with open(path) as f:
                obj = json.load(f)
            raw = obj["grid"]
            version = obj.get("version", 0)
            self._push_history()
            self.grid_data = empty_grid()
            if version >= 1:
                # Format v1+ : raw[y][x], première dimension = y (ligne)
                for y in range(min(GRID_H, len(raw))):
                    for x in range(min(GRID_W, len(raw[y]))):
                        self.grid_data[x][y] = Cell(raw[y][x])
            else:
                # Format v0 (legacy) : raw[x][y], première dimension = x (colonne)
                # Utiliser migrate_levels.py --from 0 --to 1 pour migrer définitivement.
                for x in range(min(GRID_W, len(raw))):
                    for y in range(min(GRID_H, len(raw[x]))):
                        self.grid_data[x][y] = Cell(raw[x][y])
                messagebox.showwarning(
                    "Format v0 détecté",
                    "Ce fichier est au format v0 (axes inversés).\n"
                    "Utilisez migrate_levels.py --from 0 --to 1 pour le migrer.\n"
                    "Il a été chargé correctement mais sera sauvegardé en v1."
                )
            self._refresh_canvas()
            messagebox.showinfo("Chargé", f"Niveau chargé (v{version}) :\n{path}")
        except Exception as e:
            messagebox.showerror("Erreur", str(e))

    # ── Export / Import texte ────────────────────────────────────────────────

    def _export_ascii(self):
        self.ascii_text.delete("1.0", tk.END)
        self.ascii_text.insert("1.0", grid_to_ascii(self.grid_data))

    def _import_ascii(self):
        text = self.ascii_text.get("1.0", tk.END)
        grid = ascii_to_grid(text)
        if grid:
            self._push_history()
            self.grid_data = grid
            self._refresh_canvas()
        else:
            messagebox.showerror("Erreur", "Impossible de parser la grille ASCII.")

    def _export_cpp(self):
        self.cpp_text.delete("1.0", tk.END)
        self.cpp_text.insert("1.0", grid_to_cpp(self.grid_data))

    def _export_raw(self):
        self.raw_text.delete("1.0", tk.END)
        self.raw_text.insert("1.0", grid_to_raw_array(self.grid_data))

    def _import_raw(self):
        text = self.raw_text.get("1.0", tk.END)
        grid = raw_array_to_grid(text)
        if grid:
            self._push_history()
            self.grid_data = grid
            self._refresh_canvas()
        else:
            messagebox.showerror("Erreur",
                "Impossible de parser le tableau.\n"
                f"Il faut au moins {GRID_W * GRID_H} valeurs.")

    def _copy_text(self, widget):
        self.clipboard_clear()
        self.clipboard_append(widget.get("1.0", tk.END))
        messagebox.showinfo("Copié", "Code copié dans le presse-papier.")


# ─── Point d'entrée ───────────────────────────────────────────────────────────

if __name__ == "__main__":
    app = PacManEditor()
    app.mainloop()