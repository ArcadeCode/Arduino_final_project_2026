# editor/gui/levelset_panel.py
# Phase 3 — Level set manager panel.
#
# Displayed as a dockable panel below the palette/stats column.
# Shows the ordered list of levels; lets the user add, duplicate, remove,
# reorder (up/down), and rename individual levels.
#
# All mutations go through callbacks injected by the parent App so that
# app.py stays the single owner of the LevelSet state.

import tkinter as tk
from tkinter import simpledialog, messagebox
from typing import Callable

from ..constants import MAX_LEVELS
from ..levelset import LevelSet
from .theme import (
    BG_DARK, BG_PANEL, BG_CANVAS,
    FG_PRIMARY, FG_SECONDARY, FG_TEXT, FG_DIM,
    BTN_NORMAL, BTN_ACTIVE,
)


class LevelSetPanel:
    """
    Level-set manager widget rendered inside a parent tk.Frame.

    Injected callbacks (all provided by app.py):
        get_levelset()          → LevelSet
        on_select(index)        — user clicked on a level row; switch the canvas to it
        on_add()                — add a new empty level at the end
        on_duplicate(index)     — duplicate the level at *index*
        on_remove(index)        — remove the level at *index*
        on_move_up(index)       — move level *index* one position up
        on_move_down(index)     — move level *index* one position down
        on_rename(index, name)  — rename level *index*

    Call refresh(selected_index) after any structural change to rebuild the list.
    """

    def __init__(
        self,
        parent: tk.Frame,
        get_levelset: Callable[[], LevelSet],
        on_select:    Callable[[int], None],
        on_add:       Callable[[], None],
        on_duplicate: Callable[[int], None],
        on_remove:    Callable[[int], None],
        on_move_up:   Callable[[int], None],
        on_move_down: Callable[[int], None],
        on_rename:    Callable[[int, str], None],
    ) -> None:
        self._get_levelset = get_levelset
        self._on_select    = on_select
        self._on_add       = on_add
        self._on_duplicate = on_duplicate
        self._on_remove    = on_remove
        self._on_move_up   = on_move_up
        self._on_move_down = on_move_down
        self._on_rename    = on_rename

        self._selected: int = 0
        self._suppress_name_trace: bool = False

        # ── Section header ────────────────────────────────────────────────────
        tk.Label(parent, text="", bg=BG_DARK).pack(pady=2)
        tk.Label(
            parent, text="LEVELS",
            font=("Courier", 9, "bold"),
            fg=FG_SECONDARY, bg=BG_DARK,
        ).pack(anchor=tk.W)

        # ── Scrollable level list ──────────────────────────────────────────────
        list_frame = tk.Frame(parent, bg=BG_CANVAS, bd=1, relief=tk.SUNKEN)
        list_frame.pack(fill=tk.BOTH, expand=True, pady=(2, 4))

        scrollbar = tk.Scrollbar(list_frame, orient=tk.VERTICAL)
        scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

        self._listbox = tk.Listbox(
            list_frame,
            yscrollcommand=scrollbar.set,
            selectmode=tk.SINGLE,
            bg=BG_CANVAS, fg=FG_TEXT,
            selectbackground="#1A1A50", selectforeground=FG_PRIMARY,
            font=("Courier", 8),
            relief=tk.FLAT, bd=0,
            activestyle="none",
            exportselection=False,
            height=8,
        )
        self._listbox.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scrollbar.config(command=self._listbox.yview)
        self._listbox.bind("<<ListboxSelect>>", self._on_listbox_select)
        self._listbox.bind("<Double-Button-1>", self._on_double_click)

        # ── Action buttons ────────────────────────────────────────────────────
        _btn = dict(
            font=("Courier", 8, "bold"),
            bg=BTN_NORMAL, fg=FG_TEXT,
            activebackground=BTN_ACTIVE, activeforeground="#FFFFFF",
            relief="flat", bd=0, pady=2, cursor="hand2",
        )

        # Row 1: add / duplicate
        row1 = tk.Frame(parent, bg=BG_DARK)
        row1.pack(fill=tk.X, pady=(0, 1))
        tk.Button(row1, text="+ Add",       command=self._add,       **_btn).pack(side=tk.LEFT,  fill=tk.X, expand=True, padx=(0, 1))
        tk.Button(row1, text="⧉ Duplicate", command=self._duplicate, **_btn).pack(side=tk.LEFT,  fill=tk.X, expand=True)

        # Row 2: move up / move down
        row2 = tk.Frame(parent, bg=BG_DARK)
        row2.pack(fill=tk.X, pady=(0, 1))
        tk.Button(row2, text="▲ Up",   command=self._move_up,   **_btn).pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 1))
        tk.Button(row2, text="▼ Down", command=self._move_down, **_btn).pack(side=tk.LEFT, fill=tk.X, expand=True)

        # Row 3: rename / remove
        row3 = tk.Frame(parent, bg=BG_DARK)
        row3.pack(fill=tk.X, pady=(0, 2))
        tk.Button(row3, text="✎ Rename", command=self._rename, **_btn).pack(side=tk.LEFT, fill=tk.X, expand=True, padx=(0, 1))
        tk.Button(row3, text="✕ Remove", command=self._remove,
                  fg="#FF6666", activeforeground="#FF3333",
                  **{k: v for k, v in _btn.items() if k not in ("fg", "activeforeground")}
                  ).pack(side=tk.LEFT, fill=tk.X, expand=True)

        # ── Set-name editor ───────────────────────────────────────────────────
        tk.Label(parent, text="Set name:", font=("Courier", 8),
                 fg=FG_DIM, bg=BG_DARK).pack(anchor=tk.W)
        self._set_name_var = tk.StringVar()
        name_entry = tk.Entry(
            parent,
            textvariable=self._set_name_var,
            font=("Courier", 8),
            bg=BG_CANVAS, fg=FG_TEXT,
            insertbackground=FG_TEXT,
            relief=tk.FLAT, bd=1,
        )
        name_entry.pack(fill=tk.X, pady=(0, 2))
        self._set_name_var.trace_add("write", self._on_set_name_change)

    # ── Public interface ──────────────────────────────────────────────────────

    def refresh(self, selected_index: int) -> None:
        """Rebuild the listbox from the current LevelSet and highlight *selected_index*."""
        ls = self._get_levelset()
        self._selected = max(0, min(selected_index, len(ls) - 1))

        self._listbox.delete(0, tk.END)
        for i, (name, _) in enumerate(ls.levels):
            label = f" {i:2d}. {name}"
            self._listbox.insert(tk.END, label)

        self._listbox.selection_clear(0, tk.END)
        self._listbox.selection_set(self._selected)
        self._listbox.see(self._selected)

        # Update set-name entry without triggering the write-back trace.
        self._suppress_name_trace = True
        self._set_name_var.set(ls.name)
        self._suppress_name_trace = False

    # ── Internal helpers ──────────────────────────────────────────────────────

    def _current(self) -> int:
        sel = self._listbox.curselection()
        return sel[0] if sel else self._selected

    def _on_listbox_select(self, _event) -> None:
        sel = self._listbox.curselection()
        if sel:
            idx = sel[0]
            self._selected = idx
            self._on_select(idx)

    def _on_double_click(self, _event) -> None:
        """Double-click opens the rename dialog."""
        self._rename()

    # ── Button handlers ───────────────────────────────────────────────────────

    def _add(self) -> None:
        ls = self._get_levelset()
        if len(ls) >= MAX_LEVELS:
            messagebox.showwarning(
                "Level set full",
                f"Cannot add a level: the maximum of {MAX_LEVELS} levels is already reached.",
            )
            return
        self._on_add()

    def _duplicate(self) -> None:
        ls = self._get_levelset()
        if len(ls) >= MAX_LEVELS:
            messagebox.showwarning(
                "Level set full",
                f"Cannot duplicate: the maximum of {MAX_LEVELS} levels is already reached.",
            )
            return
        self._on_duplicate(self._current())

    def _remove(self) -> None:
        ls = self._get_levelset()
        if len(ls) <= 1:
            messagebox.showwarning(
                "Cannot remove",
                "The level set must contain at least one level.",
            )
            return
        idx   = self._current()
        name  = ls.get_name(idx)
        if messagebox.askyesno(
            "Remove level",
            f"Remove level {idx} — {name!r}?\nThis cannot be undone.",
            icon="warning",
        ):
            self._on_remove(idx)

    def _move_up(self) -> None:
        self._on_move_up(self._current())

    def _move_down(self) -> None:
        self._on_move_down(self._current())

    def _rename(self) -> None:
        idx   = self._current()
        ls    = self._get_levelset()
        name  = ls.get_name(idx)
        new_name = simpledialog.askstring(
            "Rename level",
            f"New name for level {idx}:",
            initialvalue=name,
            parent=self._listbox,
        )
        if new_name and new_name.strip():
            self._on_rename(idx, new_name.strip())

    def _on_set_name_change(self, *_args) -> None:
        if self._suppress_name_trace:
            return
        new_name = self._set_name_var.get().strip()
        if new_name:
            self._get_levelset().name = new_name
