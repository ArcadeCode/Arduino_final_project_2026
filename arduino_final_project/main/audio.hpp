/**
 * audio.hpp — Non-blocking Audio Engine for Arduino Pac-Man
 *
 * Architecture
 * ────────────────────────────────────────────────────────────────────────────
 * Sounds are played via a priority queue of "audio events". Each event is
 * a sequence of (frequency, duration_ms) pairs stored in PROGMEM.
 *
 * The engine is driven by AudioEngine::update(), which must be called once
 * per game loop iteration. It checks if the current note has elapsed using
 * millis() and advances the sequence — no blocking, no delay().
 *
 * Hardware
 * ────────────────────────────────────────────────────────────────────────────
 * Target: passive buzzer on pin AUDIO_PIN, driven by Arduino tone() API.
 * tone() itself uses Timer2 in the background, so the square wave is
 * generated in hardware without CPU polling. The RC filter on the buzzer
 * output smooths the square wave into a quasi-sine for better timbre.
 *
 * Priority system
 * ────────────────────────────────────────────────────────────────────────────
 * Each sound has a priority level (0 = highest). A new sound will interrupt
 * a currently playing sound only if its priority is strictly lower (higher
 * urgency). Sounds of equal or lower urgency are queued and played after
 * the current one finishes.
 *
 *   Priority 0 — CRITICAL  : Death jingle (always interrupts)
 *   Priority 1 — HIGH      : Level complete, Intro
 *   Priority 2 — MEDIUM    : Frightened mode loop, Ghost retreat loop
 *   Priority 3 — LOW       : Normal movement loop (wakka), Eat ghost
 *
 * Continuous sounds (frightened, normal movement) use a loop flag that
 * restarts the sequence automatically when it reaches the end.
 *
 * Usage (from game.cpp or main.ino)
 * ────────────────────────────────────────────────────────────────────────────
 *   // In setup():
 *   AudioEngine::init();
 *
 *   // In loop() — MUST be called every iteration:
 *   AudioEngine::update();
 *
 *   // Trigger events:
 *   AudioEngine::play(SFX_EAT_DOT);
 *   AudioEngine::play(SFX_DEATH);        // interrupts everything
 *   AudioEngine::play(SFX_FRIGHTENED);   // loops until stop() or override
 *   AudioEngine::stop();                 // silence immediately
 */

#pragma once
#include <Arduino.h>
#include <avr/pgmspace.h>

// ── Pin ──────────────────────────────────────────────────────────────────────
#define AUDIO_PIN 3  // Must be pin 11 on Uno/Mega for Timer2 / tone() compatibility

// ── Note sentinel — marks end of a sequence ──────────────────────────────────
#define NOTE_END  0
#define DUR_END   0

// ── Sound IDs ────────────────────────────────────────────────────────────────
enum SoundId : uint8_t {
    SFX_NONE        = 0,
    SFX_INTRO       = 1,   // Startup jingle
    SFX_EAT_DOT     = 2,   // "Wakka" — one pellet
    SFX_EAT_GHOST   = 3,   // Ghost eaten (not yet in sounds/ but easy to add)
    SFX_DEATH       = 4,   // Pac-Man death sequence
    SFX_LEVEL_WIN   = 5,   // Level cleared
    SFX_FRIGHTENED  = 6,   // Frightened mode loop (continuous)
    SFX_NORMAL_MOVE = 7,   // Normal movement ambient loop (continuous)
    SFX_GHOST_RETREAT = 8, // Ghost-retreat ascending loop (continuous)
    _SFX_COUNT
};

// ── Priority table (lower number = higher urgency) ────────────────────────────
// Index matches SoundId
static const uint8_t SFX_PRIORITY[_SFX_COUNT] PROGMEM = {
    255,  // SFX_NONE        — never played
    1,    // SFX_INTRO
    3,    // SFX_EAT_DOT
    2,    // SFX_EAT_GHOST
    0,    // SFX_DEATH       — highest priority
    1,    // SFX_LEVEL_WIN
    2,    // SFX_FRIGHTENED
    3,    // SFX_NORMAL_MOVE
    2,    // SFX_GHOST_RETREAT
};

// ── Loop flag (sounds that restart automatically) ─────────────────────────────
// Index matches SoundId
static const bool SFX_LOOP[_SFX_COUNT] PROGMEM = {
    false, // SFX_NONE
    false, // SFX_INTRO
    false, // SFX_EAT_DOT
    false, // SFX_EAT_GHOST
    false, // SFX_DEATH
    false, // SFX_LEVEL_WIN
    true,  // SFX_FRIGHTENED  — loops until overridden
    true,  // SFX_NORMAL_MOVE — loops until overridden
    true,  // SFX_GHOST_RETREAT — loops until overridden
};

// ─────────────────────────────────────────────────────────────────────────────
// Note sequence type:  array of uint16_t pairs (freq, dur_ms) in PROGMEM.
// Terminated by {NOTE_END, DUR_END}.
// ─────────────────────────────────────────────────────────────────────────────
struct AudioNote {
    uint16_t freq;   // Hz — 0 = silence (rest)
    uint16_t dur_ms; // Duration in milliseconds
};

// ── AudioEngine — static singleton ───────────────────────────────────────────
class AudioEngine {
public:
    /**
     * @brief Call once in setup(). Configures the audio pin.
     */
    static void init();

    /**
     * @brief Must be called every game loop iteration.
     *        Advances the current note sequence if the note has elapsed.
     *        Handles looping, silence gaps, and sequence end.
     */
    static void update();

    /**
     * @brief Request playback of a sound.
     *        The sound will interrupt the current one only if its priority
     *        is strictly higher (lower number). Otherwise it is ignored
     *        (fire-and-forget model — no queue to avoid SRAM pressure).
     *
     * @param id  One of the SoundId enum values.
     */
    static void play(SoundId id);

    /**
     * @brief Immediately stop all audio and silence the buzzer.
     */
    static void stop();

    /**
     * @brief Return the SoundId currently playing (SFX_NONE if silent).
     */
    static SoundId currentSound();

private:
    // Pointer into the current sequence (PROGMEM)
    static const AudioNote* _seq;
    static uint8_t          _noteIndex;
    static unsigned long    _noteStartMs;
    static SoundId          _currentId;
    static bool             _loop;

    // Advance to the next note in the sequence (or loop / stop).
    static void _nextNote();

    // Look up the PROGMEM sequence for a given SoundId.
    static const AudioNote* _getSequence(SoundId id);
};
