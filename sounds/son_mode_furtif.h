//#include <Arduino.h>

const int buzzer = 11;

void setup() {
  pinMode(buzzer, OUTPUT);
}

void loop() {
  // On simule le son "Frightened Monster" de Pac-Man
  // C'est un balayage (sweep) ultra-rapide et répétitif
  
  for (int repetition = 0; repetition < 50; repetition++) {
    
    // Phase 1 : Montée très rapide et serrée
    for (int f = 400; f < 550; f += 20) {
      tone(buzzer, f);
      delay(4); // Vitesse du balayage
    }
    
    // Phase 2 : Descente très rapide
    for (int f = 550; f > 400; f -= 20) {
      tone(buzzer, f);
      delay(4);
    }
  }

  noTone(buzzer);
  delay(2000); 
}