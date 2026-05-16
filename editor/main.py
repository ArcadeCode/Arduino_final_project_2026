#!/usr/bin/env python3
"""
main.py — Entry point for the Pac-Man Level Editor.

Run with:
    python main.py

TODO:
= Phase 1 COMPLETE =
- English comments.
- Shard editor.py into different editor/ files.
- Add rules to force having max 4 Ghosts, 1 Pacman, 1 Fruit.

= Phase 2 =
- Add the pac-gums counter to the level file
- Add a "preview screen" where some ghosts and pacman cells is replaced by their Sprites
- When ctrl+c or other force stop execution signal is send, save current work under ./levels/ and give the user the possibility to reload his work if an error occurred.

= Phase 3 =
- Add Scroll click to move the grid without using scroll bars.
- Add versioning to `.json` levels.
- Add version used to exporter C++ ready strings.
- Add version translator system.

= Phase 4 =
- Add a `.levels.json` who regroup each level of the game under one file.
- Give the possibility to export the game file to `levels.hpp` with the level loader function ready.

= Phase 5 =
- Add parameter to set the facing of Pacman and the Ghosts.
- Add parameter to set the SCATTER mode target.
- Add parameter to set TPS max (speed up the game)
"""

from gui.app import PacManEditor


def main() -> None:
    app = PacManEditor()
    app.mainloop()


if __name__ == "__main__":
    main()
