//#include <Arduino.h>

const int buzzer = 11;

// --- OCTAVE BASSE : DOUX ET NERVEUX ---
float freq = 150.0;      
float freqMin = 150.0;   // Très grave (Ré 2)
float freqMax = 300.0;   // Plafond bas (Ré 3)
float pas = 0.3;         // Pas ajusté pour que le cycle reste rapide
bool montant = true;     

void setup() {
  pinMode(buzzer, OUTPUT);
}
void loop() {
  // On joue la note en continu pour garder l'aspect "tout plat" (glissade)
  tone(buzzer, (int)freq);

  // Cycle court (seulement 150Hz d'amplitude)
  if (montant) {
    freq += pas;
    if (freq >= freqMax) montant = false;
  } else {
    freq -= pas;
    if (freq <= freqMin) montant = true;
  }

  // Fluidité maximale à 1ms
  delay(1); 
}