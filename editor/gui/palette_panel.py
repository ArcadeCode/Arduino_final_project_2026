# editor/gui/palette_panel.py
# Left-side palette panel: tile selector (Wall, Gum, Ghosts…).

import tkinter as tk
from typing import Callable

from ..models import BG, ENT
from .theme import CELL_COLORS, PALETTE_ITEMS, BG_DARK, FG_SECONDARY


class PalettePanel:
    """
    Palette widget rendered inside a parent tk.Frame.

    Calls on_select(bg, ent) whenever the user picks a different tile type.
    """

    def __init__(self, parent: tk.Frame, on_select: Callable[[BG, ENT], None]) -> None:
        self._on_select = on_select
        self._buttons: list = []
        self._var = tk.IntVar(value=1)  # Wall selected by default (index 1)

        tk.Label(
            parent, text="PALETTE",
            font=("Courier", 9, "bold"),
            fg=FG_SECONDARY, bg=BG_DARK,
        ).pack(anchor=tk.W, pady=(0, 4))

        for i, (label, bg, ent) in enumerate(PALETTE_ITEMS):
            fill, sym, _ = CELL_COLORS.get((bg, ent), ("#333333", "?", "?"))
            frame = tk.Frame(parent, bg=BG_DARK)
            frame.pack(fill=tk.X, pady=1)

            btn = tk.Radiobutton(
                frame,
                variable=self._var, value=i,
                text=f" {sym or '·'} {label}",
                font=("Courier", 9),
                fg="#DDDDFF", bg=BG_DARK,
                selectcolor=fill,
                activebackground=BG_DARK, activeforeground="#FFFFFF",
                indicatoron=False, relief=tk.FLAT,
                bd=0, padx=6, pady=3,
                command=lambda idx=i: self._select(idx),
                anchor=tk.W, width=18, cursor="hand2",
            )
            btn.configure(highlightbackground=fill, highlightcolor=fill)
            btn.pack(fill=tk.X)
            self._buttons.append(btn)

        # Select Wall by default
        self._select(1)

    def _select(self, idx: int) -> None:
        self._var.set(idx)
        _, bg, ent = PALETTE_ITEMS[idx]
        for i, btn in enumerate(self._buttons):
            _, bg_i, ent_i = PALETTE_ITEMS[i]
            fill, _, _ = CELL_COLORS.get((bg_i, ent_i), ("#333333", "?", "?"))
            if i == idx:
                btn.configure(fg=FG_SECONDARY[0].replace("#", "#FF") if False else "#FFD700",
                               bg=fill, relief=tk.SUNKEN)
            else:
                btn.configure(fg="#AAAACC", bg=BG_DARK, relief=tk.FLAT)
        self._on_select(bg, ent)
