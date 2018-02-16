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

#define BRIGHTNESS  15
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

void showTemp()
{
    static CRGBPalette256 myPal = heatmap_256;
    char string[64];

    //MLX
    float temp=mlx.readObjectTempC();
    Serial.print("Ambient = "); 
    Serial.print(mlx.readAmbientTempC()); 
    Serial.print("*C\tObject = "); 
    Serial.print(temp); 
    Serial.println("*C");
    
    FastLED.clear ();
    
    int led = (temp*10-200)/5;
    Serial.print("Led: ");
    Serial.println(led);
    if( led > 44 ) led=44;
    if( led < 0 ) led=0;
    
    for(int dot=0; dot < led; dot++) {
    lcolor = ColorFromPalette( myPal, (25500/(NUM_LEDS-1)+1)*dot/100 ); // normal palette access
    snprintf(string, sizeof(string), "dot: %d color index: %d\n", dot, (25500/(NUM_LEDS-1)+1)*dot/100 );
    Serial.print(string);
      leds[dot] = lcolor;
      }
    FastLED.show();
}

void loop()
{
  delay(20);
  showTemp();
}

