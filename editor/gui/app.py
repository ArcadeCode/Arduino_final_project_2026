# editor/gui/app.py
# Main application window: assembles all panels and owns the LevelSet state.
#
# Responsibilities (Phase 3):
#   - LevelSet ownership (replaces bare grid_data from Phase 2)
#   - Per-level undo history stack (max 50 snapshots per level)
#   - Wiring palette → paint/erase → canvas → stats
#   - LevelSetPanel callbacks: add, duplicate, remove, reorder, rename, select
#   - File I/O:
#       • Open/Save .levels.json  (full level set)
#       • Import level from .json (adds one level to the set)
#       • Export current level to .json (standalone v2)
#   - Phase 2 rule validation before saving
#   - Zoom label synchronisation

import tkinter as tk
from tkinter import filedialog, messagebox, simpledialog
from typing import List, Optional

from ..constants import CELL_SIZE_DEFAULT, GRID_W, GRID_H, MAX_LEVELS
from ..models import BG, ENT, Cell, empty_grid
from ..rules import validate_grid
from ..levelset import LevelSet, save_levelset, load_levelset, export_level_json, import_level_json
from .theme import BG_DARK, BG_PANEL, FG_PRIMARY, FG_SECONDARY, FG_TEXT, FG_DIM, toolbar_btn_style
from .palette_panel import PalettePanel
from .stats_panel import StatsPanel
from .canvas_view import CanvasView
from .export_panel import ExportPanel
from .levelset_panel import LevelSetPanel


