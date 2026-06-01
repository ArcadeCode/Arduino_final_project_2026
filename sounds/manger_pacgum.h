//#include <Arduino.h>

const int buzzer = 11;

void setup() {
  pinMode(buzzer, OUTPUT);

  // --- UN SEUL "WAKKA" (UNE PAC-GOMME) ---

  // 1. Le "Wa" (Montée rapide)
  for (int f = 350; f < 600; f += 50) {
    tone(buzzer, f);
    delay(6);
  }
  
  // 2. Le "Kka" (Descente sèche)
  for (int f = 550; f > 250; f -= 60) {
    tone(buzzer, f);
    delay(5);
  }
  
  // On coupe le son définitivement
  noTone(buzzer); 
}

void loop() {
  // On laisse vide pour ne pas que le son se répète
}