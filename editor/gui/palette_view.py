"""
palette_view.py — Left sidebar: tile palette and grid statistics.

PaletteView is a self-contained tk.Frame that:
  - Displays every paintable tile as a radio button.
  - Exposes the currently selected (BG, ENT) via .selected_bg / .selected_ent.
  - Shows live grid statistics updated by the parent app.
  - Shows the hovered cell's coordinates and type.
"""

import tkinter as tk
from typing import Callable, Optional

from models import BG, ENT, Cell
from constants import CELL_COLORS, PALETTE_ITEMS


class PaletteView(tk.Frame):
    """
    Left sidebar widget.

    After construction the parent reads:
      .selected_bg  — currently active BG value
      .selected_ent — currently active ENT value
    """

    def __init__(self, parent: tk.Widget, **kwargs):
        super().__init__(parent, bg="#0A0A1A", **kwargs)

        self.selected_bg:  BG  = BG.WALL
        self.selected_ent: ENT = ENT.EMPTY

        self._build_palette()
        self._build_stats()

    # ── Palette section ───────────────────────────────────────────────────────

    def _build_palette(self):
        tk.Label(
            self, text="PALETTE", font=("Courier", 9, "bold"),
            fg="#4488FF", bg="#0A0A1A",
        ).pack(anchor=tk.W, pady=(0, 4))

        self._palette_var     = tk.IntVar(value=1)  # Wall selected by default
        self._palette_buttons = []

        for i, (label, bg, ent) in enumerate(PALETTE_ITEMS):
            fill, sym, _ = CELL_COLORS.get((bg, ent), ("#333333", "?", "?"))
            frame = tk.Frame(self, bg="#0A0A1A")
            frame.pack(fill=tk.X, pady=1)

            btn = tk.Radiobutton(
                frame,
                variable=self._palette_var,
                value=i,
                text=f" {sym or '·'} {label}",
                font=("Courier", 9),
                fg="#DDDDFF",
                bg="#0A0A1A",
                selectcolor=fill,
                activebackground="#0A0A1A",
                activeforeground="#FFFFFF",
                indicatoron=False,
                relief=tk.FLAT,
                bd=0,
                padx=6,
                pady=3,
                command=lambda idx=i: self.select(idx),
                anchor=tk.W,
                width=18,
                cursor="hand2",
            )
            btn.pack(fill=tk.X)
            self._palette_buttons.append(btn)

        self.select(1)  # Wall active at startup

    def select(self, idx: int) -> None:
        """Activate palette entry at position idx."""
        self._palette_var.set(idx)
        label, bg, ent = PALETTE_ITEMS[idx]
        self.selected_bg  = bg
        self.selected_ent = ent

        for i, btn in enumerate(self._palette_buttons):
            _, bg_i, ent_i = PALETTE_ITEMS[i]
            fill, _, _ = CELL_COLORS.get((bg_i, ent_i), ("#333333", "?", "?"))
            if i == idx:
                btn.configure(fg="#FFD700", bg=fill, relief=tk.SUNKEN)
            else:
                btn.configure(fg="#AAAACC", bg="#0A0A1A", relief=tk.FLAT)

    # ── Stats section ─────────────────────────────────────────────────────────

    def _build_stats(self):
        tk.Label(self, text="", bg="#0A0A1A").pack(pady=4)
        tk.Label(
            self, text="STATISTICS", font=("Courier", 9, "bold"),
            fg="#4488FF", bg="#0A0A1A",
        ).pack(anchor=tk.W)

        self._stat_labels: dict = {}
        stat_rows = [
            ("walls",         "Walls"),
            ("gums",          "Pac-gums"),
            ("power_pellets", "Power pellets"),
            ("ghosts",        f"Ghosts (max 4)"),
            ("pacman",        "Pac-Man (max 1)"),
            ("fruit",         "Fruit (max 1)"),
            ("ghost_house",   "Ghost House"),
        ]
        for key, display in stat_rows:
            row = tk.Frame(self, bg="#0A0A1A")
            row.pack(fill=tk.X, pady=1)
            tk.Label(
                row, text=display + ":", font=("Courier", 8),
                fg="#888888", bg="#0A0A1A", width=16, anchor=tk.W,
            ).pack(side=tk.LEFT)
            lbl = tk.Label(
                row, text="0", font=("Courier", 8, "bold"),
                fg="#FFD700", bg="#0A0A1A",
            )
            lbl.pack(side=tk.LEFT)
            self._stat_labels[key] = lbl

        # Hover info
        tk.Label(self, text="", bg="#0A0A1A").pack(pady=2)
        self._hover_label = tk.Label(
            self, text="[x=-, y=-]",
            font=("Courier", 8), fg="#555588",
            bg="#0A0A1A", wraplength=150, justify=tk.LEFT,
        )
        self._hover_label.pack(anchor=tk.W)

    def update_stats(self, counts: dict) -> None:
        """Refresh the statistics panel. `counts` is the dict from rules.grid_summary()."""
        for key, lbl in self._stat_labels.items():
            lbl.configure(text=str(counts.get(key, 0)))

    def update_hover(self, x: Optional[int], y: Optional[int], cell: Optional[Cell]) -> None:
        """Update the hovered-cell info label."""
        if x is None or cell is None:
            self._hover_label.configure(text="[x=-, y=-]")
            return
        from gui.canvas_view import cell_visual
        _, _, name = cell_visual(cell)
        self._hover_label.configure(
            text=f"x={x}, y={y}\n{name}\ndata=0x{cell.data:02X}"
        )
