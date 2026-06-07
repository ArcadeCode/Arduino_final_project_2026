/**
 * sfx_normal_move.h — Ambiance de déplacement normal (loop)
 *
 * Source originale : son_normal_pacman.h
 *
 * Conversion
 * ─────────────────────────────────────────────────────────────────────────────
 * L'original utilisait une variable flottante `freq` incrémentée de 0.3 Hz
 * à chaque delay(1ms), soit un balayage 150→300→150 Hz sur ~1 000 ms.
 * Cette résolution sub-Hz est inutile pour un buzzer passif avec filtre RC ;
 * on approxime avec un pas de 10 Hz / 33 ms — même enveloppe sonore,
 * 30× moins d'entrées PROGMEM.
 *
 * Cycle complet : montée (16 notes) + descente (14 notes) = 30 notes × 33 ms
 * = ~990 ms par boucle.
 *
 * SoundId associé : SFX_NORMAL_MOVE  (voir audio.hpp)
 * Loop            : true  ← OBLIGATOIRE dans SFX_LOOP[]
 * Priorité        : 3 (LOW)
 */

#pragma once
#include "sfx_common.h"

static const AudioNote SEQ_NORMAL_MOVE[] PROGMEM = {
    // ── Montée : 150 → 300 Hz, step +10, 33 ms ───────────────────────────
    N(150, 33), N(160, 33), N(170, 33), N(180, 33),
    N(190, 33), N(200, 33), N(210, 33), N(220, 33),
    N(230, 33), N(240, 33), N(250, 33), N(260, 33),
    N(270, 33), N(280, 33), N(290, 33), N(300, 33),

    // ── Descente : 290 → 160 Hz, step -10, 33 ms ─────────────────────────
    // (on évite de répéter 300 et 150 qui sont déjà aux extrémités)
    N(290, 33), N(280, 33), N(270, 33), N(260, 33),
    N(250, 33), N(240, 33), N(230, 33), N(220, 33),
    N(210, 33), N(200, 33), N(190, 33), N(180, 33),
    N(170, 33), N(160, 33),

    SEQ_END  // → AudioEngine reboucle sur N(150,33) grâce à LOOP=true
};
