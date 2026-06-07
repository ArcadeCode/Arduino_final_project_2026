/**
 * sfx_level_win.h — Fanfare de fin de niveau
 *
 * Source originale : son_gagner.h
 *
 * Conversion
 * ─────────────────────────────────────────────────────────────────────────────
 * Le sketch original jouait 7 notes avec tone()+delay() bloquants.
 * Chaque paire (tone, delay) est remplacée par une entrée AudioNote.
 * Les durées sont légèrement réduites (×0.9) et un REST() est inséré
 * entre chaque note pour reproduire le côté "piqué" de l'original.
 *
 * Notes originales et durées :
 *   C5 70ms · E5 70ms · G5 70ms · C6 70ms ← 4 premières (rapides)
 *   E5 50ms · G5 50ms                      ← accélération
 *   C6 350ms                               ← note finale tenue
 *
 * SoundId associé : SFX_LEVEL_WIN  (voir audio.hpp)
 * Loop            : false
 * Priorité        : 1 (HIGH)
 */

#pragma once
#include "sfx_common.h"

#define SFX_WIN_C5   523
#define SFX_WIN_E5   659
#define SFX_WIN_G5   784
#define SFX_WIN_C6  1047

static const AudioNote SEQ_LEVEL_WIN[] PROGMEM = {
    // ── 4 notes rapides ───────────────────────────────────────────────────
    N(SFX_WIN_C5, 63), REST(7),
    N(SFX_WIN_E5, 63), REST(7),
    N(SFX_WIN_G5, 63), REST(7),
    N(SFX_WIN_C6, 63), REST(7),

    // ── Accélération ──────────────────────────────────────────────────────
    N(SFX_WIN_E5, 45), REST(5),
    N(SFX_WIN_G5, 45), REST(5),

    // ── Note finale tenue ─────────────────────────────────────────────────
    N(SFX_WIN_C6, 315), REST(35),

    SEQ_END
};
