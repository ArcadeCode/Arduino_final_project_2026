/**
 * sfx_eat_dot.h — "Wakka" : son de déglutition d'une pac-gomme
 *
 * Source originale : manger_pacgum.h
 *
 * Conversion
 * ─────────────────────────────────────────────────────────────────────────────
 * Boucle originale :
 *   Montée  : for f = 350 → 600, step=+50, delay=6 ms  (6 notes)
 *   Descente: for f = 550 → 250, step=-60, delay=5 ms  (6 notes)
 *
 * Toutes les itérations déroulées explicitement.
 * Durée totale : 6×6 + 6×5 = 66 ms — suffisamment court pour
 * ne pas gêner le rythme de jeu.
 *
 * SoundId associé : SFX_EAT_DOT  (voir audio.hpp)
 * Loop            : false
 * Priorité        : 3 (LOW) — interrompu par tout le reste
 */

#pragma once
#include "sfx_common.h"

static const AudioNote SEQ_EAT_DOT[] PROGMEM = {
    // ── Montée : 350 → 600 Hz, step +50, 6 ms chacune ───────────────────
    N(350, 6),
    N(400, 6),
    N(450, 6),
    N(500, 6),
    N(550, 6),
    N(600, 6),

    // ── Descente : 550 → 250 Hz, step -60, 5 ms chacune ─────────────────
    N(550, 5),
    N(490, 5),
    N(430, 5),
    N(370, 5),
    N(310, 5),
    N(250, 5),

    SEQ_END
};
