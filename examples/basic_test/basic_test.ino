/**
 * Basic test example for SunFounder AI Camera
 * Here list all avaiable commands. Every command have an explanation.
 * If there's an "optional" lable on the end on the explanation, then it's optional.
 * Or you must have that command to make the device work.
 */

#include "SunFounder_AI_Camera.h"

/**
 * Configure Wifi mode, SSID, password
 */
// AP Mode
#define WIFI_MODE WIFI_MODE_AP
#define SSID "AiCamera"
#define PASSWORD "12345678"

// STA Mode
// #define WIFI_MODE WIFI_MODE_STA
// #define SSID "xxxxxxxxxx"
// #define PASSWORD "xxxxxxxxxx"

/** Configure device name */
#define NAME "My Camera"

/** Configure device type */
#define TYPE "AiCamera"

/**
 * Configure device port
 * Set port to 8765 for SunFounder Controller APP
 */
#define PORT "8765"

/**
 * Create AiCamera Object
 */
AiCamera aiCam = AiCamera(NAME, TYPE);

/**
 * Define a function on data received
 * after received, use all get/set functions to get set all the data or values
 */
void onReceive() {
  int16_t sliderD = aiCam.getSlider(REGION_D);
  bool buttonE = aiCam.getButton(REGION_E);
  bool switchF = aiCam.getSwitch(REGION_F);
  int16_t joystickKX = aiCam.getJoystick(REGION_K, JOYSTICK_X);
  int16_t joystickKY = aiCam.getJoystick(REGION_K, JOYSTICK_Y);
  //uint8_t dpadK = aiCam.getDPad(REGION_K);
  int16_t throttleQ = aiCam.getThrottle(REGION_Q);
  char speechI[20];
  aiCam.getSpeech(REGION_I, speechI);
  aiCam.setMeter(REGION_C, 20);
  aiCam.setRadar(REGION_D, 90, 20);
  aiCam.setGreyscale(REGION_B, 100, 200, 300);
  aiCam.setValue(REGION_G, 20);
  Serial.print("Received: ");
  Serial.print("Slider D: ");
  Serial.print(sliderD);
  Serial.print(" Button E: ");
  Serial.print(buttonE);
  Serial.print(" Switch F: ");
  Serial.print(switchF);
  Serial.print(" Joystick K: [");
  Serial.print(joystickKX);
  Serial.print(", ");
  Serial.print(joystickKY);
  Serial.print("]");
  // Serial.print(" DPad K: ");
  // switch(dpadK){
  //   case DPAD_STOP: Serial.print("STOP"); break;
  //   case DPAD_FORWARD: Serial.print("FORWARD"); break;
  //   case DPAD_BACKWARD: Serial.print("BACKEARD"); break;
  //   case DPAD_LEFT: Serial.print("LEFT"); break;
  //   case DPAD_RIGHT: Serial.print("RIGHT"); break;
  // }
  Serial.print(" Throttle Q: ");
  Serial.print(throttleQ);
  Serial.print(" Speech I: ");
  Serial.print(speechI);
  Serial.println();
}


void setup() {
  // Initialize Serial(optional, if you need to debug)
  Serial.begin(115200);
  // Initialize AiCamera
  aiCam.begin(SSID, PASSWORD, WIFI_MODE, PORT);
  // Set on data received function
  aiCam.setOnReceived(onReceive);
  // Set command timeout (optional, default 100)
  aiCam.setCommandTimeout(100);
  //
}

void loop() {
  // AiCamera loop. must put it in loop(). And more importantly, every time loop
  // runs, a perioad of data send receive runs, so loop must be fast enough.
  // or it will be laggy or even stuck by other code.
  aiCam.loop();
}