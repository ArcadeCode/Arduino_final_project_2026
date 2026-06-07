/**
 * sfx_frightened.h — Son du mode furtif des fantômes (loop)
 *
 * Source originale : son_mode_furtif.h
 *
 * Conversion
 * ─────────────────────────────────────────────────────────────────────────────
 * Boucle originale (50 répétitions) :
 *   Montée  : for f = 400 → 530, step=+20, delay=4 ms  (7 pas, s'arrête avant 550)
 *   Descente: for f = 550 → 410, step=-20, delay=4 ms  (7 pas)
 *
 * Un seul cycle montée+descente est stocké (14 notes, ~112 ms).
 * Le flag LOOP=true dans audio.hpp relance la séquence automatiquement,
 * reproduisant les 50 répétitions sans coût PROGMEM.
 *
 * SoundId associé : SFX_FRIGHTENED  (voir audio.hpp)
 * Loop            : true  ← OBLIGATOIRE dans SFX_LOOP[]
 * Priorité        : 2 (MEDIUM)
 */

#pragma once
#include "sfx_common.h"

static const AudioNote SEQ_FRIGHTENED[] PROGMEM = {
    // ── Montée : 400 → 540 Hz, step +20, 4 ms ────────────────────────────
    N(400, 4),
    N(420, 4),
    N(440, 4),
    N(460, 4),
    N(480, 4),
    N(500, 4),
    N(520, 4),
    N(540, 4),

    // ── Descente : 520 → 420 Hz, step -20, 4 ms ──────────────────────────
    N(520, 4),
    N(500, 4),
    N(480, 4),
    N(460, 4),
    N(440, 4),
    N(420, 4),

    SEQ_END  // → AudioEngine reboucle sur N(400,4) grâce à LOOP=true
};
