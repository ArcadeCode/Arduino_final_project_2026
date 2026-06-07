/**
 * audio.cpp — Non-blocking Audio Engine implementation.
 *
 * Les séquences sonores ne sont PAS définies ici.
 * Elles sont stockées dans sounds/sfx_*.h sous forme de tableaux
 * AudioNote[] PROGMEM, inclus ci-dessous dans l'ordre des SoundId.
 *
 * Pour ajouter un son :
 *   1. Créer sounds/sfx_monson.h  avec SEQ_MON_SON[] PROGMEM
 *   2. Ajouter SFX_MON_SON dans enum SoundId       (audio.hpp)
 *   3. Ajouter sa priorité dans SFX_PRIORITY[]     (audio.hpp)
 *   4. Ajouter son flag dans SFX_LOOP[]            (audio.hpp)
 *   5. #include "sounds/sfx_monson.h"              (ici)
 *   6. Référencer SEQ_MON_SON dans SFX_SEQUENCES[] (ici)
 */

#include "audio.hpp"

// ── Inclusion des séquences (order = SoundId enum order) ─────────────────────
#include "sounds/sfx_intro.h"         // SEQ_INTRO         — SFX_INTRO
#include "sounds/sfx_eat_dot.h"       // SEQ_EAT_DOT       — SFX_EAT_DOT
#include "sounds/sfx_eat_ghost.h"     // SEQ_EAT_GHOST     — SFX_EAT_GHOST
#include "sounds/sfx_death.h"         // SEQ_DEATH         — SFX_DEATH
#include "sounds/sfx_level_win.h"     // SEQ_LEVEL_WIN     — SFX_LEVEL_WIN
#include "sounds/sfx_frightened.h"    // SEQ_FRIGHTENED    — SFX_FRIGHTENED
#include "sounds/sfx_normal_move.h"   // SEQ_NORMAL_MOVE   — SFX_NORMAL_MOVE
#include "sounds/sfx_ghost_retreat.h" // SEQ_GHOST_RETREAT — SFX_GHOST_RETREAT

// ── Table de dispatch SoundId → séquence PROGMEM ─────────────────────────────
// Stockée elle-même en PROGMEM : un pgm_read_ptr() suffit pour récupérer
// le pointeur vers la séquence — zéro octet de SRAM consommé par cette table.
static const AudioNote* const SFX_SEQUENCES[_SFX_COUNT] PROGMEM = {
    nullptr,              // SFX_NONE         — jamais joué
    SEQ_INTRO,            // SFX_INTRO
    SEQ_EAT_DOT,          // SFX_EAT_DOT
    SEQ_EAT_GHOST,        // SFX_EAT_GHOST
    SEQ_DEATH,            // SFX_DEATH
    SEQ_LEVEL_WIN,        // SFX_LEVEL_WIN
    SEQ_FRIGHTENED,       // SFX_FRIGHTENED
    SEQ_NORMAL_MOVE,      // SFX_NORMAL_MOVE
    SEQ_GHOST_RETREAT,    // SFX_GHOST_RETREAT
};

// ── Définitions des membres statiques ────────────────────────────────────────
const AudioNote* AudioEngine::_seq         = nullptr;
uint8_t          AudioEngine::_noteIndex   = 0;
unsigned long    AudioEngine::_noteStartMs = 0;
SoundId          AudioEngine::_currentId   = SFX_NONE;
bool             AudioEngine::_loop        = false;

// ─────────────────────────────────────────────────────────────────────────────
// init()
// ─────────────────────────────────────────────────────────────────────────────
void AudioEngine::init() {
    pinMode(AUDIO_PIN, OUTPUT);
    noTone(AUDIO_PIN);
}

// ─────────────────────────────────────────────────────────────────────────────
// _getSequence() — lit le pointeur depuis la table PROGMEM
// ─────────────────────────────────────────────────────────────────────────────
const AudioNote* AudioEngine::_getSequence(SoundId id) {
    if (id == SFX_NONE || id >= _SFX_COUNT) return nullptr;
    return (const AudioNote*)pgm_read_ptr(&SFX_SEQUENCES[id]);
}

// ─────────────────────────────────────────────────────────────────────────────
// play()
// ─────────────────────────────────────────────────────────────────────────────
void AudioEngine::play(SoundId id) {
    if (id == SFX_NONE || id >= _SFX_COUNT) return;

    uint8_t newPriority = pgm_read_byte(&SFX_PRIORITY[id]);
    uint8_t curPriority = (_currentId == SFX_NONE)
                          ? 255
                          : pgm_read_byte(&SFX_PRIORITY[_currentId]);

    // Ignore si la priorité du nouveau son est inférieure (nombre plus grand)
    if (newPriority > curPriority) return;

    _seq         = _getSequence(id);
    _noteIndex   = 0;
    _currentId   = id;
    _loop        = (bool)pgm_read_byte(&SFX_LOOP[id]);
    _noteStartMs = millis();

    if (_seq == nullptr) { stop(); return; }

    // Joue la première note immédiatement
    uint16_t freq   = pgm_read_word(&_seq[0].freq);
    uint16_t dur_ms = pgm_read_word(&_seq[0].dur_ms);

    if (dur_ms == 0) { stop(); return; }  // SEQ_END immédiat (séquence vide)

    if (freq == 0) noTone(AUDIO_PIN);
    else           tone(AUDIO_PIN, freq);
}

// ─────────────────────────────────────────────────────────────────────────────
// stop()
// ─────────────────────────────────────────────────────────────────────────────
void AudioEngine::stop() {
    noTone(AUDIO_PIN);
    _seq       = nullptr;
    _noteIndex = 0;
    _currentId = SFX_NONE;
    _loop      = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// _nextNote() — avance dans la séquence, gère la boucle
// ─────────────────────────────────────────────────────────────────────────────
void AudioEngine::_nextNote() {
    _noteIndex++;

    uint16_t freq   = pgm_read_word(&_seq[_noteIndex].freq);
    uint16_t dur_ms = pgm_read_word(&_seq[_noteIndex].dur_ms);

    if (dur_ms == 0) {
        // Fin de séquence
        if (_loop) {
            _noteIndex = 0;
            freq   = pgm_read_word(&_seq[0].freq);
            dur_ms = pgm_read_word(&_seq[0].dur_ms);
            if (dur_ms == 0) { stop(); return; }  // Garde contre boucle vide
        } else {
            stop();
            return;
        }
    }

    _noteStartMs = millis();

    if (freq == 0) noTone(AUDIO_PIN);
    else           tone(AUDIO_PIN, freq);
}

// ─────────────────────────────────────────────────────────────────────────────
// update() — à appeler à chaque itération du game loop
// ─────────────────────────────────────────────────────────────────────────────
void AudioEngine::update() {
    if (_seq == nullptr || _currentId == SFX_NONE) return;

    uint16_t dur_ms = pgm_read_word(&_seq[_noteIndex].dur_ms);
    if (dur_ms == 0) { stop(); return; }

    if ((millis() - _noteStartMs) >= dur_ms) {
        _nextNote();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// currentSound()
// ─────────────────────────────────────────────────────────────────────────────
SoundId AudioEngine::currentSound() {
    return _currentId;
}
