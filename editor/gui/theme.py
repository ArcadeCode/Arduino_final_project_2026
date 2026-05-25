# editor/gui/theme.py
# Visual theme: cell colors, palette item definitions, and shared UI style helpers.
# All color constants live here so the rest of the GUI never hardcodes hex strings.

from typing import Tuple, Dict
from ..models import BG, ENT

# ── Cell color map ────────────────────────────────────────────────────────────
# Maps (BG, ENT) → (fill_color, symbol, display_label)

CellVisual = Tuple[str, str, str]

CELL_COLORS: Dict[Tuple[BG, ENT], CellVisual] = {
    (BG.EMPTY,    ENT.EMPTY):        ("#0A0A1A", "",  "Empty"),
    (BG.WALL,     ENT.EMPTY):        ("#1A3A8F", "",  "Wall"),
    (BG.GUM,      ENT.EMPTY):        ("#1A1A2E", "·", "Pac-gum"),
    (BG.ENERGIZE, ENT.EMPTY):        ("#1A1A2E", "●", "Super pac-gum"),
    (BG.EMPTY,    ENT.PACMAN):       ("#FFD700", "P", "Pac-Man"),
    (BG.EMPTY,    ENT.RED_GHOST):    ("#FF3333", "R", "Red Ghost"),
    (BG.EMPTY,    ENT.BLUE_GHOST):   ("#33CCFF", "B", "Blue Ghost"),
    (BG.EMPTY,    ENT.PINK_GHOST):   ("#FF88CC", "K", "Pink Ghost"),
    (BG.EMPTY,    ENT.ORANGE_GHOST): ("#FF8800", "O", "Orange Ghost"),
    (BG.EMPTY,    ENT.FRUIT):        ("#FF4488", "F", "Fruit"),
}


def cell_visual(cell) -> CellVisual:
    """Return the (fill, symbol, label) tuple for a given Cell."""
    return CELL_COLORS.get((cell.get_bg(), cell.get_ent()), ("#333333", "?", "Unknown"))


# ── Palette items ─────────────────────────────────────────────────────────────
# Ordered list of (label, BG, ENT) shown in the left palette panel.

PALETTE_ITEMS = [
    ("Empty",          BG.EMPTY,    ENT.EMPTY),
    ("Wall",           BG.WALL,     ENT.EMPTY),
    ("Pac-gum",        BG.GUM,      ENT.EMPTY),
    ("Super pac-gum",  BG.ENERGIZE, ENT.EMPTY),
    ("Pac-Man",        BG.EMPTY,    ENT.PACMAN),
    ("Red Ghost",      BG.EMPTY,    ENT.RED_GHOST),
    ("Blue Ghost",     BG.EMPTY,    ENT.BLUE_GHOST),
    ("Pink Ghost",     BG.EMPTY,    ENT.PINK_GHOST),
    ("Orange Ghost",   BG.EMPTY,    ENT.ORANGE_GHOST),
    ("Fruit",          BG.EMPTY,    ENT.FRUIT),
]

# ── Shared UI colors ──────────────────────────────────────────────────────────

BG_DARK     = "#0A0A1A"   # main window background
BG_PANEL    = "#111130"   # toolbar / panel background
BG_CANVAS   = "#080818"   # canvas / text area background
FG_PRIMARY  = "#FFD700"   # primary accent (yellow)
FG_SECONDARY= "#4488FF"   # secondary accent (blue)
FG_TEXT     = "#CCCCFF"   # general text
FG_DIM      = "#888888"   # dimmed labels
FG_CODE     = "#AAFFAA"   # code text area foreground
BTN_NORMAL  = "#1A1A40"   # button background
BTN_ACTIVE  = "#3333AA"   # button hover/active background
BTN_EXPORT  = "#1A1A50"   # export panel button background


def toolbar_btn_style() -> dict:
    """Return a kwargs dict for toolbar tk.Button widgets."""
    return dict(
        font=("Courier", 9, "bold"),
        bg=BTN_NORMAL, fg=FG_TEXT,
        activebackground=BTN_ACTIVE, activeforeground="#FFFFFF",
        relief="flat", bd=0, padx=10, pady=4, cursor="hand2",
    )


def panel_btn_style() -> dict:
    """Return a kwargs dict for export/import panel tk.Button widgets."""
    return dict(
        font=("Courier", 8, "bold"),
        bg=BTN_EXPORT, fg=FG_TEXT,
        activebackground=BTN_ACTIVE, activeforeground="#FFFFFF",
        relief="flat", bd=0, pady=4, cursor="hand2",
    )


def code_text_style() -> dict:
    """Return a kwargs dict for ScrolledText code widgets."""
    return dict(
        bg=BG_CANVAS, fg=FG_CODE,
        font=("Courier", 8),
        insertbackground=FG_CODE,
        relief="flat", wrap="none", bd=0,
    )
