/**
 * sfx_eat_ghost.h — Sting de capture d'un fantôme
 *
 * Source originale : aucune — séquence originale créée pour le projet.
 *
 * Description
 * ─────────────────────────────────────────────────────────────────────────────
 * Montée rapide sur 4 paliers + bref silence final.
 * Distinctif et court (220 ms) pour ne pas empiéter sur SFX_FRIGHTENED
 * qui reprend aussitôt après.
 *
 * SoundId associé : SFX_EAT_GHOST  (voir audio.hpp)
 * Loop            : false
 * Priorité        : 2 (MEDIUM)
 */

#pragma once
#include "sfx_common.h"

static const AudioNote SEQ_EAT_GHOST[] PROGMEM = {
    N( 400, 40),
    N( 600, 40),
    N( 800, 40),
    N(1000, 80),
    REST(20),
    SEQ_END
};
