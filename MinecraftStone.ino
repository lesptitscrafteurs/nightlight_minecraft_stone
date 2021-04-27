#include <Adafruit_NeoPixel.h>

/*     /!\ CHANGEMENT NECESSAIRE /!\      */
/* Nombre de Led dans le montage NeoPixel */
const uint16_t PixelCount = 96;

/*          /!\ CHANGEMENT NECESSAIRE /!\            */
/* Numéro du pin utilisé pour contrôler les NeoPixel */
const uint8_t PixelPin = 6;

/*          /!\ CHANGEMENT NECESSAIRE /!\            */
/* Numéro du pin utilisé pour connaître la valeur du */
/* potentiomètre qui determine la coleur des leds    */
const uint8_t ColorPin = A2;

/*          /!\ CHANGEMENT NECESSAIRE /!\            */
/* Numéro du pin utilisé pour le bouton poussoir     */
const uint8_t ButtonPin = 8;


/* Creation de l'objet pour gérer les NeoPixel */
/* Argument 1 = Number of pixels in NeoPixel strip
   Argument 2 = Arduino pin number (most are valid)
   Argument 3 = Pixel type flags, add together as needed:
      NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
      NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
      NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
      NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
      NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products) */
Adafruit_NeoPixel pixels(PixelCount, PixelPin, NEO_RGBW + NEO_KHZ800);

/* Permet de définir l'intervalle minimum et maximum */
/* pour l'intensité des led NeoPixel                 */
const int MaxBrightness = 255;
const int MinBrightness = 10;
/* Permet de définir l'intervalle minimum et maximum */
/* pour régler le delais de scintillement            */
const int DelayMin      = 30;
const int DelayMax      = 200;

/* liste des modes pour les LEDS */
const int MODE_FIRE               = 0;
const int MODE_BREATH             = 2;

/* Structure qui permettra de gérer le delais entre chaque boucle */
struct DelaySettingsStruct {
  unsigned long StartTimeMillis;
  unsigned long ReqDelayMillis;
};

/* Structure permettant de gérer les variables qui définissent la couleur des led */
struct RGBStruct {
  int red;
  int green;
  int blue;
  int white;
};


/* dernière valeur enregistrée pour le bouton poussoir */
uint8_t lastButtonState   = 0;

/* Variable utilisée pour la gestion du délais des boucles */
DelaySettingsStruct DelaySettings = { 0, 0 };

/* Variable qui permet de connaître la couleur actuellement choisie par l'utilisateur */
RGBStruct currentColor = { 0, 0, 0, 0 };

/* mode actuelle des LEDs */
int currentMode = MODE_FIRE;

int breathDirection = 1;
int breathBright = MaxBrightness;

void setup() {
  /* initialisation du pin du potentiomètre qui gèere les couleurs */
  // pinMode (ColorPin, INPUT);
  /* initialisation du pin du bouton poussoir */
  pinMode (ButtonPin, INPUT_PULLUP);
  /* initialisation / démarrage des leds */
  pixels.begin();
  pixels.clear();
  /* on récupère la couleur indiquée par le potentiomètre */
  currentColor = getUserColor();
  /* on initialise la couleur de tous les pixels avec la couleur souhaitée */
  for (int i=0; i<PixelCount; i++) { 
    pixels.setPixelColor (i, currentColor.red, currentColor.green, currentColor.blue, currentColor.white);
  }
  pixels.setBrightness(255);
  pixels.show();
  /* initialisation du delay */
  setDelay(0);

  Serial.begin(9600);
}

