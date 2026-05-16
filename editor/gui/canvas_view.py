"""
canvas_view.py — The zoomable, scrollable grid canvas widget.

Responsibilities:
  - Render the Cell grid as coloured rectangles with optional symbols.
  - Handle left-click (paint), right-click (erase), and hover events.
  - Manage zoom level and communicate cell clicks back to the app
    via callback functions injected at construction.
"""

import tkinter as tk
from typing import Callable, List, Optional, Tuple

from models import Cell, BG, ENT
from constants import (
    GRID_W, GRID_H,
    CELL_SIZE_MIN, CELL_SIZE_DEFAULT, CELL_SIZE_MAX, ZOOM_STEP,
    CELL_COLORS,
)


def cell_visual(cell: Cell) -> Tuple[str, str, str]:
    """Return (fill_colour, symbol, label) for a cell."""
    key = (cell.get_bg(), cell.get_ent())
    return CELL_COLORS.get(key, ("#333333", "?", "Unknown"))


class CanvasView(tk.Frame):
    """
    Scrollable canvas that displays and edits the level grid.

    Callbacks injected by the parent app:
      on_paint(x, y)   — user paints a cell
      on_erase(x, y)   — user erases a cell
      on_hover(x, y)   — user moves the mouse over a cell (x/y may be None)
      on_begin_stroke() — called once at the start of a paint/erase drag
    """

    def __init__(
        self,
        parent: tk.Widget,
        on_paint:        Callable[[int, int], None],
        on_erase:        Callable[[int, int], None],
        on_hover:        Callable[[Optional[int], Optional[int]], None],
        on_begin_stroke: Callable[[], None],
        **kwargs,
    ):
        super().__init__(parent, bg="#111130", bd=2, relief=tk.SUNKEN, **kwargs)

        self.cell_size: int = CELL_SIZE_DEFAULT
        self._on_paint        = on_paint
        self._on_erase        = on_erase
        self._on_hover        = on_hover
        self._on_begin_stroke = on_begin_stroke
        self._painting = False
        self._erasing  = False

        self._build_widgets()
        self._bind_events()

    # ── Widget construction ───────────────────────────────────────────────────

    def _build_widgets(self):
        self.rowconfigure(0, weight=1)
        self.columnconfigure(0, weight=1)

        self.canvas = tk.Canvas(
            self, bg="#0A0A1A", highlightthickness=0, cursor="crosshair"
        )
        self.canvas.grid(row=0, column=0, sticky="nsew")

        vbar = tk.Scrollbar(
            self, orient=tk.VERTICAL, command=self.canvas.yview,
            bg="#111130", troughcolor="#080818",
        )
        vbar.grid(row=0, column=1, sticky="ns")

        hbar = tk.Scrollbar(
            self, orient=tk.HORIZONTAL, command=self.canvas.xview,
            bg="#111130", troughcolor="#080818",
        )
        hbar.grid(row=1, column=0, sticky="ew")

        self.canvas.configure(
            xscrollcommand=hbar.set, yscrollcommand=vbar.set
        )

    def _bind_events(self):
        c = self.canvas
        c.bind("<Button-1>",        self._on_press_left)
        c.bind("<B1-Motion>",       self._on_drag_left)
        c.bind("<ButtonRelease-1>", self._on_release)
        c.bind("<Button-3>",        self._on_press_right)
        c.bind("<B3-Motion>",       self._on_drag_right)
        c.bind("<ButtonRelease-3>", self._on_release)
        c.bind("<Motion>",          self._on_motion)

        # Zoom: Ctrl + scroll wheel (Windows/Mac) or Ctrl + Button-4/5 (Linux)
        c.bind("<Control-MouseWheel>", self._on_ctrl_wheel_win)
        c.bind("<Control-Button-4>",   lambda e: self.zoom_in())
        c.bind("<Control-Button-5>",   lambda e: self.zoom_out())

        # Plain scroll (Windows/Mac) and Linux Button-4/5
        c.bind("<MouseWheel>", self._on_scroll_win)
        c.bind("<Button-4>",   lambda e: self.canvas.yview_scroll(-1, "units"))
        c.bind("<Button-5>",   lambda e: self.canvas.yview_scroll( 1, "units"))

    # ── Rendering ─────────────────────────────────────────────────────────────

    def full_redraw(self, grid: List[List[Cell]]) -> None:
        """Erase and repaint the entire canvas from the given grid."""
        self.canvas.delete("all")
        cs = self.cell_size
        total_w = GRID_W * cs
        total_h = GRID_H * cs
        self.canvas.configure(scrollregion=(0, 0, total_w, total_h))

        for x in range(GRID_W):
            for y in range(GRID_H):
                self._draw_cell(grid[x][y], x, y)

        # Lightweight alignment guides every 4 blocks
        for gx in range(0, GRID_W + 1, 4):
            self.canvas.create_line(gx * cs, 0, gx * cs, total_h,
                                    fill="#1A1A35", width=1)
        for gy in range(0, GRID_H + 1, 4):
            self.canvas.create_line(0, gy * cs, total_w, gy * cs,
                                    fill="#1A1A35", width=1)

    def redraw_cell(self, cell: Cell, x: int, y: int) -> None:
        """Repaint a single cell without touching the rest of the canvas."""
        self._draw_cell(cell, x, y)

    def _draw_cell(self, cell: Cell, x: int, y: int) -> None:
        cs = self.cell_size
        fill, sym, _ = cell_visual(cell)
        x0, y0 = x * cs, y * cs
        x1, y1 = x0 + cs, y0 + cs
        self.canvas.create_rectangle(x0, y0, x1, y1,
                                     fill=fill, outline="#0D0D20", width=1)
        if sym and cs >= 10:
            self.canvas.create_text(
                x0 + cs // 2, y0 + cs // 2,
                text=sym, fill="#FFFFFF",
                font=("Courier", max(7, cs - 8), "bold"),
            )

    # ── Zoom ─────────────────────────────────────────────────────────────────

    def zoom_in(self) -> bool:
        """Increase cell size one step. Returns True if size changed."""
        if self.cell_size < CELL_SIZE_MAX:
            self.cell_size = min(self.cell_size + ZOOM_STEP, CELL_SIZE_MAX)
            return True
        return False

    def zoom_out(self) -> bool:
        """Decrease cell size one step. Returns True if size changed."""
        if self.cell_size > CELL_SIZE_MIN:
            self.cell_size = max(self.cell_size - ZOOM_STEP, CELL_SIZE_MIN)
            return True
        return False

    def zoom_reset(self) -> bool:
        if self.cell_size != CELL_SIZE_DEFAULT:
            self.cell_size = CELL_SIZE_DEFAULT
            return True
        return False

    # ── Mouse helpers ─────────────────────────────────────────────────────────

    def _canvas_to_cell(self, event) -> Tuple[Optional[int], Optional[int]]:
        """Convert a mouse event to grid (x, y), or (None, None) if out of bounds."""
        cx = self.canvas.canvasx(event.x)
        cy = self.canvas.canvasy(event.y)
        gx = int(cx) // self.cell_size
        gy = int(cy) // self.cell_size
        if 0 <= gx < GRID_W and 0 <= gy < GRID_H:
            return gx, gy
        return None, None

    # ── Mouse event handlers ──────────────────────────────────────────────────

    def _on_press_left(self, event):
        self._on_begin_stroke()
        self._painting = True
        x, y = self._canvas_to_cell(event)
        if x is not None:
            self._on_paint(x, y)

    def _on_drag_left(self, event):
        if self._painting:
            x, y = self._canvas_to_cell(event)
            if x is not None:
                self._on_paint(x, y)

    def _on_press_right(self, event):
        self._on_begin_stroke()
        self._erasing = True
        x, y = self._canvas_to_cell(event)
        if x is not None:
            self._on_erase(x, y)

    def _on_drag_right(self, event):
        if self._erasing:
            x, y = self._canvas_to_cell(event)
            if x is not None:
                self._on_erase(x, y)

    def _on_release(self, _event):
        self._painting = False
        self._erasing  = False

    def _on_motion(self, event):
        x, y = self._canvas_to_cell(event)
        self._on_hover(x, y)

    def _on_ctrl_wheel_win(self, event):
        if event.delta > 0:
            self.zoom_in()
        else:
            self.zoom_out()

    def _on_scroll_win(self, event):
        delta = -1 if event.delta > 0 else 1
        self.canvas.yview_scroll(delta, "units")
