/**
 * sfx_common.h — Macros partagées par tous les fichiers sounds/
 *
 * Ce fichier ne contient que les macros d'écriture des séquences.
 */

#pragma once
#include "../audio.hpp"  // AudioNote, PROGMEM — chemin relatif depuis sounds/

// ── Macros d'écriture ─────────────────────────────────────────────────────────
#define N(f, d)  {(uint16_t)(f), (uint16_t)(d)}   // Note normale
#define REST(d)  {0,             (uint16_t)(d)}    // Silence
#define SEQ_END  {0,             0            }    // Terminateur de séquence