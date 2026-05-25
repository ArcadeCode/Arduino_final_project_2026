# editor/gui/app.py
# Main application window: assembles all panels and owns the grid state.
#
# Responsibilities:
#   - Grid ownership (grid_data, history stack)
#   - Wiring palette → paint/erase → canvas → stats
#   - File I/O (new, open, save) with Phase 2 rule validation before saving
#   - Zoom label synchronisation

import tkinter as tk
from tkinter import filedialog, messagebox
from typing import List

from ..constants import CELL_SIZE_DEFAULT, GRID_W, GRID_H
from ..models import BG, ENT, Cell, empty_grid
from ..rules import validate_grid
from ..serialization import save_json, load_json
from .theme import BG_DARK, BG_PANEL, FG_PRIMARY, FG_SECONDARY, FG_TEXT, toolbar_btn_style
from .palette_panel import PalettePanel
from .stats_panel import StatsPanel
from .canvas_view import CanvasView
from .export_panel import ExportPanel


class PacManEditor(tk.Tk):
    """Top-level application window."""

    def __init__(self) -> None:
        super().__init__()
        self.title("Pac-Man Level Editor — Arduino")
        self.configure(bg=BG_DARK)
        self.resizable(True, True)
        self.minsize(700, 500)

        # ── State ─────────────────────────────────────────────────────────────
        self.grid_data: List[List[Cell]] = empty_grid()
        self.history:   List[List[List[Cell]]] = []   # undo stack (max 50 entries)

        self._selected_bg:  BG  = BG.WALL
        self._selected_ent: ENT = ENT.EMPTY

        # ── Build UI ──────────────────────────────────────────────────────────
        self._build_title_bar()
        self._build_toolbar()
        self._build_body()

        # Initial render
        self._canvas.full_redraw(self.grid_data)
        self._stats.refresh(self.grid_data)

    # ── UI construction ───────────────────────────────────────────────────────

    def _build_title_bar(self) -> None:
        bar = tk.Frame(self, bg=BG_DARK)
        bar.pack(fill=tk.X, padx=10, pady=(8, 0))
        tk.Label(bar, text="PAC-MAN",
                 font=("Courier", 20, "bold"), fg=FG_PRIMARY, bg=BG_DARK).pack(side=tk.LEFT)
        tk.Label(bar, text="Level Editor",
                 font=("Courier", 12), fg=FG_SECONDARY, bg=BG_DARK).pack(side=tk.LEFT, padx=(8, 0))

    def _build_toolbar(self) -> None:
        toolbar = tk.Frame(self, bg=BG_PANEL, pady=4)
        toolbar.pack(fill=tk.X, padx=0, pady=(6, 0))
        btn = toolbar_btn_style()

        for label, cmd in [
            ("🗋 New",         self._new_level),
            ("📂 Open JSON",   self._load_json),
            ("💾 Save JSON",   self._save_json),
            ("↩ Undo",         self._undo),
            ("🧹 Clear all",   self._clear_all),
        ]:
            tk.Button(toolbar, text=label, command=cmd, **btn).pack(side=tk.LEFT, padx=3)

        # Separator
        tk.Label(toolbar, text="│", fg="#333366", bg=BG_PANEL,
                 font=("Courier", 14)).pack(side=tk.LEFT, padx=4)

        # Zoom controls
        tk.Button(toolbar, text="🔍−", command=self._zoom_out, **btn).pack(side=tk.LEFT, padx=2)
        self._zoom_label = tk.Label(
            toolbar, text=f"{CELL_SIZE_DEFAULT}px",
            font=("Courier", 9, "bold"), fg=FG_PRIMARY, bg=BG_PANEL, width=5,
        )
        self._zoom_label.pack(side=tk.LEFT)
        tk.Button(toolbar, text="🔍+",  command=self._zoom_in,    **btn).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="⟳",    command=self._zoom_reset,  **btn).pack(side=tk.LEFT, padx=2)

    def _build_body(self) -> None:
        body = tk.Frame(self, bg=BG_DARK)
        body.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        # Left panel: palette + stats
        left = tk.Frame(body, bg=BG_DARK, width=160)
        left.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 8))
        left.pack_propagate(False)

        self._palette = PalettePanel(left, on_select=self._on_palette_select)
        self._stats   = StatsPanel(left)

        # Centre: scrollable canvas
        canvas_outer = tk.Frame(body, bg=BG_PANEL, bd=2, relief=tk.SUNKEN)
        canvas_outer.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self._canvas = CanvasView(
            parent=canvas_outer,
            on_paint=self._paint,
            on_erase=self._erase,
            on_hover=self._on_hover,
            on_leave=self._stats.clear_hover,
            on_stroke_begin=self._push_history,
        )

        # Right panel: export / import
        right = tk.Frame(body, bg=BG_DARK, width=320)
        right.pack(side=tk.LEFT, fill=tk.Y, padx=(8, 0))
        right.pack_propagate(False)

        self._export = ExportPanel(
            parent=right,
            get_grid=lambda: self.grid_data,
            set_grid=self._replace_grid,
        )

    # ── Grid mutation helpers ─────────────────────────────────────────────────

    def _paint(self, x: int, y: int) -> None:
        cell = self.grid_data[x][y]
        cell.set_bg(self._selected_bg)
        cell.set_ent(self._selected_ent)
        self._canvas.redraw_cell(cell, x, y)
        self._stats.refresh(self.grid_data)

    def _erase(self, x: int, y: int) -> None:
        self.grid_data[x][y] = Cell()
        self._canvas.redraw_cell(self.grid_data[x][y], x, y)
        self._stats.refresh(self.grid_data)

    def _replace_grid(self, grid: List[List[Cell]]) -> None:
        """Replace the current grid (used by import and load operations)."""
        self._push_history()
        self.grid_data = grid
        self._canvas.full_redraw(self.grid_data)
        self._stats.refresh(self.grid_data)

    # ── Palette callback ──────────────────────────────────────────────────────

    def _on_palette_select(self, bg: BG, ent: ENT) -> None:
        self._selected_bg  = bg
        self._selected_ent = ent

    # ── Hover callback ────────────────────────────────────────────────────────

    def _on_hover(self, x: int, y: int) -> None:
        self._stats.set_hover(x, y, self.grid_data[x][y])

    # ── Zoom ──────────────────────────────────────────────────────────────────

    def _zoom_in(self) -> None:
        self._canvas.zoom_in()
        self._sync_zoom()

    def _zoom_out(self) -> None:
        self._canvas.zoom_out()
        self._sync_zoom()

    def _zoom_reset(self) -> None:
        self._canvas.zoom_reset()
        self._sync_zoom()

    def _sync_zoom(self) -> None:
        self._zoom_label.configure(text=f"{self._canvas.cell_size}px")
        self._canvas.full_redraw(self.grid_data)

    # ── Undo history ──────────────────────────────────────────────────────────

    def _push_history(self) -> None:
        snapshot = [[self.grid_data[x][y].copy() for y in range(GRID_H)]
                    for x in range(GRID_W)]
        self.history.append(snapshot)
        if len(self.history) > 50:
            self.history.pop(0)

    def _undo(self) -> None:
        if not self.history:
            return
        self.grid_data = self.history.pop()
        self._canvas.full_redraw(self.grid_data)
        self._stats.refresh(self.grid_data)

    # ── Toolbar actions ───────────────────────────────────────────────────────

    def _new_level(self) -> None:
        if messagebox.askyesno("New level", "Create a new empty level?"):
            self._push_history()
            self.grid_data = empty_grid()
            self._canvas.full_redraw(self.grid_data)
            self._stats.refresh(self.grid_data)

    def _clear_all(self) -> None:
        if messagebox.askyesno("Clear", "Erase the entire grid?"):
            self._push_history()
            self.grid_data = empty_grid()
            self._canvas.full_redraw(self.grid_data)
            self._stats.refresh(self.grid_data)

    def _save_json(self) -> None:
        # Phase 2: validate before saving
        violations = validate_grid(self.grid_data)
        if violations:
            msg = "The level has the following issues:\n\n" + "\n\n".join(
                f"• {v}" for v in violations
            )
            msg += "\n\nSave anyway?"
            if not messagebox.askyesno("Validation warnings", msg, icon="warning"):
                return

        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON", "*.json"), ("All files", "*.*")],
            title="Save level",
        )
        if not path:
            return
        save_json(path, self.grid_data)
        messagebox.showinfo("Saved", f"Level saved:\n{path}")

    def _load_json(self) -> None:
        path = filedialog.askopenfilename(
            filetypes=[("JSON", "*.json"), ("All files", "*.*")],
            title="Open level",
        )
        if not path:
            return
        try:
            grid, version, warnings = load_json(path)
        except Exception as exc:
            messagebox.showerror("Load error", str(exc))
            return

        for warning in warnings:
            messagebox.showwarning("Format warning", warning)

        self._push_history()
        self.grid_data = grid
        self._canvas.full_redraw(self.grid_data)
        self._stats.refresh(self.grid_data)
        messagebox.showinfo("Loaded", f"Level loaded (format v{version}):\n{path}")
