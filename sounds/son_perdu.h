//#include <Arduino.h>

const int buzzer = 11;

void setup() {
  pinMode(buzzer, OUTPUT);
}

void loop() {
  // 1. SEQUENCE DE CHUTE (Le son de la mort)
  // On fait plusieurs glissades vers le bas
  for (int i = 0; i < 5; i++) {
    // On part d'une note moyenne (600Hz) vers le grave (200Hz)
    for (int freq = 600; freq > 200; freq -= 15) {
      tone(buzzer, freq);
      // Le délai augmente à chaque répétition (i) pour étirer la chute
      delay(5 + (i * 2)); 
    }
    noTone(buzzer);
    delay(20); // Très courte pause entre les glissades
  }

  // 2. LE "PLOP" FINAL
  // Une chute finale très lente qui s'éteint dans le grave
  for (int freq = 200; freq > 100; freq -= 5) {
    tone(buzzer, freq);
    delay(15);
  }
  
  noTone(buzzer);

  // Pause de 3 secondes avant de rejouer (pour tes tests)
  delay(3000);
}