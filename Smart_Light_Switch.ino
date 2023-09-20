#include "WiFi.h"
#include <ezButton.h>
#include <Espalexa.h>
#include <ESP32Servo.h>
#include <FastLED.h>

#define NUM_LEDS 5
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define BRIGHTNESS  100
#define DATA_PIN D0 // ESP32S3 pin D0 connected to LED strip

#define SERVO_PIN D1 // ESP32S3 pin D1 connected to servo motor

#define Touch_Button D10 // ESP32S3 pin D10 connected to capacitive touch sensor

Servo servoMotor;
int switch_state = 0;
int last_state = 0;

ezButton button(Touch_Button);  // create ezButton object that attach to pin D10;
int button_state = 0;

// Define the array of leds
CRGB leds[NUM_LEDS];

// Replace with your network credentials
const char* ssid = "WIFI_NETWORK";
const char* password = "WIFI_PASSWORD";

boolean wifi_connection = false;

Espalexa smart_switch_network;
EspalexaDevice* device1;

void setup() {
  Serial.begin(11520);
  servoMotor.attach(SERVO_PIN); // attaches the servo on ESP32 pin

  button.setDebounceTime(100); // set debounce time to 100 milliseconds
  button.setCountMode(COUNT_RISING);

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
    .setCorrection(TypicalLEDStrip)
    .setDither(BRIGHTNESS < 255);

  // set master brightness control
  FastLED.setBrightness(BRIGHTNESS);

  // Connect to WiFi
  initWiFi();

  // Add Alexa device
  device1 = new EspalexaDevice("Smart Switch", smart_light_switch);
  smart_switch_network.addDevice(device1);
  smart_switch_network.begin();
}

void loop() {
  // Needed for espalex and ezbutton
  smart_switch_network.loop();
  button.loop();

  // Get current state of switch, check if button is pressed
  switch_state = device1->getValue();
  switch_state = check_button(switch_state);

  // Update switch position
  device1->setValue(switch_state);
  switch_servo(switch_state);
  switch_state = last_state;
  delay(1);
}

// Function toggles servo
void switch_servo(int state){
  if(state > 0){
    // Move switch to on
    servoMotor.write(50);
  }
  else{
    // Turn/leave switch off
    servoMotor.write(140);
  }
}

int check_button(int state){
  // Check state of the button,
  if (button.isPressed() && button_state == 0){
    Serial.println("Button is pressed");
    if (state > 0) {state = 0;}
    else{state = 255;}
    button_state = 1;
    for (int i = 0; i < 5; i++){
      leds[i] = CRGB::Green;
    }
    FastLED.show();
  }
  else if (button.isReleased()){ 
    button_state = 0;
    for (int i = 0; i < 5; i++){
      leds[i] = CRGB::Red;
    }
    FastLED.show();
    delay(500);
  }
  return state;
}

// Alexa device to be manipulated
int smart_light_switch(int state){
  Serial.print("Device 1 changed to ");
    if (state > 0) {
      Serial.println("ON");
    }
    else  {
      Serial.println("OFF");
    }
    return state;
}

boolean initWiFi() {
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  boolean state = false;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi .");
  while (WiFi.status() != WL_CONNECTED) { // Loop until WiFi is connected
    Serial.print('.');
    delay(1000);
  }
  state = true;
  Serial.println();
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
  return state;
}

