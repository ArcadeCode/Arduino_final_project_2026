# editor/gui/canvas_view.py
# Scrollable grid canvas: rendering, zoom, and all mouse interactions.

import tkinter as tk
from typing import Callable, List, Optional, Tuple

from ..constants import GRID_W, GRID_H, CELL_SIZE_DEFAULT, CELL_SIZE_MIN, CELL_SIZE_MAX, ZOOM_STEP
from ..models import BG, ENT, Cell
from .theme import cell_visual, BG_DARK


class CanvasView:
    """
    Scrollable canvas that renders the level grid.

    Callbacks injected by the parent App:
        on_paint(x, y)   — user left-clicked / dragged on a cell
        on_erase(x, y)   — user right-clicked / dragged on a cell
        on_hover(x, y)   — mouse moved over cell (x, y)
        on_leave()       — mouse left the canvas area
        on_stroke_begin() — called once at the start of each paint/erase stroke
    """

    def __init__(
        self,
        parent: tk.Frame,
        on_paint:        Callable[[int, int], None],
        on_erase:        Callable[[int, int], None],
        on_hover:        Callable[[int, int], None],
        on_leave:        Callable[[], None],
        on_stroke_begin: Callable[[], None],
    ) -> None:
        self._on_paint        = on_paint
        self._on_erase        = on_erase
        self._on_hover        = on_hover
        self._on_leave        = on_leave
        self._on_stroke_begin = on_stroke_begin

        self.cell_size: int = CELL_SIZE_DEFAULT
        self._painting = False
        self._erasing  = False

        # ── Canvas + scrollbars ───────────────────────────────────────────────
        parent.rowconfigure(0, weight=1)
        parent.columnconfigure(0, weight=1)

        self.canvas = tk.Canvas(
            parent,
            bg=BG_DARK, highlightthickness=0, cursor="crosshair",
        )
        self.canvas.grid(row=0, column=0, sticky="nsew")

        vbar = tk.Scrollbar(parent, orient=tk.VERTICAL,   command=self.canvas.yview)
        vbar.grid(row=0, column=1, sticky="ns")
        hbar = tk.Scrollbar(parent, orient=tk.HORIZONTAL, command=self.canvas.xview)
        hbar.grid(row=1, column=0, sticky="ew")
        self.canvas.configure(xscrollcommand=hbar.set, yscrollcommand=vbar.set)

        # ── Mouse bindings ────────────────────────────────────────────────────
        self.canvas.bind("<Button-1>",        self._on_press)
        self.canvas.bind("<B1-Motion>",       self._on_drag)
        self.canvas.bind("<ButtonRelease-1>", self._on_release)
        self.canvas.bind("<Button-3>",        self._on_rclick)
        self.canvas.bind("<B3-Motion>",       self._on_rdrag)
        self.canvas.bind("<ButtonRelease-3>", self._on_release)
        self.canvas.bind("<Motion>",          self._on_motion)
        self.canvas.bind("<Leave>",           lambda _e: self._on_leave())

        # Zoom: Ctrl+wheel (Windows/Mac) and Ctrl+Button-4/5 (Linux)
        self.canvas.bind("<Control-MouseWheel>", self._on_ctrl_wheel)
        self.canvas.bind("<Control-Button-4>",   lambda _e: self.zoom_in())
        self.canvas.bind("<Control-Button-5>",   lambda _e: self.zoom_out())

        # Vertical scroll without Ctrl
        self.canvas.bind("<MouseWheel>", self._on_scroll_wheel)
        self.canvas.bind("<Button-4>",   lambda _e: self.canvas.yview_scroll(-1, "units"))
        self.canvas.bind("<Button-5>",   lambda _e: self.canvas.yview_scroll( 1, "units"))

    # ── Public interface ──────────────────────────────────────────────────────

    def full_redraw(self, grid: List[List[Cell]]) -> None:
        """Erase and redraw the entire canvas from the grid data."""
        self.canvas.delete("all")
        cs = self.cell_size
        total_w = GRID_W * cs
        total_h = GRID_H * cs
        self.canvas.configure(scrollregion=(0, 0, total_w, total_h))

        for x in range(GRID_W):
            for y in range(GRID_H):
                self._draw_cell(grid[x][y], x, y)

        # Light guide lines every 4 cells (matches the 4-cells-per-byte packing)
        for gx in range(0, GRID_W + 1, 4):
            self.canvas.create_line(gx * cs, 0, gx * cs, total_h,
                                    fill="#1A1A35", width=1)
        for gy in range(0, GRID_H + 1, 4):
            self.canvas.create_line(0, gy * cs, total_w, gy * cs,
                                    fill="#1A1A35", width=1)

    def redraw_cell(self, cell: Cell, x: int, y: int) -> None:
        """Redraw a single cell without touching the rest of the canvas."""
        self._draw_cell(cell, x, y)

    def zoom_in(self) -> None:
        if self.cell_size < CELL_SIZE_MAX:
            self.cell_size = min(self.cell_size + ZOOM_STEP, CELL_SIZE_MAX)

    def zoom_out(self) -> None:
        if self.cell_size > CELL_SIZE_MIN:
            self.cell_size = max(self.cell_size - ZOOM_STEP, CELL_SIZE_MIN)

    def zoom_reset(self) -> None:
        self.cell_size = CELL_SIZE_DEFAULT

    # ── Internal drawing ──────────────────────────────────────────────────────

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

    def _canvas_to_cell(self, event) -> Tuple[Optional[int], Optional[int]]:
        """Convert a mouse-event pixel position to grid (x, y) coordinates."""
        cx = self.canvas.canvasx(event.x)
        cy = self.canvas.canvasy(event.y)
        x = int(cx) // self.cell_size
        y = int(cy) // self.cell_size
        if 0 <= x < GRID_W and 0 <= y < GRID_H:
            return x, y
        return None, None

    # ── Mouse event handlers ──────────────────────────────────────────────────

    def _on_press(self, event) -> None:
        self._on_stroke_begin()
        self._painting = True
        x, y = self._canvas_to_cell(event)
        if x is not None:
            self._on_paint(x, y)

    def _on_drag(self, event) -> None:
        if self._painting:
            x, y = self._canvas_to_cell(event)
            if x is not None:
                self._on_paint(x, y)

    def _on_rclick(self, event) -> None:
        self._on_stroke_begin()
        self._erasing = True
        x, y = self._canvas_to_cell(event)
        if x is not None:
            self._on_erase(x, y)

    def _on_rdrag(self, event) -> None:
        if self._erasing:
            x, y = self._canvas_to_cell(event)
            if x is not None:
                self._on_erase(x, y)

    def _on_release(self, event) -> None:
        self._painting = False
        self._erasing  = False

    def _on_motion(self, event) -> None:
        x, y = self._canvas_to_cell(event)
        if x is not None:
            self._on_hover(x, y)
        else:
            self._on_leave()

    def _on_ctrl_wheel(self, event) -> None:
        if event.delta > 0:
            self.zoom_in()
        else:
            self.zoom_out()

    def _on_scroll_wheel(self, event) -> None:
        delta = -1 if event.delta > 0 else 1
        self.canvas.yview_scroll(delta, "units")
