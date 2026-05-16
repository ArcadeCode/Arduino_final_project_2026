"""
export_view.py — Right-side export / import notebook panel.

Three tabs:
  ASCII       — human-readable char map (import + export)
  C++ setters — Arduino loadLevel() function body (export + copy)
  Raw array   — flat uint8_t array (import + export + copy)

The panel holds no grid state itself; it receives the current grid
from the parent app via the generate_* callbacks injected at construction.
"""

import tkinter as tk
from tkinter import ttk, scrolledtext, messagebox
from typing import Callable, List

from models import Cell


class ExportView(tk.Frame):
    """
    Notebook with three export/import tabs.

    Callbacks injected at construction (all () → str):
      generate_ascii()    — current grid → ASCII string
      generate_cpp()      — current grid → C++ setter string
      generate_raw()      — current grid → raw uint8_t string
      import_ascii(text)  — parse ASCII text → update grid (returns bool success)
      import_raw(text)    — parse raw text  → update grid (returns bool success)
    """

    def __init__(
        self,
        parent: tk.Widget,
        generate_ascii:  Callable[[], str],
        generate_cpp:    Callable[[], str],
        generate_raw:    Callable[[], str],
        import_ascii:    Callable[[str], bool],
        import_raw:      Callable[[str], bool],
        **kwargs,
    ):
        super().__init__(parent, bg="#0A0A1A", **kwargs)

        self._gen_ascii   = generate_ascii
        self._gen_cpp     = generate_cpp
        self._gen_raw     = generate_raw
        self._imp_ascii   = import_ascii
        self._imp_raw     = import_raw

        self._build_notebook()

    # ── Notebook ──────────────────────────────────────────────────────────────

    def _build_notebook(self):
        notebook = ttk.Notebook(self)
        notebook.pack(fill=tk.BOTH, expand=True)

        style = ttk.Style()
        style.theme_use("default")
        style.configure("TNotebook",     background="#0A0A1A", borderwidth=0)
        style.configure("TNotebook.Tab", background="#111130", foreground="#AAAACC",
                        font=("Courier", 8, "bold"), padding=[8, 4])
        style.map("TNotebook.Tab",
                  background=[("selected", "#1A1A50")],
                  foreground=[("selected", "#FFD700")])

        ta_cfg = dict(
            bg="#080818", fg="#AAFFAA",
            font=("Courier", 8),
            insertbackground="#AAFFAA",
            relief=tk.FLAT, wrap=tk.NONE, bd=0,
        )
        btn = self._btn_style

        # ── Tab: ASCII ───────────────────────────────────────────────────────
        tab_ascii = tk.Frame(notebook, bg="#0A0A1A")
        notebook.add(tab_ascii, text="ASCII")
        tk.Button(tab_ascii, text="↑ Export to canvas",
                  command=self._do_export_ascii, **btn()).pack(
                      fill=tk.X, padx=4, pady=(4, 2))
        tk.Button(tab_ascii, text="↓ Import from text",
                  command=self._do_import_ascii, **btn()).pack(
                      fill=tk.X, padx=4, pady=(0, 4))
        self._ascii_text = scrolledtext.ScrolledText(tab_ascii, **ta_cfg)
        self._ascii_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # ── Tab: C++ setters ─────────────────────────────────────────────────
        tab_cpp = tk.Frame(notebook, bg="#0A0A1A")
        notebook.add(tab_cpp, text="C++ setters")
        tk.Button(tab_cpp, text="↑ Generate C++ code",
                  command=self._do_export_cpp, **btn()).pack(
                      fill=tk.X, padx=4, pady=(4, 2))
        tk.Button(tab_cpp, text="📋 Copy",
                  command=lambda: self._copy(self._cpp_text), **btn()).pack(
                      fill=tk.X, padx=4, pady=(0, 4))
        self._cpp_text = scrolledtext.ScrolledText(tab_cpp, **ta_cfg)
        self._cpp_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

        # ── Tab: Raw array ───────────────────────────────────────────────────
        tab_raw = tk.Frame(notebook, bg="#0A0A1A")
        notebook.add(tab_raw, text="Raw array")
        tk.Button(tab_raw, text="↑ Generate uint8_t array",
                  command=self._do_export_raw, **btn()).pack(
                      fill=tk.X, padx=4, pady=(4, 2))
        tk.Button(tab_raw, text="↓ Import raw array",
                  command=self._do_import_raw, **btn()).pack(
                      fill=tk.X, padx=4, pady=(0, 2))
        tk.Button(tab_raw, text="📋 Copy",
                  command=lambda: self._copy(self._raw_text), **btn()).pack(
                      fill=tk.X, padx=4, pady=(0, 4))
        self._raw_text = scrolledtext.ScrolledText(tab_raw, **ta_cfg)
        self._raw_text.pack(fill=tk.BOTH, expand=True, padx=4, pady=4)

    # ── Button actions ────────────────────────────────────────────────────────

    def _do_export_ascii(self):
        self._set_text(self._ascii_text, self._gen_ascii())

    def _do_import_ascii(self):
        text = self._ascii_text.get("1.0", tk.END)
        if not self._imp_ascii(text):
            messagebox.showerror("Import error", "Could not parse the ASCII grid.")

    def _do_export_cpp(self):
        self._set_text(self._cpp_text, self._gen_cpp())

    def _do_export_raw(self):
        self._set_text(self._raw_text, self._gen_raw())

    def _do_import_raw(self):
        text = self._raw_text.get("1.0", tk.END)
        if not self._imp_raw(text):
            messagebox.showerror(
                "Import error",
                "Could not parse the raw array.\n"
                "Make sure it contains enough hex/decimal values.",
            )

    # ── Helpers ───────────────────────────────────────────────────────────────

    @staticmethod
    def _set_text(widget: scrolledtext.ScrolledText, content: str) -> None:
        widget.delete("1.0", tk.END)
        widget.insert("1.0", content)

    def _copy(self, widget: scrolledtext.ScrolledText) -> None:
        self.clipboard_clear()
        self.clipboard_append(widget.get("1.0", tk.END))
        messagebox.showinfo("Copied", "Code copied to clipboard.")

    @staticmethod
    def _btn_style() -> dict:
        return dict(
            font=("Courier", 8, "bold"),
            bg="#1A1A50", fg="#CCCCFF",
            activebackground="#3333AA", activeforeground="#FFFFFF",
            relief=tk.FLAT, bd=0, pady=4, cursor="hand2",
        )
