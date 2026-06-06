#!/usr/bin/env python3
"""
Pac-Man Level Editor for Arduino — entry point.

Run with:
    python editor.py

DONE:
= Phase 1 - Basic cleanup =
- English comments.
- Shard editor.py into different editor/ files.

= Phase 2 - Declaring rules =
- Add rules to force having max 4 Ghosts, 1 Pacman, 1 Fruit.
- Add rule to have maximum 255 Pac-gum + Super pac-gum by level
- Add the pac-gums counter to the level file

= Phase 3 - Assembling multiple levels =
- Add a .levels.json file grouping all levels of a game under one file.
- Enforce a maximum of 10 levels per level set.
- Add a custom level-select menu (order, add, remove levels).
- Allow importing/exporting individual levels to/from a level set.
- Export the full levels.hpp file (all levels) with the level loader ready.

TODO:
= Phase 4 - Improving UX =
- Add a "preview screen" where ghosts and Pac-Man cells are replaced by their sprites.
    - Each entity occupies 4 cells.
    - Sprites stored in ./editor/assets/
- On Ctrl+C or other forced-stop signal, auto-save current work under ./levels/
  and offer to reload on the next startup.
- Add middle-click drag to pan the grid without using scrollbars.
- Show the editor format version in the toolbar.
- Add a format upgrade system (migrate older .json files on load).

= Phase 5 - Complex editing =
- Add a parameter to set the starting facing of Pac-Man and each ghost.
- Add a parameter to set the SCATTER mode corner target per ghost.
- Add a parameter to configure the Chase/Scatter timing per level.
"""

from editor.gui import PacManEditor

if __name__ == "__main__":
    app = PacManEditor()
    app.mainloop()