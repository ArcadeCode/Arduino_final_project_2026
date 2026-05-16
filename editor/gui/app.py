"""
app.py — PacManEditor: the top-level application window.

Wires together:
  - PaletteView   (left sidebar)
  - CanvasView    (central grid canvas)
  - ExportView    (right notebook)
  - Toolbar buttons

All business logic (history, painting, rule enforcement) lives here
rather than in the individual views so each view stays focused on UI.
"""

import tkinter as tk
from tkinter import filedialog, messagebox
from typing import List, Optional

from models import Cell, BG, ENT, empty_grid
from constants import GRID_W, GRID_H, CELL_SIZE_DEFAULT
from rules import can_place, grid_summary
from grid_io import (
    save_json, load_json,
    grid_to_ascii, grid_from_ascii,
    grid_to_cpp,
    grid_to_raw_array, grid_from_raw_array,
)
from gui.canvas_view import CanvasView
from gui.palette_view import PaletteView
from gui.export_view import ExportView


class PacManEditor(tk.Tk):
    """Root application window."""

    _HISTORY_LIMIT = 50   # maximum undo steps kept in memory

    def __init__(self):
        super().__init__()
        self.title("Pac-Man Level Editor — Arduino")
        self.configure(bg="#0A0A1A")
        self.resizable(True, True)
        self.minsize(700, 500)

        self._grid: List[List[Cell]] = empty_grid(GRID_W, GRID_H)
        self._history: List[List[List[Cell]]] = []

        self._build_ui()
        self._canvas_view.full_redraw(self._grid)
        self._refresh_stats()

    # ── UI construction ───────────────────────────────────────────────────────

    def _build_ui(self):
        self._build_title_bar()
        self._build_toolbar()
        self._build_body()

    def _build_title_bar(self):
        bar = tk.Frame(self, bg="#0A0A1A")
        bar.pack(fill=tk.X, padx=10, pady=(8, 0))
        tk.Label(bar, text="PAC-MAN", font=("Courier", 20, "bold"),
                 fg="#FFD700", bg="#0A0A1A").pack(side=tk.LEFT)
        tk.Label(bar, text="Level Editor", font=("Courier", 12),
                 fg="#4488FF", bg="#0A0A1A").pack(side=tk.LEFT, padx=(8, 0))

    def _build_toolbar(self):
        toolbar = tk.Frame(self, bg="#111130", pady=4)
        toolbar.pack(fill=tk.X, pady=(6, 0))

        btn = dict(
            font=("Courier", 9, "bold"),
            bg="#1A1A40", fg="#CCCCFF",
            activebackground="#3333AA", activeforeground="#FFFFFF",
            relief=tk.FLAT, bd=0, padx=10, pady=4, cursor="hand2",
        )
        actions = [
            ("🗋 New",         self._new_level),
            ("📂 Open JSON",   self._load_json),
            ("💾 Save JSON",   self._save_json),
            ("↩ Undo",         self._undo),
            ("🧹 Clear all",   self._clear_all),
        ]
        for label, cmd in actions:
            tk.Button(toolbar, text=label, command=cmd, **btn).pack(
                side=tk.LEFT, padx=3)

        # Separator
        tk.Label(toolbar, text="│", fg="#333366", bg="#111130",
                 font=("Courier", 14)).pack(side=tk.LEFT, padx=4)

        # Zoom controls
        tk.Button(toolbar, text="🔍−", command=self._zoom_out, **btn).pack(
            side=tk.LEFT, padx=2)
        self._zoom_label = tk.Label(
            toolbar, text=f"{CELL_SIZE_DEFAULT}px",
            font=("Courier", 9, "bold"), fg="#FFD700", bg="#111130", width=5,
        )
        self._zoom_label.pack(side=tk.LEFT)
        tk.Button(toolbar, text="🔍+", command=self._zoom_in,  **btn).pack(
            side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="⟳",   command=self._zoom_reset, **btn).pack(
            side=tk.LEFT, padx=2)

    def _build_body(self):
        body = tk.Frame(self, bg="#0A0A1A")
        body.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        # Left: palette + stats
        left = tk.Frame(body, bg="#0A0A1A", width=170)
        left.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 8))
        left.pack_propagate(False)
        self._palette_view = PaletteView(left)
        self._palette_view.pack(fill=tk.BOTH, expand=True)

        # Centre: scrollable canvas
        self._canvas_view = CanvasView(
            body,
            on_paint        = self._handle_paint,
            on_erase        = self._handle_erase,
            on_hover        = self._handle_hover,
            on_begin_stroke = self._push_history,
        )
        self._canvas_view.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        # Wire toolbar zoom buttons to the canvas zoom and react to Ctrl+wheel
        # that fires inside CanvasView.
        # (CanvasView.zoom_in/out already call _apply_zoom via the toolbar
        #  buttons below; the canvas internal Ctrl+wheel also calls them but
        #  doesn't update the label — we override via event on the root.)
        self.bind_all("<Control-MouseWheel>", self._toolbar_ctrl_wheel)
        self.bind_all("<Control-Button-4>",   lambda e: self._zoom_in())
        self.bind_all("<Control-Button-5>",   lambda e: self._zoom_out())

        # Right: export/import notebook
        right = tk.Frame(body, bg="#0A0A1A", width=320)
        right.pack(side=tk.LEFT, fill=tk.Y, padx=(8, 0))
        right.pack_propagate(False)
        self._export_view = ExportView(
            right,
            generate_ascii = lambda: grid_to_ascii(self._grid),
            generate_cpp   = lambda: grid_to_cpp(self._grid),
            generate_raw   = lambda: grid_to_raw_array(self._grid),
            import_ascii   = self._import_ascii,
            import_raw     = self._import_raw,
        )
        self._export_view.pack(fill=tk.BOTH, expand=True)

    # ── Painting / erasing ────────────────────────────────────────────────────

    def _handle_paint(self, x: int, y: int) -> None:
        bg  = self._palette_view.selected_bg
        ent = self._palette_view.selected_ent

        ok, reason = can_place(self._grid, x, y, bg, ent)
        if not ok:
            messagebox.showwarning("Placement rule", reason)
            return

        self._grid[x][y].set_bg(bg)
        self._grid[x][y].set_ent(ent)
        self._canvas_view.redraw_cell(self._grid[x][y], x, y)
        self._refresh_stats()

    def _handle_erase(self, x: int, y: int) -> None:
        self._grid[x][y] = Cell()
        self._canvas_view.redraw_cell(self._grid[x][y], x, y)
        self._refresh_stats()

    def _handle_hover(self, x: Optional[int], y: Optional[int]) -> None:
        cell = self._grid[x][y] if x is not None else None
        self._palette_view.update_hover(x, y, cell)

    # ── Stats ─────────────────────────────────────────────────────────────────

    def _refresh_stats(self) -> None:
        self._palette_view.update_stats(grid_summary(self._grid))

    # ── History (undo) ────────────────────────────────────────────────────────

    def _push_history(self) -> None:
        snapshot = [
            [self._grid[x][y].copy() for y in range(GRID_H)]
            for x in range(GRID_W)
        ]
        self._history.append(snapshot)
        if len(self._history) > self._HISTORY_LIMIT:
            self._history.pop(0)

    def _undo(self) -> None:
        if not self._history:
            return
        self._grid = self._history.pop()
        self._canvas_view.full_redraw(self._grid)
        self._refresh_stats()

    # ── Toolbar actions ───────────────────────────────────────────────────────

    def _new_level(self) -> None:
        if messagebox.askyesno("New level", "Create a new empty level?"):
            self._push_history()
            self._grid = empty_grid(GRID_W, GRID_H)
            self._canvas_view.full_redraw(self._grid)
            self._refresh_stats()

    def _clear_all(self) -> None:
        if messagebox.askyesno("Clear", "Erase the entire grid?"):
            self._push_history()
            self._grid = empty_grid(GRID_W, GRID_H)
            self._canvas_view.full_redraw(self._grid)
            self._refresh_stats()

    def _save_json(self) -> None:
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON", "*.json"), ("All files", "*.*")],
            title="Save level",
        )
        if not path:
            return
        save_json(self._grid, path)
        messagebox.showinfo("Saved", f"Level saved:\n{path}")

    def _load_json(self) -> None:
        path = filedialog.askopenfilename(
            filetypes=[("JSON", "*.json"), ("All files", "*.*")],
            title="Open level",
        )
        if not path:
            return
        try:
            self._push_history()
            self._grid = load_json(path)
            self._canvas_view.full_redraw(self._grid)
            self._refresh_stats()
            messagebox.showinfo("Loaded", f"Level loaded:\n{path}")
        except Exception as exc:
            messagebox.showerror("Error", str(exc))

    # ── Import helpers (called by ExportView) ─────────────────────────────────

    def _import_ascii(self, text: str) -> bool:
        grid = grid_from_ascii(text)
        if grid is None:
            return False
        self._push_history()
        self._grid = grid
        self._canvas_view.full_redraw(self._grid)
        self._refresh_stats()
        return True

    def _import_raw(self, text: str) -> bool:
        grid = grid_from_raw_array(text)
        if grid is None:
            return False
        self._push_history()
        self._grid = grid
        self._canvas_view.full_redraw(self._grid)
        self._refresh_stats()
        return True

    # ── Zoom ─────────────────────────────────────────────────────────────────

    def _zoom_in(self) -> None:
        if self._canvas_view.zoom_in():
            self._apply_zoom()

    def _zoom_out(self) -> None:
        if self._canvas_view.zoom_out():
            self._apply_zoom()

    def _zoom_reset(self) -> None:
        if self._canvas_view.zoom_reset():
            self._apply_zoom()

    def _apply_zoom(self) -> None:
        self._zoom_label.configure(text=f"{self._canvas_view.cell_size}px")
        self._canvas_view.full_redraw(self._grid)

    def _toolbar_ctrl_wheel(self, event) -> None:
        if event.delta > 0:
            self._zoom_in()
        else:
            self._zoom_out()
