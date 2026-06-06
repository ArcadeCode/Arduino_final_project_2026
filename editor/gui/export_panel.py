# editor/gui/export_panel.py
# Right-side tabbed panel: ASCII ↔ grid, C++ PROGMEM export (full set), raw uint8_t export/import.
#
# Phase 3 changes:
#   - C++ PROGMEM tab now exports the entire LevelSet via levelset_to_cpp().
#   - ASCII and Raw array tabs remain scoped to the currently active level.
#   - Receives get_levelset() + get_grid() callbacks instead of just get_grid().

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
from typing import Callable, List

from ..models import Cell
from ..levelset import LevelSet
from ..converters import (
    levelset_to_cpp,
    grid_to_ascii, ascii_to_grid,
    grid_to_raw_array, raw_array_to_grid,
)
from ..constants import GRID_W, GRID_H
from .theme import (
    BG_DARK, BG_PANEL, BG_CANVAS,
    FG_PRIMARY, FG_SECONDARY, FG_CODE,
    panel_btn_style, code_text_style,
)


class ExportPanel:
    """
    Tabbed export / import panel.

    Callbacks injected by the parent App:
        get_levelset()   → LevelSet   (used for full C++ PROGMEM export)
        get_grid()       → current level grid  (used for ASCII / Raw tabs)
        set_grid(grid)   → replaces current level grid (triggers full redraw)
    """

    def __init__(
        self,
        parent: tk.Frame,
        get_levelset: Callable[[], LevelSet],
        get_grid: Callable[[], List[List[Cell]]],
        set_grid: Callable[[List[List[Cell]]], None],
    ) -> None:
        self._get_levelset = get_levelset
        self._get_grid     = get_grid
        self._set_grid     = set_grid

        # ── Notebook styling ──────────────────────────────────────────────────
        notebook = ttk.Notebook(parent)
        notebook.pack(fill=tk.BOTH, expand=True)

        style = ttk.Style()
        style.theme_use("default")
        style.configure("TNotebook", background=BG_DARK, borderwidth=0)
        style.configure("TNotebook.Tab",
                        background=BG_PANEL, foreground="#AAAACC",
                        font=("Courier", 8, "bold"), padding=[8, 4])
        style.map("TNotebook.Tab",
                  background=[("selected", "#1A1A50")],
                  foreground=[("selected", FG_PRIMARY)])

        ta  = code_text_style()
        btn = panel_btn_style()

        # ── Tab: ASCII (current level) ────────────────────────────────────────
        tab_ascii = tk.Frame(notebook, bg=BG_DARK)
        notebook.add(tab_ascii, text="ASCII")

        tk.Label(tab_ascii, text="Current level only",
                 font=("Courier", 7), fg="#555588", bg=BG_DARK).pack(anchor=tk.W, padx=4)

        tk.Button(tab_ascii, text="↑ Export grid → text",
                  command=self._export_ascii, **btn).pack(fill=tk.X, padx=4, pady=(2, 2))
        tk.Button(tab_ascii, text="↓ Import text → grid",
                  command=self._import_ascii, **btn).pack(fill=tk.X, padx=4, pady=(0, 4))

        self._ascii_text = scrolledtext.ScrolledText(tab_ascii, **ta)
        self._ascii_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # ── Tab: C++ PROGMEM (full level set) ────────────────────────────────
        tab_cpp = tk.Frame(notebook, bg=BG_DARK)
        notebook.add(tab_cpp, text="C++ PROGMEM")

        tk.Label(tab_cpp, text="Full level set — all levels",
                 font=("Courier", 7), fg="#558855", bg=BG_DARK).pack(anchor=tk.W, padx=4)

        tk.Button(tab_cpp, text="↑ Generate levels.hpp (full set)",
                  command=self._export_cpp, **btn).pack(fill=tk.X, padx=4, pady=(2, 2))
        tk.Button(tab_cpp, text="📋 Copy to clipboard",
                  command=lambda: self._copy_text(self._cpp_text),
                  **btn).pack(fill=tk.X, padx=4, pady=(0, 4))

        self._cpp_text = scrolledtext.ScrolledText(tab_cpp, **ta)
        self._cpp_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # ── Tab: Raw array (current level) ────────────────────────────────────
        tab_raw = tk.Frame(notebook, bg=BG_DARK)
        notebook.add(tab_raw, text="Raw array")

        tk.Label(tab_raw, text="Current level only",
                 font=("Courier", 7), fg="#555588", bg=BG_DARK).pack(anchor=tk.W, padx=4)

        tk.Button(tab_raw, text="↑ Generate uint8_t array",
                  command=self._export_raw, **btn).pack(fill=tk.X, padx=4, pady=(2, 2))
        tk.Button(tab_raw, text="↓ Import uint8_t array",
                  command=self._import_raw, **btn).pack(fill=tk.X, padx=4, pady=(0, 2))
        tk.Button(tab_raw, text="📋 Copy to clipboard",
                  command=lambda: self._copy_text(self._raw_text),
                  **btn).pack(fill=tk.X, padx=4, pady=(0, 4))

        self._raw_text = scrolledtext.ScrolledText(tab_raw, **ta)
        self._raw_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

    # ── Handlers ─────────────────────────────────────────────────────────────

    def _export_ascii(self) -> None:
        self._ascii_text.delete("1.0", tk.END)
        self._ascii_text.insert("1.0", grid_to_ascii(self._get_grid()))

    def _import_ascii(self) -> None:
        text = self._ascii_text.get("1.0", tk.END)
        grid = ascii_to_grid(text)
        if grid:
            self._set_grid(grid)
        else:
            messagebox.showerror("Import error", "Could not parse the ASCII grid.")

    def _export_cpp(self) -> None:
        """Export the full LevelSet as a levels.hpp snippet."""
        self._cpp_text.delete("1.0", tk.END)
        self._cpp_text.insert("1.0", levelset_to_cpp(self._get_levelset()))

    def _export_raw(self) -> None:
        self._raw_text.delete("1.0", tk.END)
        self._raw_text.insert("1.0", grid_to_raw_array(self._get_grid()))

    def _import_raw(self) -> None:
        text = self._raw_text.get("1.0", tk.END)
        grid = raw_array_to_grid(text)
        if grid:
            self._set_grid(grid)
        else:
            messagebox.showerror(
                "Import error",
                f"Could not parse the array.\n"
                f"At least {GRID_W * GRID_H} hex values are required.",
            )

    def _copy_text(self, widget: scrolledtext.ScrolledText) -> None:
        content = widget.get("1.0", tk.END)
        widget.clipboard_clear()
        widget.clipboard_append(content)
        messagebox.showinfo("Copied", "Code copied to clipboard.")
