/**
 * sfx_ghost_retreat.h — Mélodie de retour des yeux (ghost retreat, loop)
 *
 * Source originale : retour_fantome.h
 *
 * Conversion
 * ─────────────────────────────────────────────────────────────────────────────
 * L'original joue 4 paliers de trille (2 notes alternées × 3 répétitions)
 * sans jamais appeler noTone() — la boucle loop() repart directement du
 * palier 1, créant une coupure brutale qui reproduit l'arcade.
 *
 * Structure d'un cycle complet (24 notes) :
 *   Palier 1 : Ré5 / Mi5   (587 / 659 Hz) × 3   →  6 notes × 25 ms
 *   Palier 2 : 700 / Sol5  (700 / 784 Hz) × 3   →  6 notes × 25 ms
 *   Palier 3 : La5 / Si5   (880 / 988 Hz) × 3   →  6 notes × 25 ms
 *   Palier 4 : Do6 / Ré6  (1046 /1175 Hz) × 3   →  6 notes × 25 ms
 *   Total : 24 notes × 25 ms = 600 ms / cycle
 *
 * Le flag LOOP=true fait reboucler l'engine sur le palier 1 dès la fin,
 * reproduisant exactement le comportement du loop() original.
 *
 * SoundId associé : SFX_GHOST_RETREAT  (voir audio.hpp)
 * Loop            : true  ← OBLIGATOIRE dans SFX_LOOP[]
 * Priorité        : 2 (MEDIUM)
 */

#pragma once
#include "sfx_common.h"

static const AudioNote SEQ_GHOST_RETREAT[] PROGMEM = {
    // ── Palier 1 : Ré5 / Mi5 (587 / 659 Hz) ─────────────────────────────
    N( 587, 25), N( 659, 25),
    N( 587, 25), N( 659, 25),
    N( 587, 25), N( 659, 25),

    // ── Palier 2 : 700 / Sol5 (700 / 784 Hz) ─────────────────────────────
    N( 700, 25), N( 784, 25),
    N( 700, 25), N( 784, 25),
    N( 700, 25), N( 784, 25),

    // ── Palier 3 : La5 / Si5 (880 / 988 Hz) ──────────────────────────────
    N( 880, 25), N( 988, 25),
    N( 880, 25), N( 988, 25),
    N( 880, 25), N( 988, 25),

    // ── Palier 4 : Do6 / Ré6 (1046 / 1175 Hz) ────────────────────────────
    N(1046, 25), N(1175, 25),
    N(1046, 25), N(1175, 25),
    N(1046, 25), N(1175, 25),

    SEQ_END  // → AudioEngine reboucle sur le palier 1 grâce à LOOP=true
};
