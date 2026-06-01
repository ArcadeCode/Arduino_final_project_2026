//#include <Arduino.h>

const int buzzer = 11;

void setup() {
  pinMode(buzzer, OUTPUT);
}

void loop() {
  // --- LA MÉLODIE COMPLÈTE DU RETOUR DES YEUX (GHOST RETREAT) ---
  // Le son est composé de 4 paliers de hauteur différente.
  // Chaque palier alterne très vite entre deux notes (effet de trille).

  // PALIER 1 : Le départ (Médium-Grave)
  for (int i = 0; i < 3; i++) {
    tone(buzzer, 587); delay(25); // Ré 5
    tone(buzzer, 659); delay(25); // Mi 5
  }

  // PALIER 2 : La montée 1
  for (int i = 0; i < 3; i++) {
    tone(buzzer, 700); delay(25); 
    tone(buzzer, 784); delay(25); // Sol 5
  }

  // PALIER 3 : La montée 2
  for (int i = 0; i < 3; i++) {
    tone(buzzer, 880); delay(25); // La 5
    tone(buzzer, 988); delay(25); // Si 5
  }

  // PALIER 4 : Le sommet (Aigu)
  for (int i = 0; i < 3; i++) {
    tone(buzzer, 1046); delay(25); // Do 6
    tone(buzzer, 1175); delay(25); // Ré 6
  }

  // Pas de noTone() ici ! 
  // La boucle se relance tout de suite pour faire l'effet de cassure
  // quand le son retombe brutalement au Palier 1.
}