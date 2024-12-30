#include "SunFounder_AI_Camera.h"

/**
 *  functions for manipulating string
 */
#define IsStartWith(str, prefix) (strncmp(str, prefix, strlen(prefix)) == 0)
#define StrAppend(str, suffix) \
  uint32_t len = strlen(str);  \
  str[len] = suffix;           \
  str[len + 1] = '\0'
#define StrClear(str) str[0] = 0

int32_t cmdTimeout = SERIAL_TIMEOUT;
int32_t wsSendTime = millis();
int32_t wsSendInterval = 60; // 100

int32_t volSendTime = millis();
int32_t volSendInterval = 5000;

/**
 * Declare global variables
 */
char name[25];
char type[25];

/**
 * Declare the receive callback function
 */
void (*__onReceive__)();
void (*__onReceiveBinary__)();

/**
 * @brief instantiate AiCamera Class, set name and type
 * @param _name set name
 * @param _type set type
 */
AiCamera::AiCamera(const char *_name, const char *_type)
{
  strcpy(name, _name);
  strcpy(type, _type);
}

/** !!!!!!!     Plan to deprecate   !!!!!!!
 *
 * @brief Set wifi and websocket port to esp32-cam,
 *        block and wait for the setting to succeed
 *
 * @param ssid  wifi ssid
 * @param password wifi password
 * @param wifiMode  0,None; 1, STA; 2, AP
 * @param wsPort websocket server port
 */
void AiCamera::begin(const char *ssid, const char *password, const char *wifiMode, const char *wsPort)
{
// !!!!!!!     Plan to deprecate   !!!!!!!
#ifdef AI_CAM_DEBUG_CUSTOM
  DateSerial.begin(115200);
#endif
  char ip[25];
  char version[25];

  setCommandTimeout(3000);
  this->get("RESET", version);
  DebugSerial.print(F("ESP32 firmware version "));
  DebugSerial.println(version);

  setCommandTimeout(1000);
  this->set("NAME", name);
  this->set("TYPE", type);
  this->set("SSID", ssid);
  this->set("PSK", password);
  this->set("MODE", wifiMode);
  this->set("PORT", wsPort);

  setCommandTimeout(5000);
  this->get("START", ip);
  delay(20);
  DebugSerial.print(F("WebServer started on ws://"));
  DebugSerial.print(ip);
  DebugSerial.print(F(":"));
  DebugSerial.println(wsPort);
  DebugSerial.print(F("Video streamer started on http://"));
  DebugSerial.print(ip);
  DebugSerial.println(F(":9000/mjpg"));

  setCommandTimeout(SERIAL_TIMEOUT);
}

/**
 * @brief Set wifi and websocket port to esp32-cam,
 *        block and wait for the setting to succeed
 *
 * @param ssid  wifi ssid
 * @param password wifi password
 * @param wifiMode  0,None; 1, STA; 2, AP
 * @param wsPort websocket server port
 */
void AiCamera::begin(const char *ssid, const char *password, const char *wsPort, bool autoSend)
{
#ifdef ARDUINO_MINIMA
  DataSerial.begin(115200);
#endif
  char ip[25];
  char version[25];
  this->autoSend = autoSend;

  setCommandTimeout(3000);
  this->get("RESET", version);
  DebugSerial.print(F("ESP32 firmware version "));
  DebugSerial.println(version);
  if (!checkFirmwareVersion(String(version)))
  {
    DebugSerial.print(F("ESP32 firmware version not match, minial firmware version is "));
    DebugSerial.print(MINIMAL_VERSION_MAJOR);
    DebugSerial.print(F("."));
    DebugSerial.print(MINIMAL_VERSION_MINOR);
    DebugSerial.print(F("."));
    DebugSerial.println(MINIMAL_VERSION_PATCH);
    DataSerial.println(F("ESP32 firmware version not match"));
    return;
  }

  setCommandTimeout(1000);
  delay(1000);
  this->set("NAME", name);
  this->set("TYPE", type);
  this->set("APSSID", ssid);
  this->set("APPSK", password);
  this->set("PORT", wsPort);

  setCommandTimeout(10000);
  this->get("START", ip);
  delay(20);
  DebugSerial.print(F("WebServer started on ws://"));
  DebugSerial.print(ip);
  DebugSerial.print(F(":"));
  DebugSerial.println(wsPort);
  DebugSerial.print(F("Video streamer started on http://"));
  DebugSerial.print(ip);
  DebugSerial.println(F(":9000/mjpg"));

  setCommandTimeout(SERIAL_TIMEOUT);
}

