#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
//#include "/Users/aki.salminen/Verstas/BasicOTA/BasicOTA/BasicOTA.ino"
#include <FastLED.h>

#include <Wire.h>
#include <Adafruit_MLX90614.h>
  
#define LED_PIN     6
#define PIR_PIN     D5
#define COLOR_ORDER GRB
#define CHIPSET     WS2812
#define NUM_LEDS    44

#define BRIGHTNESS  100
#define FRAMES_PER_SECOND 60

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

bool gReverseDirection = false;

CRGB leds[NUM_LEDS];
CRGB lcolor;

DEFINE_GRADIENT_PALETTE( heatmap_256 ) {
  0,     0,  0,  255,   //blue
  //64,   0,  20,   50,   //blue
  128,   0,  255,   0,   //green
  255,   255,  0,   0};   //red

//224,   255,255,  0,   //bright yellow
//255,   255,255,255 }; //full white
/*
DEFINE_GRADIENT_PALETTE( heatmap_44 ) {
  0,     0,  0,  255,   //blue
  21,   0,  255,   0,   //green
  43,   255,  0,   0};   //red
*/

void setup() {
  delay(300); // sanity delay

  Serial.begin(115200);

  //pinMode(1, OUTPUT);      // sets the digital pin 1 as output
  pinMode(PIR_PIN, INPUT_PULLUP);        // sets the digital pin 0 as input

  //MLX
  Serial.println("MLX sart");
  delay(200); // sanity delay
  mlx.begin();
  Serial.println("MLX done");
   
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );

/*  Serial.println("OTA setup");
  delay(200); // sanity delay
  otasetup();*/
}

void moveDots(int dot)
{
    static CRGBPalette256 myPal = heatmap_256;
    static int firemode=0;
    firemode = getfiremode();     
    char string[64];

    while( firemode ) {
//      otaloop();
      Fire2012();
      delay(20);
      firemode = getfiremode();     
    }
     
    while( digitalRead(PIR_PIN)  ) {
//      otaloop();
      delay(20);     
      Serial.println("Liikettä.");
      firemode = getfiremode();     
    }
    //digitalWrite(ledPin, val);    // sets the LED to the button's value
    lcolor = ColorFromPalette( myPal, (25500/(NUM_LEDS-1)+1)*dot/100 ); // normal palette access
    snprintf(string, sizeof(string), "dot: %d color index: %d\n", dot, (25500/(NUM_LEDS-1)+1)*dot/100 );
    Serial.print(string);
    leds[dot] = lcolor;
    //leds[dot-1] = lcolor;
    //leds[dot-2] = lcolor;
    //leds[dot-3] = lcolor;
    FastLED.show();
    // clear this led for the next time around the loop
    //leds[dot] = CRGB::Black;
    leds[dot+3] = CRGB::Black;
    leds[dot-3] = CRGB::Black;
    //if(dot<(NUM_LEDS-1)) leds[dot-3] = CRGB::Black;
    delay(80);  
}

int getfiremode()
{
  //MLX
  float temp=mlx.readObjectTempC();
  Serial.print("Ambient = "); 
  Serial.print(mlx.readAmbientTempC()); 
  Serial.print("*C\tObject = "); 
  Serial.print(temp); 
  Serial.println("*C");
  delay(20);
  if( temp > 30 ) return 1;
  return 0;       
}

void loop()
{
  /*
  Serial.println("Loop");
  snprintf( string, sizeof(string), "Pir: %d\n", digitalRead(PIR_PIN) );
  Serial.print(string);
  //digitalRead(PIR_PIN);
  delay(200);
  */
  delay(20);
//  otaloop();

  //FastLED.show(); // display this frame
  //FastLED.delay(1000 / FRAMES_PER_SECOND);
  //delay(1000 / FRAMES_PER_SECOND); // delay

  //Moving dots forward
  for(int dot = 3; dot < (NUM_LEDS-1); dot++) {
    moveDots(dot);
    }
  //Moving dots backwards
  for(int dot = NUM_LEDS-1; dot >= 0; dot--) { 
    moveDots(dot);
   }  
}


// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
//// 
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation, 
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking. 
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100 
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


void Fire2012()
{
  // Add entropy to random number generator; we use a lot of it.
  //random16_add_entropy( random8());

  //Serial.println("fire");
  //Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }
  
    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }
    
    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
 delay(80); // delay
 FastLED.show();
}