class PacManEditor(tk.Tk):
    """Top-level application window."""

    def __init__(self) -> None:
        super().__init__()
        self.title("Pac-Man Level Editor — Arduino")
        self.configure(bg=BG_DARK)
        self.resizable(True, True)
        self.minsize(820, 560)

        # ── State ─────────────────────────────────────────────────────────────
        self._levelset: LevelSet = LevelSet.new()
        self._current_idx: int   = 0

        # Per-level undo stacks: history[level_index] = list of grid snapshots.
        self._history: List[List[List[List[Cell]]]] = [[]]

        self._selected_bg:  BG  = BG.WALL
        self._selected_ent: ENT = ENT.EMPTY

        self._levelset_path: Optional[str] = None   # last saved/opened .levels.json

        # ── Build UI ──────────────────────────────────────────────────────────
        self._build_title_bar()
        self._build_toolbar()
        self._build_body()

        # Initial render
        self._full_refresh()

    # ── Convenience accessors ─────────────────────────────────────────────────

    @property
    def _grid(self) -> List[List[Cell]]:
        """The grid of the currently active level."""
        return self._levelset.get_grid(self._current_idx)

    def _ensure_history_slot(self, idx: int) -> None:
        """Grow the history list if *idx* is beyond its current length."""
        while len(self._history) <= idx:
            self._history.append([])

    # ── UI construction ───────────────────────────────────────────────────────

    def _build_title_bar(self) -> None:
        bar = tk.Frame(self, bg=BG_DARK)
        bar.pack(fill=tk.X, padx=10, pady=(8, 0))
        tk.Label(bar, text="PAC-MAN",
                 font=("Courier", 20, "bold"), fg=FG_PRIMARY, bg=BG_DARK).pack(side=tk.LEFT)
        tk.Label(bar, text="Level Editor",
                 font=("Courier", 12), fg=FG_SECONDARY, bg=BG_DARK).pack(side=tk.LEFT, padx=(8, 0))
        # Level-set name indicator (right-aligned)
        self._title_set_label = tk.Label(
            bar, text="",
            font=("Courier", 9), fg=FG_DIM, bg=BG_DARK,
        )
        self._title_set_label.pack(side=tk.RIGHT)

    def _build_toolbar(self) -> None:
        toolbar = tk.Frame(self, bg=BG_PANEL, pady=4)
        toolbar.pack(fill=tk.X, padx=0, pady=(6, 0))
        btn = toolbar_btn_style()

        # ── Level-set file operations ─────────────────────────────────────────
        for label, cmd in [
            ("🗋 New set",        self._new_levelset),
            ("📂 Open set",       self._load_levelset),
            ("💾 Save set",       self._save_levelset),
        ]:
            tk.Button(toolbar, text=label, command=cmd, **btn).pack(side=tk.LEFT, padx=3)

        tk.Label(toolbar, text="│", fg="#333366", bg=BG_PANEL,
                 font=("Courier", 14)).pack(side=tk.LEFT, padx=4)

        # ── Individual-level operations ───────────────────────────────────────
        for label, cmd in [
            ("📥 Import level",   self._import_level),
            ("📤 Export level",   self._export_level),
        ]:
            tk.Button(toolbar, text=label, command=cmd, **btn).pack(side=tk.LEFT, padx=3)

        tk.Label(toolbar, text="│", fg="#333366", bg=BG_PANEL,
                 font=("Courier", 14)).pack(side=tk.LEFT, padx=4)

        # ── Grid editing operations ───────────────────────────────────────────
        for label, cmd in [
            ("↩ Undo",           self._undo),
            ("🧹 Clear",         self._clear_all),
        ]:
            tk.Button(toolbar, text=label, command=cmd, **btn).pack(side=tk.LEFT, padx=3)

        tk.Label(toolbar, text="│", fg="#333366", bg=BG_PANEL,
                 font=("Courier", 14)).pack(side=tk.LEFT, padx=4)

        # ── Zoom controls ─────────────────────────────────────────────────────
        tk.Button(toolbar, text="🔍−", command=self._zoom_out, **btn).pack(side=tk.LEFT, padx=2)
        self._zoom_label = tk.Label(
            toolbar, text=f"{CELL_SIZE_DEFAULT}px",
            font=("Courier", 9, "bold"), fg=FG_PRIMARY, bg=BG_PANEL, width=5,
        )
        self._zoom_label.pack(side=tk.LEFT)
        tk.Button(toolbar, text="🔍+", command=self._zoom_in,   **btn).pack(side=tk.LEFT, padx=2)
        tk.Button(toolbar, text="⟳",   command=self._zoom_reset, **btn).pack(side=tk.LEFT, padx=2)

    def _build_body(self) -> None:
        body = tk.Frame(self, bg=BG_DARK)
        body.pack(fill=tk.BOTH, expand=True, padx=8, pady=8)

        # ── Left panel: palette + stats + level manager ───────────────────────
        left = tk.Frame(body, bg=BG_DARK, width=170)
        left.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 8))
        left.pack_propagate(False)

        self._palette = PalettePanel(left, on_select=self._on_palette_select)
        self._stats   = StatsPanel(left)

        tk.Frame(left, bg="#222244", height=1).pack(fill=tk.X, pady=4)

        self._level_panel = LevelSetPanel(
            parent=left,
            get_levelset=lambda: self._levelset,
            on_select=self._lvl_select,
            on_add=self._lvl_add,
            on_duplicate=self._lvl_duplicate,
            on_remove=self._lvl_remove,
            on_move_up=self._lvl_move_up,
            on_move_down=self._lvl_move_down,
            on_rename=self._lvl_rename,
        )

        # ── Centre: scrollable canvas ─────────────────────────────────────────
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

        # ── Right panel: export / import ──────────────────────────────────────
        right = tk.Frame(body, bg=BG_DARK, width=320)
        right.pack(side=tk.LEFT, fill=tk.Y, padx=(8, 0))
        right.pack_propagate(False)

        self._export = ExportPanel(
            parent=right,
            get_levelset=lambda: self._levelset,
            get_grid=lambda: self._grid,
            set_grid=self._replace_grid,
        )

    # ── Full-refresh helper ───────────────────────────────────────────────────

    def _full_refresh(self) -> None:
        """Redraw canvas + stats + level panel + title bar."""
        self._canvas.full_redraw(self._grid)
        self._stats.refresh(self._grid)
        self._level_panel.refresh(self._current_idx)
        name = self._levelset.name
        path_hint = f" — {self._levelset_path}" if self._levelset_path else ""
        self._title_set_label.configure(text=f"{name}{path_hint}")

    # ── Grid mutation helpers ─────────────────────────────────────────────────

    def _paint(self, x: int, y: int) -> None:
        cell = self._grid[x][y]
        cell.set_bg(self._selected_bg)
        cell.set_ent(self._selected_ent)
        self._canvas.redraw_cell(cell, x, y)
        self._stats.refresh(self._grid)

    def _erase(self, x: int, y: int) -> None:
        self._grid[x][y] = Cell()
        self._levelset.set_grid(self._current_idx, self._grid)   # ensure ref is consistent
        self._canvas.redraw_cell(self._grid[x][y], x, y)
        self._stats.refresh(self._grid)

    def _replace_grid(self, grid: List[List[Cell]]) -> None:
        """Replace the current level's grid (used by import and ASCII/Raw load)."""
        self._push_history()
        self._levelset.set_grid(self._current_idx, grid)
        self._canvas.full_redraw(self._grid)
        self._stats.refresh(self._grid)

    # ── Palette callback ──────────────────────────────────────────────────────

    def _on_palette_select(self, bg: BG, ent: ENT) -> None:
        self._selected_bg  = bg
        self._selected_ent = ent

    # ── Hover callback ────────────────────────────────────────────────────────

    def _on_hover(self, x: int, y: int) -> None:
        self._stats.set_hover(x, y, self._grid[x][y])

    # ── Zoom ──────────────────────────────────────────────────────────────────

    def _zoom_in(self)    -> None: self._canvas.zoom_in();    self._sync_zoom()
    def _zoom_out(self)   -> None: self._canvas.zoom_out();   self._sync_zoom()
    def _zoom_reset(self) -> None: self._canvas.zoom_reset(); self._sync_zoom()

    def _sync_zoom(self) -> None:
        self._zoom_label.configure(text=f"{self._canvas.cell_size}px")
        self._canvas.full_redraw(self._grid)

    # ── Undo history ──────────────────────────────────────────────────────────

    def _push_history(self) -> None:
        self._ensure_history_slot(self._current_idx)
        snapshot = [[self._grid[x][y].copy() for y in range(GRID_H)]
                    for x in range(GRID_W)]
        stack = self._history[self._current_idx]
        stack.append(snapshot)
        if len(stack) > 50:
            stack.pop(0)

    def _undo(self) -> None:
        self._ensure_history_slot(self._current_idx)
        stack = self._history[self._current_idx]
        if not stack:
            return
        self._levelset.set_grid(self._current_idx, stack.pop())
        self._canvas.full_redraw(self._grid)
        self._stats.refresh(self._grid)

    # ── LevelSetPanel callbacks ───────────────────────────────────────────────

    def _lvl_select(self, idx: int) -> None:
        """Switch the canvas to level *idx*."""
        self._current_idx = idx
        self._canvas.full_redraw(self._grid)
        self._stats.refresh(self._grid)
        self._level_panel.refresh(self._current_idx)

    def _lvl_add(self) -> None:
        try:
            new_idx = self._levelset.add_level()
        except ValueError as e:
            messagebox.showwarning("Cannot add", str(e))
            return
        self._ensure_history_slot(new_idx)
        self._current_idx = new_idx
        self._full_refresh()

    def _lvl_duplicate(self, idx: int) -> None:
        try:
            new_idx = self._levelset.duplicate_level(idx)
        except ValueError as e:
            messagebox.showwarning("Cannot duplicate", str(e))
            return
        self._ensure_history_slot(new_idx)
        self._current_idx = new_idx
        self._full_refresh()

    def _lvl_remove(self, idx: int) -> None:
        try:
            self._levelset.remove_level(idx)
        except ValueError as e:
            messagebox.showwarning("Cannot remove", str(e))
            return
        # Remove the matching history slot.
        if idx < len(self._history):
            self._history.pop(idx)
        self._current_idx = max(0, min(idx, len(self._levelset) - 1))
        self._full_refresh()

    def _lvl_move_up(self, idx: int) -> None:
        new_idx = self._levelset.move_up(idx)
        # Mirror history swap.
        if new_idx != idx:
            self._ensure_history_slot(idx)
            self._ensure_history_slot(new_idx)
            self._history[idx], self._history[new_idx] = (
                self._history[new_idx], self._history[idx]
            )
        self._current_idx = new_idx
        self._full_refresh()

    def _lvl_move_down(self, idx: int) -> None:
        new_idx = self._levelset.move_down(idx)
        if new_idx != idx:
            self._ensure_history_slot(idx)
            self._ensure_history_slot(new_idx)
            self._history[idx], self._history[new_idx] = (
                self._history[new_idx], self._history[idx]
            )
        self._current_idx = new_idx
        self._full_refresh()

    def _lvl_rename(self, idx: int, name: str) -> None:
        self._levelset.set_name(idx, name)
        self._level_panel.refresh(self._current_idx)
        self._full_refresh()

    # ── Toolbar actions ───────────────────────────────────────────────────────

    def _new_levelset(self) -> None:
        if not messagebox.askyesno("New level set",
                                   "Create a new empty level set?\nAll unsaved changes will be lost."):
            return
        name = simpledialog.askstring(
            "Level set name", "Name for the new level set:",
            initialvalue="New Level Set", parent=self,
        ) or "New Level Set"
        self._levelset = LevelSet.new(name)
        self._current_idx = 0
        self._history = [[]]
        self._levelset_path = None
        self._full_refresh()

    def _save_levelset(self) -> None:
        # Validate every level; warn for each with issues.
        all_violations = []
        for i, (lname, grid) in enumerate(self._levelset.levels):
            v = validate_grid(grid)
            if v:
                all_violations.append((i, lname, v))

        if all_violations:
            lines = []
            for i, lname, v in all_violations:
                lines.append(f"Level {i} — {lname}:")
                lines.extend(f"  • {msg}" for msg in v)
            msg = "Some levels have issues:\n\n" + "\n".join(lines) + "\n\nSave anyway?"
            if not messagebox.askyesno("Validation warnings", msg, icon="warning"):
                return

        path = self._levelset_path or filedialog.asksaveasfilename(
            defaultextension=".levels.json",
            filetypes=[("Level set", "*.levels.json"), ("JSON", "*.json"), ("All files", "*.*")],
            title="Save level set",
        )
        if not path:
            return
        save_levelset(path, self._levelset)
        self._levelset_path = path
        self._full_refresh()
        messagebox.showinfo("Saved", f"Level set saved ({len(self._levelset)} levels):\n{path}")

    def _load_levelset(self) -> None:
        path = filedialog.askopenfilename(
            filetypes=[("Level set", "*.levels.json"), ("JSON", "*.json"), ("All files", "*.*")],
            title="Open level set",
        )
        if not path:
            return
        try:
            levelset, warnings = load_levelset(path)
        except Exception as exc:
            messagebox.showerror("Load error", str(exc))
            return

        for w in warnings:
            messagebox.showwarning("Format warning", w)

        self._levelset = levelset
        self._current_idx = 0
        self._history = [[] for _ in range(len(levelset))]
        self._levelset_path = path
        self._full_refresh()
        messagebox.showinfo(
            "Loaded",
            f"Level set loaded ({len(levelset)} levels):\n{path}",
        )

    def _import_level(self) -> None:
        """Import a standalone .json level and append it to the current set."""
        if len(self._levelset) >= MAX_LEVELS:
            messagebox.showwarning(
                "Level set full",
                f"Cannot import: the set already contains {MAX_LEVELS} levels.",
            )
            return
        path = filedialog.askopenfilename(
            filetypes=[("JSON level", "*.json"), ("All files", "*.*")],
            title="Import level from JSON",
        )
        if not path:
            return
        try:
            name, grid, warnings = import_level_json(path)
        except Exception as exc:
            messagebox.showerror("Import error", str(exc))
            return

        for w in warnings:
            messagebox.showwarning("Format warning", w)

        try:
            new_idx = self._levelset.add_level(name=name, grid=grid)
        except ValueError as e:
            messagebox.showwarning("Cannot import", str(e))
            return

        self._ensure_history_slot(new_idx)
        self._current_idx = new_idx
        self._full_refresh()
        messagebox.showinfo("Imported", f"Level {name!r} imported as Level {new_idx}.")

    def _export_level(self) -> None:
        """Export the current level as a standalone .json v2 file."""
        violations = validate_grid(self._grid)
        if violations:
            msg = "The current level has issues:\n\n" + "\n".join(f"• {v}" for v in violations)
            msg += "\n\nExport anyway?"
            if not messagebox.askyesno("Validation warnings", msg, icon="warning"):
                return

        lname = self._levelset.get_name(self._current_idx)
        path = filedialog.asksaveasfilename(
            defaultextension=".json",
            filetypes=[("JSON level", "*.json"), ("All files", "*.*")],
            title=f"Export level {self._current_idx} — {lname}",
            initialfile=f"{lname}.json",
        )
        if not path:
            return
        export_level_json(path, lname, self._grid)
        messagebox.showinfo("Exported", f"Level {lname!r} exported to:\n{path}")

    def _clear_all(self) -> None:
        lname = self._levelset.get_name(self._current_idx)
        if messagebox.askyesno("Clear level", f"Erase the entire grid of {lname!r}?"):
            self._push_history()
            self._levelset.set_grid(self._current_idx, empty_grid())
            self._canvas.full_redraw(self._grid)
            self._stats.refresh(self._grid)