/**
 * @brief Set callback function method for receive
 *
 * @param func  callback function pointer
 */
void AiCamera::setOnReceived(void (*func)()) { __onReceive__ = func; }

/**
 * @brief Set callback function method for receive binary
 *
 * @param func  callback function pointer
 */
void AiCamera::setOnReceivedBinary(void (*func)()) { __onReceiveBinary__ = func; }

/**
 * @brief Receive and process serial port data in a loop
 */
void AiCamera::loop()
{
  this->readInto(recvBuffer);
  if (strlen(recvBuffer) != 0 || recvBufferType != WS_BUFFER_TYPE_NONE)
  {
    // Serial.print("recv: ");Serial.println(recvBuffer);

    // ESP32-CAM reboot detection
    if (IsStartWith(recvBuffer, CAM_INIT))
    {
      // Serial.println(F("ESP32-CAM reboot detected"));
      ws_connected = false;
    }
    // ESP32-CAM websocket connected
    else if (IsStartWith(recvBuffer, WS_CONNECT))
    {
      // Serial.println(F("ESP32-CAM websocket connected"));
      ws_connected = true;
    }
    // ESP32-CAM websocket disconnected
    else if (IsStartWith(recvBuffer, WS_DISCONNECT))
    {
      // Serial.println(F("ESP32-CAM websocket disconnected"));
      ws_connected = false;
    }
    // ESP32-CAM APP_STOP
    else if (IsStartWith(recvBuffer, APP_STOP))
    {
      if (ws_connected)
      {
        Serial.println(F("APP STOP"));
      }
      ws_connected = false;
    }
    // recv WS+ data
    else if (IsStartWith(recvBuffer, WS_HEADER))
    {
      debug("RX:");
      debug(recvBuffer);
      // DataSerial.print("RX:"); DataSerial.println(recvBuffer);
      ws_connected = true;
      this->subString(recvBuffer, strlen(WS_HEADER));
      if (__onReceive__ != NULL)
      {
        __onReceive__();
        // if (millis() - wsSendTime > wsSendInterval) {
        //   this->sendData();
        //   wsSendTime = millis();
        // }
      }
    }
    // recv WSB+ binary data
    else if (recvBufferType == WS_BUFFER_TYPE_BINARY)
    {
      ws_connected = true;
      // this->subString(recvBuffer, strlen(WS_BIN_HEADER));
      if (__onReceiveBinary__ != NULL)
      {
        __onReceiveBinary__();
      }
    }

    if (this->autoSend)
    {
      if (millis() - wsSendTime > wsSendInterval)
      {
        this->sendData();
        wsSendTime = millis();
      }
    }

    recvBufferType = WS_BUFFER_TYPE_NONE;
  }
}

/**
 * @brief Print the information received from esp32-CAm,
 *        according to the set of CAM_DEBUG_LEVEL
 *
 * @param msg Message to be detected
 */
void AiCamera::debug(char *msg)
{
#if (CAM_DEBUG_LEVEL == CAM_DEBUG_LEVEL_ALL) // all
  DebugSerial.println(msg);
#elif (CAM_DEBUG_LEVEL == CAM_DEBUG_LEVEL_ERROR) // error
  if (IsStartWith(msg, CAM_DEBUG_HEAD_ERROR))
  {
    DebugSerial.println(msg);
  }
#elif (CAM_DEBUG_LEVEL == CAM_DEBUG_LEVEL_INFO)  // info
  if (IsStartWith(msg, CAM_DEBUG_HEAD_ERROR))
  {
    DebugSerial.println(msg);
  }
  else if (IsStartWith(msg, CAM_DEBUG_HEAD_INFO))
  {
    DebugSerial.println(msg);
  }
#elif (CAM_DEBUG_LEVEL == CAM_DEBUG_LEVEL_DEBUG) // debug
  if (IsStartWith(msg, CAM_DEBUG_HEAD_ERROR))
  {
    DebugSerial.println(msg);
  }
  else if (IsStartWith(msg, CAM_DEBUG_HEAD_INFO))
  {
    DebugSerial.println(msg);
  }
  else if (IsStartWith(msg, CAM_DEBUG_HEAD_DEBUG))
  {
    DebugSerial.println(msg);
  }
#endif
}

