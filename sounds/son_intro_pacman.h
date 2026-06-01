//#include <Arduino.h>

// Définitions des notes pour l'intro complète
#define NOTE_B4  494
#define NOTE_B5  988
#define NOTE_FS5 740
#define NOTE_DS5 622
#define NOTE_C5  523
#define NOTE_C6  1047
#define NOTE_G6  1568
#define NOTE_E6  1319
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880

int buzzer = 11;

// La mélodie complète de l'arcade (Note, Durée)
// Les durées sont : 16 = double croche, 8 = croche, etc.
int fullIntro[] = {
  NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8, 
  NOTE_C5, 16, NOTE_C6, 16, NOTE_G6, 16, NOTE_E6, 16, NOTE_C6, 32, NOTE_G6, -16, NOTE_E6, 8,
  NOTE_B4, 16, NOTE_B5, 16, NOTE_FS5, 16, NOTE_DS5, 16, NOTE_B5, 32, NOTE_FS5, -16, NOTE_DS5, 8,
  NOTE_DS5, 32, NOTE_E5, 32, NOTE_F5, 32, NOTE_F5, 32, NOTE_FS5, 32, NOTE_G5, 32, 
  NOTE_G5, 32, NOTE_GS5, 32, NOTE_A5, 16, NOTE_B5, 8
};

void setup() {
  pinMode(buzzer, OUTPUT);
  
  // Calcul du tempo
  int wholenote = (60000 * 4) / 105; 

  // Boucle pour jouer toute l'intro
  for (int i = 0; i < sizeof(fullIntro)/sizeof(fullIntro[0]); i += 2) {
    int divider = fullIntro[i+1];
    int noteDuration = 0;
    
    if (divider > 0) {
      noteDuration = wholenote / divider;
    } else {
      noteDuration = (wholenote / abs(divider)) * 1.5;
    }

    tone(buzzer, fullIntro[i], noteDuration * 0.9);
    delay(noteDuration);
    noTone(buzzer);
  }
}

void loop() {
  // Une fois l'intro finie, le loop reste vide ou lance la sirène
}