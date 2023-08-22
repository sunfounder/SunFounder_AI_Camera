# SunFounder AI Camera Library for Arduino

In the process of Arduino board communication with the SunFounder Controller app, each widget requires specific functions to read/write their values.

Please note that the examples provided assume usage within an Arduino-like environment. Adjustments may be needed based on the specific programming context.

---

### Robot Get the APP's Data

**getSlider(uint8_t region)**

This function is used to retrieve the value of a Slider widget.

**Parameters**
- `region`: The region where the widget is located on the SunFounder Controller. It should be of type `uint8_t` and can be assigned a value like `3` or `REGION_D`.

**Return Value**
The return value depends on the settings in the SunFounder Controller app. If the widget's minimum and maximum values are set to 0 and 180 respectively, the return value will range from 0 to 180.

**Example**

```cpp
int16_t sliderD = aiCam.getSlider(REGION_D);
Serial.print("Slider D: ");
Serial.println(sliderD);
```

---

**getButton(uint8_t region)**

This function is used to retrieve the state of a Button widget.

**Parameters**
- `region`: The region where the widget is located on the SunFounder Controller. It should be of type `uint8_t` and can be assigned a value like `4` or `REGION_E`.

**Return Value**
The return value is a boolean indicating the state of the button. It will be `true` if the button is pressed and `false` otherwise.

**Example**

```cpp
bool buttonA = aiCam.getButton(REGION_E);
if (buttonA) {
    Serial.println("Button A is pressed");
} else {
    Serial.println("Button A is not pressed");
}
```

---

**getSwitch(uint8_t region)**

This function is used to retrieve the state of a Switch widget.

**Parameters**
- `region`: The region where the widget is located on the SunFounder Controller. It should be of type `uint8_t` and can be assigned a value like `5` or `REGION_F`.

**Return Value**
The return value is a boolean indicating the state of the switch. It will be `true` if the switch is in the ON position and `false` if it is in the OFF position.

**Example**

```cpp
bool switchB = aiCam.getSwitch(REGION_F);
if (switchB) {
    Serial.println("Switch B is ON");
} else {
    Serial.println("Switch B is OFF");
}
```

---

**getJoystick(uint8_t region, uint8_t axis)**

This function is used to retrieve the value of a specific axis of a Joystick widget.

**Parameters**
- `region`: The region where the widget is located on the SunFounder Controller. It should be of type `uint8_t` and can be assigned a value like `10` or `REGION_K`.
- `axis`: The axis of the joystick to retrieve the value from. It should be of type `uint8_t` and can be assigned a value such as `JOYSTICK_X` or `JOYSTICK_Y`.

**Return Value**
The return value is an `int16_t` representing the position or value of the specified joystick axis. The range of the return value depends on the joystick's configuration.

**Example**

```cpp
int16_t joystickX = aiCam.getJoystick(REGION_K, JOYSTICK_X);
int16_t joystickY = aiCam.getJoystick(REGION_K, JOYSTICK_Y);
Serial.print("Joystick: ");
Serial.print(joystickX);
Serial.print(",");
Serial.println(joystickY);
```

---

**getDPad(uint8_t region)**

This function is used to retrieve the

 state of a D-pad widget.

**Parameters**
- `region`: The region where the widget is located on the SunFounder Controller. It should be of type `uint8_t` and can be assigned a value like `10` or `REGION_K`.

**Return Value**
The return value is a `uint8_t` representing the state of the D-pad. The value can be interpreted as follows:
- `DPAD_STOP`: The D-pad is not pressed.
- `DPAD_FORWARD`: The D-pad is pressed up.
- `DPAD_BACKWARD`: The D-pad is pressed down.
- `DPAD_LEFT`: The D-pad is pressed left.
- `DPAD_RIGHT`: The D-pad is pressed right.

**Example**

```cpp
uint8_t dpadState = aiCam.getDPad(REGION_K);
switch (dpadState) {
    case DPAD_STOP:
        Serial.println("D-pad is not pressed");
        break;
    case DPAD_FORWARD:
        Serial.println("D-pad is pressed up");
        break;
    case DPAD_BACKWARD:
        Serial.println("D-pad is pressed down");
        break;
    case DPAD_LEFT:
        Serial.println("D-pad is pressed left");
        break;
    case DPAD_RIGHT:
        Serial.println("D-pad is pressed right");
        break;
}
```

---

**getThrottle(uint8_t region)**

This function is used to retrieve the value of a Throttle widget.

**Parameters**
- `region`: The region where the widget is located on the SunFounder Controller. It should be of type `uint8_t` and can be assigned a value like `16` or `REGION_Q`.

**Return Value**
The return value is an `int16_t` representing the position or value of the throttle. The range of the return value depends on the throttle's configuration.

**Example**

```cpp
int16_t throttleValue = aiCam.getThrottle(REGION_Q);
Serial.print("Throttle value: ");
Serial.println(throttleValue);
```

---

**getSpeech(uint8_t region, char* result)**

This function is used to retrieve the speech input from a Speech widget.

**Parameters**
- `region`: The region where the widget is located on the SunFounder Controller. It should be of type `uint8_t` and can be assigned a value like `8` or `REGION_I`.
- `result`: A character array (string) where the speech input will be stored. Make sure the array has enough capacity to hold the speech input.

**Return Value**
This function does not return a value directly. Instead, it populates the `result` array with the speech input.

**Example**

```cpp
char speechResult[50]; // Assuming the speech input can fit in a 50-character array
aiCam.getSpeech(REGION_I, speechResult);
Serial.print("Speech input from Region I: ");
Serial.println(speechResult);
```

---

### Robot Send the Data to APP

The robot periodically sends data to the app, typically with a cycle of 60ms. The data sent is in the form of a dictionary. We can make some widgets on the app display sensor readings by filling in the values of the sensors in the `sendDoc[]` dictionary.

You can write four types of values for four types of widgets:

- Gauge: Float or integer number
- Radar: [Integer angle, Float distance]
- GreyScale Indicator: [Integer reading1, Integer reading2, Integer reading3]
- Number: Float or integer number

**Example**

```cpp
aiCam.sendDoc["N"] = 1; // for number widget
float usDistance = int(ultrasonicRead() * 100) / 100.0

; // round two decimal places
aiCam.sendDoc["O"] = usDistance; // for number or gauge widget
aiCam.sendDoc["H"] = {100, 203, 61}; // for greyscale widget
aiCam.sendDoc["L"] = {90, usDistance}; // for radar widget
```

---
### Control Flash Lamp

**Example**
```cpp
aiCam.lamp_on(); // turn on cam lamp
aiCam.lamp_on(5); // set brightness level (0 to 10)
aiCam.lamp_off(); // turn off cam lamp

```

---