/**
 * @brief Store the data read from the serial port into the buffer
 *
 * @param buffer  Pointer to the String value of the stored data
 */
void AiCamera::readInto(char *buffer)
{
  /* !!! attention buffer size*/
  bool finished = false;
  bool isBinary = false;
  bool binaryDataStarted = false;
  uint8_t binaryByteCount = 0;
  uint8_t binaryDataLength = 0;
  uint8_t binaryChecksum = 0;
  uint8_t inchar;
  uint32_t count = 0;
  StrClear(buffer);

  uint32_t char_time = millis();

  // recv Byte
  while (DataSerial.available())
  {
    count += 1;
    if (count > WS_BUFFER_SIZE)
    {
      finished = true;
      break;
    }
    inchar = (uint8_t)DataSerial.read();
    if (isBinary)
    {
      // Start Byte
      // DebugSerial.print(F("binaryDataStarted: "));DebugSerial.println(binaryDataStarted);
      // DebugSerial.print(binaryByteCount);
      // DebugSerial.print(F(": 0x"));DebugSerial.println(inchar, HEX);
      if (!binaryDataStarted && binaryByteCount == 0)
      {
        if (inchar == BIN_START_BYTE)
        {
          // DebugSerial.println("binary start");
          binaryDataStarted = true;
          StrClear(buffer);
          binaryByteCount = 1;
          continue;
        }
        else
        {
          DebugSerial.print(F("binary start byte error: 0x"));
          DebugSerial.println(inchar, HEX);
          continue;
        }
      } // Length Byte
      else if (binaryDataStarted && binaryByteCount == 1)
      {
        binaryDataLength = inchar;
        // DebugSerial.print(F("data length: "));DebugSerial.println(binaryDataLength);
      } // Checksum Byte
      else if (binaryDataStarted && binaryByteCount == 2)
      {
        binaryChecksum = inchar;
        // DebugSerial.print(F("checksum: "));DebugSerial.println(binaryChecksum);
      } // End Byte
      else if (binaryDataStarted && binaryByteCount == binaryDataLength + 3)
      {
        if (inchar != BIN_END_BYTE)
        {
          DebugSerial.println(F("end byte error"));
          continue;
        }
        // DebugSerial.println(F("binary end byte"));
        uint8_t checksum = buffer[0];
        for (uint8_t i = 1; i < binaryDataLength; i++)
        {
          checksum ^= buffer[i];
        }
        if (checksum != binaryChecksum)
        {
          DebugSerial.print(F("checksum error, expect: "));
          DebugSerial.print(checksum);
          DebugSerial.print(", actual: ");
          DebugSerial.println(binaryChecksum);
          continue;
        }
        finished = true;
        binaryDataStarted = false;
        binaryByteCount = 0;
        break;
      } // Data Byte
      else if (binaryDataStarted)
      {
        uint8_t index = binaryByteCount - 3;
        // DebugSerial.print("index: ");DebugSerial.print(index);
        // DebugSerial.print(", data: 0x");DebugSerial.println(inchar, HEX);
        buffer[index] = inchar;
      }
      binaryByteCount += 1;
    }
    else
    {
      if (inchar == '\n')
      {
        finished = true;
        break;
      }
      else if (inchar == '\r')
      {
        continue;
      }
      else if ((int)inchar > 31 && (int)inchar < 127)
      {
        StrAppend(buffer, inchar);
        if (IsStartWith(buffer, WS_BIN_HEADER))
        {
          // DebugSerial.println("binary data start");
          isBinary = true;
        }
        delay(1); // Wait for StrAppend
      }
    }
  }

  // if recv debug info
  if (finished)
  {
    if (isBinary)
    {
      recvBufferType = WS_BUFFER_TYPE_BINARY;
      recvBufferLength = binaryDataLength;
    }
    else
    {
      debug(buffer);
      recvBufferType = WS_BUFFER_TYPE_TEXT;
    }
    if (IsStartWith(buffer, CAM_DEBUG_HEAD_DEBUG))
    {
#if (CAM_DEBUG_LEVEL == CAM_DEBUG_LEVEL_DEBUG) // all
      DebugSerial.print(CAM_DEBUG_HEAD_DEBUG);
      DebugSerial.println(buffer);
#endif
      StrClear(buffer);
    }
  }
}

