//#include <Arduino.h>

const int buzzer = 11;

// Notes officielles du jeu pour la victoire
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047

void setup() {
  pinMode(buzzer, OUTPUT);

  // --- SÉQUENCE OFFICIELLE DE FIN DE NIVEAU ---

  // 1. Les 7 notes montantes (rapides et piquées)
  tone(buzzer, NOTE_C5); delay(70);
  tone(buzzer, NOTE_E5); delay(70);
  tone(buzzer, NOTE_G5); delay(70);
  tone(buzzer, NOTE_C6); delay(70);
  
  // Petite accélération sur la fin
  tone(buzzer, NOTE_E5); delay(50);
  tone(buzzer, NOTE_G5); delay(50);
  
  // Note finale tenue et victorieuse
  tone(buzzer, NOTE_C6); delay(350);

  // 2. On coupe le son proprement
  noTone(buzzer);
}

void loop() {
  // Vide pour ne jouer la victoire qu'une fois par niveau gagné
}