void loop() {
  /* lecture du button */
  int buttonState = digitalRead(ButtonPin);
  /* si la valeur du bouton n'est pas la même que celle enregistrée précédement... */
  if ( buttonState != lastButtonState ) {
    /* cela veut dire que l'utilisateur vient tout juste de presser ou de relacher le bouton */
    if ( buttonState == LOW ) {
      /* Si la valeur du bouton est LOW (relaché)  */
      /* alors on change le mode des pixels */
      currentMode = (currentMode + 2) % 4;
    } else if ( buttonState == HIGH ) {
      /* Si la valeur du bouton est HIGH (appuyé) */
      /* rien d'implémenté pour le moment */
    } 
    lastButtonState = buttonState;
  }
  
  /* on récupère la couleur indiquée par le potentiomètre et on modifie les leds si cette couleur est différente de la précédente */
  RGBStruct color = getUserColor();
  if (color.red != currentColor.red || color.green != currentColor.green || color.blue != currentColor.blue || color.white != currentColor.white) {
    currentColor = color;
    /* on applique la nouvelle couleur à chaque pixels */
    for (int i=0; i<PixelCount; i++) {
      pixels.setPixelColor (i, currentColor.red, currentColor.green, currentColor.blue, currentColor.white);
    }
  }

  /* dans le cas ou le delais pour le scintillement des led est passé */
  if (delayExpired()) {
    if (currentMode == MODE_FIRE) {
      /* on adapte la luminosité */
      // pixels.setBrightness(currentBrightnessMode);
      /* on obtient aléatoirement une nouvelle variation pour l'intensité des leds */
      int newLevel = random(MinBrightness, MaxBrightness);
      /* Nous allons modifier la luminosité de un huitième des leds. pour cela, */
      /* on va choisir aléatoirement un pixel à plusieurs reprise... */
      for (int i=0; i<PixelCount/8; i++) {
        /* et on détermine le pixel à modifier */
        int randomPixel = random(PixelCount);
        /* on modifie la couleur du pixel avec la couleur indiqué sur le potentiomètre tout en lui appliquand la variation d'intensité obtenu de façon aléatoire */
        pixels.setPixelColor (randomPixel, newLevel*currentColor.red/255, newLevel*currentColor.green/255, newLevel*currentColor.blue/255, newLevel*currentColor.white/255 );
      }
      /* on obtient aléatoirement un nouveau délais avant la prochaine modification de led */
      setDelay(random(DelayMin, DelayMax));

      
    } else if (currentMode == MODE_BREATH) {
      for (int i=0; i<PixelCount; i++) {
        pixels.setPixelColor (i, breathBright*currentColor.red/255, breathBright*currentColor.green/255, breathBright*currentColor.blue/255, breathBright*currentColor.white/255 );
      }
      if ( breathBright == 255 ) {
        breathDirection = -1;
      } else if ( breathBright == 0 ) {
        breathDirection = 1;
      }
      breathBright += breathDirection;

      
      if ( breathBright >= 150 && breathBright <= 255 ) setDelay (4);
      if ( breathBright >= 125 && breathBright < 150 ) setDelay (5);
      if ( breathBright >= 100 && breathBright < 125 ) setDelay (7);
      if ( breathBright >= 75  && breathBright < 100 ) setDelay (10);
      if ( breathBright >= 50  && breathBright < 75 ) setDelay (14);
      if ( breathBright >= 25  && breathBright < 50 ) setDelay (18);
      if ( breathBright > 0  && breathBright < 25 ) setDelay (19);
      if ( breathBright ==  0 ) setDelay (194);
    }
      
    /* on affiche la modification sur le NeoPixel */
    pixels.show();
  } 
}

void setDelay (int reqDelay) {
  DelaySettings.StartTimeMillis = millis();
  DelaySettings.ReqDelayMillis = reqDelay;
}

boolean delayExpired () {
  return ((millis() - DelaySettings.StartTimeMillis) >= DelaySettings.ReqDelayMillis);
}

RGBStruct getUserColor () {
  int currentColor = analogRead(ColorPin);
  return mapColor (currentColor);
}

RGBStruct mapColor (int value) {
  RGBStruct color = { 0, 0, 0, 0 };
  if (value < 1023/7) {
    color.red = 255;
    color.green = map(value,0, 1023/7, 0, 255);
    color.blue = 0;
  } else if (value < 1023/7*2) {
    color.red = map(value, 1023/7, 1023/7*2, 255, 0);
    color.green = 255;
    color.blue = 0;
  } else if (value < 1023/7*3) {
    color.red = 0;
    color.green = 255;
    color.blue = map(value,1023/7*2, 1023/7*3, 0, 255);
  } else if (value < 1023/7*4) {
    color.red = 0;
    color.green = map(value, 1023/7*3, 1023/7*4, 255, 0);
    color.blue = 255;
  } else if (value < 1023/7*5) {
    color.red = map(value,1023/7*4, 1023/7*5, 0, 255);
    color.green = 0;
    color.blue = 255;
  } else if (value < 1023/7*6) {
    color.red = 255;
    color.green = 0;
    color.blue = map(value, 1023/7*5, 1023/7*6, 255, 0);
  } else if (value < 1023/7*7) {
    color.red = map(value, 1023/7*6, 1023/7*7, 255, 0);
    color.green = 0;
    color.blue = 0;
    color.white = 255 - color.red;
  } else {
    color.white = 255;
  }
  return color;
}