/**
 * @brief Serial port sends data, automatically adds header (WS_HEADER)
 */
void AiCamera::sendData()
{
  DataSerial.print(F(WS_HEADER));
  // sendDoc["A"] = 0;
  serializeJson(sendDoc, DataSerial);
  DataSerial.print("\n");
}

/**
 * @brief Send binary data
 *
 * @param data binary data
 * @param len data length
 */
void AiCamera::sendBinaryData(uint8_t *data, size_t len)
{
  DataSerial.print(F(WS_BIN_HEADER));
  DataSerial.write(data, len);
  DataSerial.print("\n");
}

/**
 * @brief Set command timeout
 *
 * @param _timeout timeout value
 */
void AiCamera::setCommandTimeout(uint32_t _timeout)
{
  cmdTimeout = _timeout;
}

/**
 * @brief Send command to ESP32-CAM with serial
 *
 * @param command command keyword
 * @param value
 * @param result returned information from serial
 */
void AiCamera::command(const char *command, const char *value, char *result, bool wait)
{
  bool is_ok = false;
  uint8_t retry_count = 0;
  uint8_t retryMaxCount = 3;

  while (retry_count < retryMaxCount)
  {
    DataSerial.flush();
    DataSerial.print(F("SET+"));
    DataSerial.print(command);
    DataSerial.println(value);
    DataSerial.print(F("..."));
    if (!wait)
      return; // if not waiting, return immediately

    retry_count++;

    uint32_t st = millis();
    while ((millis() - st) < cmdTimeout)
    {
      this->readInto(recvBuffer);
      // if (recvBuffer[0] == 0) continue;
      // DebugSerial.println(recvBuffer);
      if (IsStartWith(recvBuffer, OK_FLAG))
      {
        is_ok = true;
        DataSerial.println(F(OK_FLAG));
        this->subString(recvBuffer, strlen(OK_FLAG) + 1); // Add 1 for Space
        // !!! Note that the reslut size here is too small and may be out of bounds,
        // causing unexpected data changes
        strcpy(result, recvBuffer);
        break;
      }
    }

    if (is_ok == true)
    {
      break;
    }
  }

  if (is_ok == false)
  {
    Serial.println(F("[FAIL]"));
    while (1)
      ;
  }
  DataSerial.flush();
}

/**
 * @brief Use the comand() function to set up the ESP32-CAM
 *
 * @param command command keyword
 */
void AiCamera::set(const char *command, bool wait)
{
  char result[10];
  this->command(command, "", result, wait);
}

/**
 * @brief Use the comand() function to set up the ESP32-CAM
 *
 * @param command command keyword
 * @param value
 *
 * @code {.cpp}
 * set("NAME", "Zeus_Car");
 * set("TYPE", "Zeus_Car");
 * set("SSID", "Zeus_Car");
 * set("PSK",  "12345678");
 * set("MODE", WIFI_MODE_AP);
 * set("PORT", "8765");
 * @endcode
 *
 */
void AiCamera::set(const char *command, const char *value, bool wait)
{
  char result[10];
  this->command(command, value, result, wait);
}

/**
 * @brief Use the comand() function to set up the ESP32-CAM,
 *        and receive return information
 *
 * @param command command keyword
 * @param value
 * @param result returned information from serial
 * @code {.cpp}
 * char ip[15];
 * get("START", ip);
 * @endcode
 */
void AiCamera::get(const char *command, char *result)
{
  this->command(command, "", result);
}

/**
 * @brief Use the comand() function to set up the ESP32-CAM,
 *        and receive return information
 *
 * @param command command keyword
 * @param value
 * @param result returned information from serial
 */
void AiCamera::get(const char *command, const char *value, char *result)
{
  this->command(command, value, result);
}

/**
 * @brief Interpret the value of the slider contorl component from the buf string
 *
 * @param buf string pointer to be interpreted
 * @param region the key of component
 * @return the value of the slider contorl component
 */
int16_t AiCamera::getSlider(uint8_t region)
{
  int16_t value = getIntOf(recvBuffer, region);
  return value;
}

/**
 * @brief Interpret the value of the Button component from the buf string
 *
 * @param buf string pointer to be interpreted
 * @param region the key of component
 * @return the value of the Joystick component
 *        - true
 *        - flase
 */
bool AiCamera::getButton(uint8_t region)
{
  bool value = getBoolOf(recvBuffer, region);
  return value;
}

