/**
 * sfx_intro.h — Jingle d'intro Pac-Man
 *
 * Source originale : son_intro_pacman.h
 *
 * Conversion
 * ─────────────────────────────────────────────────────────────────────────────
 * Tempo = 105 BPM → ronde = 60000*4/105 = 2286 ms
 *
 *   Valeur   | Durée brute | ×0.9 (lié) | gap (0.1)
 *   ─────────┼─────────────┼────────────┼──────────
 *   16e      |  143 ms     |  128 ms    |  15 ms
 *   16e ptée |  214 ms     |  192 ms    |  22 ms
 *   8e       |  286 ms     |  257 ms    |  29 ms
 *   32e      |   71 ms     |   64 ms    |   7 ms
 *
 * Le gap est simulé ici en réduisant la durée de la note (×0.9 arrondi),
 * l'engine ne génère pas de silence explicite entre les notes — c'est le
 * changement de fréquence qui crée la séparation perceptive.
 * Si tu veux des silences explicites, ajoute un REST(15) entre chaque note.
 *
 * SoundId associé : SFX_INTRO  (voir audio.hpp)
 * Loop            : false
 * Priorité        : 1 (HIGH)
 */

#pragma once
#include "sfx_common.h"

// Fréquences utilisées dans cette séquence
#define SFX_INTRO_B4   494
#define SFX_INTRO_DS5  622
#define SFX_INTRO_E5   659
#define SFX_INTRO_F5   698
#define SFX_INTRO_FS5  740
#define SFX_INTRO_G5   784
#define SFX_INTRO_GS5  831
#define SFX_INTRO_A5   880
#define SFX_INTRO_B5   988
#define SFX_INTRO_C5   523
#define SFX_INTRO_C6  1047
#define SFX_INTRO_E6  1319
#define SFX_INTRO_G6  1568

static const AudioNote SEQ_INTRO[] PROGMEM = {
    // ── Phrase 1 ──────────────────────────────────────────────────────────
    N(SFX_INTRO_B4,  128),  // 16e
    N(SFX_INTRO_B5,  128),  // 16e
    N(SFX_INTRO_FS5, 128),  // 16e
    N(SFX_INTRO_DS5, 128),  // 16e
    N(SFX_INTRO_B5,   64),  // 32e
    N(SFX_INTRO_FS5, 192),  // 16e ptée
    N(SFX_INTRO_DS5, 257),  // 8e  (dernière note tenue)

    // ── Phrase 2 ──────────────────────────────────────────────────────────
    N(SFX_INTRO_C5,  128),
    N(SFX_INTRO_C6,  128),
    N(SFX_INTRO_G6,  128),
    N(SFX_INTRO_E6,  128),
    N(SFX_INTRO_C6,   64),
    N(SFX_INTRO_G6,  192),
    N(SFX_INTRO_E6,  257),

    // ── Phrase 3 — répétition de la phrase 1 ─────────────────────────────
    N(SFX_INTRO_B4,  128),
    N(SFX_INTRO_B5,  128),
    N(SFX_INTRO_FS5, 128),
    N(SFX_INTRO_DS5, 128),
    N(SFX_INTRO_B5,   64),
    N(SFX_INTRO_FS5, 192),
    N(SFX_INTRO_DS5, 257),

    // ── Run final (gamme chromatique ascendante) ──────────────────────────
    N(SFX_INTRO_DS5,  64),  // 32e
    N(SFX_INTRO_E5,   64),
    N(SFX_INTRO_F5,   64),
    N(SFX_INTRO_F5,   64),
    N(SFX_INTRO_FS5,  64),
    N(SFX_INTRO_G5,   64),
    N(SFX_INTRO_G5,   64),
    N(SFX_INTRO_GS5,  64),
    N(SFX_INTRO_A5,  128),  // 16e
    N(SFX_INTRO_B5,  257),  // 8e tenue — note finale

    SEQ_END
};