/**
 * @brief Interpret the value of the getSwitch component from the buf string
 *
 * @param buf string pointer to be interpreted
 * @param region the key of component
 * @return the value of the Joystick component
 *        - true
 *        - flase
 */
bool AiCamera::getSwitch(uint8_t region)
{
  bool value = getBoolOf(recvBuffer, region);
  return value;
}

/**
 * @brief Interpret the value of the Joystick component from the buf string
 *
 * @param buf string pointer to be interpreted
 * @param region the key of component
 * @param axis which type of value that you want,
 *             could be JOYSTICK_X, JOYSTICK_Y, JOYSTICK_ANGLE, JOYSTICK_RADIUS
 * @return the value of the Joystick component
 */
int16_t AiCamera::getJoystick(uint8_t region, uint8_t axis)
{
  char valueStr[20];
  int16_t x, y, angle, radius;
  getStrOf(recvBuffer, region, valueStr, ';');
  x = getIntOf(valueStr, 0, ',');
  y = getIntOf(valueStr, 1, ',');
  angle = atan2(x, y) * 180.0 / PI;
  radius = sqrt(y * y + x * x);
  switch (axis)
  {
  case JOYSTICK_X:
    return x;
  case JOYSTICK_Y:
    return y;
  case JOYSTICK_ANGLE:
    return angle;
  case JOYSTICK_RADIUS:
    return radius;
  default:
    return 0;
  }
}

/**
 * @brief Interpret the value of the DPad component from the buf string
 *
 * @param buf string pointer to be interpreted
 * @param region the key of component
 *
 * @return the value of the DPadDPad component,
 *         it could be null, "forward", "backward", "left", "stop"
 */
uint8_t AiCamera::getDPad(uint8_t region)
{
  char value[20];
  getStrOf(recvBuffer, region, value, ';');
  uint8_t result;
  if ((String)value == (String) "forward")
    result = DPAD_FORWARD;
  else if ((String)value == (String) "backward")
    result = DPAD_BACKWARD;
  else if ((String)value == (String) "left")
    result = DPAD_LEFT;
  else if ((String)value == (String) "right")
    result = DPAD_RIGHT;
  else if ((String)value == (String) "stop")
    result = DPAD_STOP;
  return result;
}

/**
 * @brief Interpret the value of the Throttle component from the buf string
 *
 * @param buf string pointer to be interpreted
 * @param region the key of component
 *
 * @return the value of the Throttle component,
 */
int16_t AiCamera::getThrottle(uint8_t region)
{
  int16_t value = getIntOf(recvBuffer, region);
  return value;
}

/**
 * @brief Interpret the value of the Speech component from the buf string
 *
 * @param buf string pointer to be interpreted
 * @param region the key of component
 * @param result char array pointer to hold the result
 * @return the value of the Joystick component
 */
void AiCamera::getSpeech(uint8_t region, char *result)
{
  getStrOf(recvBuffer, region, result, ';');
}

/**
 * @brief Fill the value of Meter display component into the buf to be sent
 *
 * @param region the key of component
 * @param value the value to be filled
 */
void AiCamera::setMeter(uint8_t region, double value)
{
  setStrOf(recvBuffer, region, String(value));
}

/**
 * @brief Fill the value of Radar display component into the buf to be sent
 *
 * @param region the key of component
 * @param angle the orientation of the obstacle
 * @param distance the distance of the obstacle
 */
void AiCamera::setRadar(uint8_t region, int16_t angle, double distance)
{
  setStrOf(recvBuffer, region, String(angle) + "," + String(distance));
}

/**
 * @brief Fill the value of 3-way grayscale display component into the buf to be sent
 *
 * @param region the key of component
 * @param value1
 * @param value2
 * @param value3
 */
void AiCamera::setGreyscale(uint8_t region, uint16_t value1, uint16_t value2, uint16_t value3)
{
  setStrOf(recvBuffer, region, String(value1) + "," + String(value2) + "," + String(value3));
}

void AiCamera::setValue(uint8_t region, double value)
{
  setStrOf(recvBuffer, region, String(value));
}

/**
 * @brief subtract part of the string
 *
 * @param str string pointer to be subtract
 * @param start start position of content to be subtracted
 * @param end end position of Content to be subtracted
 */
void AiCamera::subString(char *str, int16_t start, int16_t end)
{
  uint8_t length = strlen(str);
  if (end == -1)
  {
    end = length;
  }
  for (uint8_t i = 0; i < end; i++)
  {
    if (i + start < end)
    {
      str[i] = str[i + start];
    }
    else
    {
      str[i] = '\0';
    }
  }
}

/**
 * @brief Split the string by a cdivider,
 *         and return characters of the selected index
 *
 * @param str string pointer to be split
 * @param index which index do you wish to return
 * @param result char array pointer to hold the result
 * @param divider
 */
void AiCamera::getStrOf(char *str, uint8_t index, char *result, char divider)
{
  uint8_t start, end;
  uint8_t length = strlen(str);
  uint8_t i, j;

  // Get start index
  if (index == 0)
  {
    start = 0;
  }
  else
  {
    for (start = 0, j = 1; start < length; start++)
    {
      if (str[start] == divider)
      {
        if (index == j)
        {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++)
  {
    if (str[end] == divider)
    {
      break;
    }
  }
  // Copy result
  // if ((end - start + 2) > sizeof(result)) { // '\0' takes up one byte
  //   end = start + sizeof(result) -1;
  // }

  for (i = start, j = 0; i < end; i++, j++)
  {
    result[j] = str[i];
  }
  result[j] = '\0';
}

/**
 * @brief split by divider, filling the value to a position in the string
 *
 * @param str string pointer to be operated
 * @param index which index do you wish to return
 * @param value the value to be filled
 * @param divider
 */
void AiCamera::setStrOf(char *str, uint8_t index, String value, char divider)
{
  uint8_t start, end;
  uint8_t length = strlen(str);
  uint8_t i, j;
  // Get start index
  if (index == 0)
  {
    start = 0;
  }
  else
  {
    for (start = 0, j = 1; start < length; start++)
    {
      if (str[start] == divider)
      {
        if (index == j)
        {
          start++;
          break;
        }
        j++;
      }
    }
  }
  // Get end index
  for (end = start, j = 0; end < length; end++)
  {
    if (str[end] == divider)
    {
      break;
    }
  }
  String strString = str;
  String strValue = strString.substring(0, start) + value + strString.substring(end);
  strcpy(str, strValue.c_str());
}

/**
 * @brief Split the string by a cdivider,
 *         and return characters of the selected index.
 *         Further, the content is converted to int type.
 *
 * @param buf string pointer to be split
 * @param index which index do you wish to return
 * @param divider
 */
int16_t AiCamera::getIntOf(char *str, uint8_t index, char divider)
{
  int16_t result;
  char strResult[20];
  getStrOf(str, index, strResult, divider);
  result = String(strResult).toInt();
  return result;
}

bool AiCamera::getBoolOf(char *str, uint8_t index)
{
  char strResult[20];
  getStrOf(str, index, strResult, ';');
  return String(strResult).toInt();
}

double AiCamera::getDoubleOf(char *str, uint8_t index)
{
  double result;
  char strResult[20];
  getStrOf(str, index, strResult, ';');
  result = String(strResult).toDouble();
  return result;
}

void AiCamera::lamp_on(uint8_t level)
{
  set("LAMP", (char *)String(level).c_str(), false);
}

void AiCamera::lamp_off(void)
{
  set("LAMP", "0", false);
}

/**
 * @brief Check the firmware version of the camera
 * Check if the firmware version of the camera greater than or equal to the version
 *
 * @param version the version to be compared
 */
bool AiCamera::checkFirmwareVersion(String version)
{
  String temp;
  Serial.print(F("checkFirmwareVersion: "));
  Serial.println(version);
  int major = version.substring(0, version.indexOf(".")).toInt();
  temp = version.substring(version.indexOf(".") + 1, version.length());
  int minor = temp.substring(0, temp.indexOf(".")).toInt();
  temp = temp.substring(temp.indexOf(".") + 1, temp.length());
  int patch = temp.substring(0, temp.indexOf(".")).toInt();
  if (major < MINIMAL_VERSION_MAJOR)
  {
    return false;
  }
  if (minor < MINIMAL_VERSION_MINOR)
  {
    return false;
  }
  if (patch < MINIMAL_VERSION_PATCH)
  {
    return false;
  }
  return true;
}

/**
 * @brief Reset the camera
 *
 * @param wait wait for the camera to restart
 */
void AiCamera::reset(bool wait)
{
  set("RESET", wait);
}
