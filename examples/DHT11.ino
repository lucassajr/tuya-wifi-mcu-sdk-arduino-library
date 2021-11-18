/*
 * @FileName: DHT11.ino 
 * @LastEditors: lucassajr
 * @Description: 
 * @Github:https://github.com/tuya/tuya-wifi-mcu-sdk-arduino-library
 */
//#include <Wire.h>
#include <TuyaWifi.h>
#include "DHT.h"
#include <SoftwareSerial.h>

TuyaWifi my_device;

/* Current LED status */
unsigned char led_state = 0;
/* Connect network button pin */
int wifi_key_pin = 7;

#define DHTPIN 5 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
DHT dht (DHTPIN, DHTTYPE);

/* Data point define */
#define DPID_TEMP_CURRENT     1
#define DPID_HUMIDITY_CURRENT 2

/* Current device DP values */

int temp = 35;
int hum = 0;

/* Stores all DPs and their types. PS: array[][0]:dpid, array[][1]:dp type. 
 *                                     dp type(TuyaDefs.h) : DP_TYPE_RAW, DP_TYPE_BOOL, DP_TYPE_VALUE, DP_TYPE_STRING, DP_TYPE_ENUM, DP_TYPE_BITMAP
*/
unsigned char dp_array[][2] =
{
  {DPID_TEMP_CURRENT, DP_TYPE_VALUE},
  {DPID_HUMIDITY_CURRENT, DP_TYPE_VALUE},
};

unsigned char pid[] = {"laszfmvkohqrx7hn"};
unsigned char mcu_ver[] = {"3.1.4"};

/* last time */
unsigned long last_time = 0;

void setup()
{
  Serial.begin(9600);

  dht.begin(); /////////////////////////

  //Initialize led port, turn off led.
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  //Initialize networking keys.
  pinMode(wifi_key_pin, INPUT_PULLUP);

  my_device.init(pid, mcu_ver);
//  incoming all DPs and their types array, DP numbers
  my_device.set_dp_cmd_total(dp_array, 2);
 // register DP download processing callback function
  my_device.dp_process_func_register(dp_process);
 // register upload all DP callback function
  my_device.dp_update_all_func_register(dp_update_all);

  delay(300);
  last_time = millis();
}

void loop()
{
  my_device.uart_service();
  
  //Enter the connection network mode when Pin7 is pressed.
  if (digitalRead(wifi_key_pin) == LOW) {
    delay(80);
    if (digitalRead(wifi_key_pin) == LOW) {
      my_device.mcu_set_wifi_mode(SMART_CONFIG);
    }
  }
  /* LED blinks when network is being connected */
  if ((my_device.mcu_get_wifi_work_state() != WIFI_LOW_POWER) && (my_device.mcu_get_wifi_work_state() != WIFI_CONN_CLOUD) && (my_device.mcu_get_wifi_work_state() != WIFI_SATE_UNKNOW)) {
    if (millis()- last_time >= 500) {
      last_time = millis();

      if (led_state == LOW) {
        led_state = HIGH;
      } else {
        led_state = LOW;
      }

      digitalWrite(LED_BUILTIN, led_state);
    }
  }

  /* get the temperature and humidity */

float h = dht.readHumidity();
float t = dht.readTemperature();
float f = dht.readTemperature(true);
hum = h;
temp = t*10;
  
//Serial.print(F("Humidity: "));
//Serial.print(h);
//Serial.print(F("& Temperature :"));
//Serial.print(temp);

  /* report the temperature and humidity */
  if ((my_device.mcu_get_wifi_work_state() == WIFI_CONNECTED) || (my_device.mcu_get_wifi_work_state() == WIFI_CONN_CLOUD)) {
    my_device.mcu_dp_update(DPID_TEMP_CURRENT, temp, 1);
    my_device.mcu_dp_update(DPID_HUMIDITY_CURRENT, hum, 1);
  }

  delay(1000);
}

/**
 * @description: DP download callback function.
 * @param {unsigned char} dpid
 * @param {const unsigned char} value
 * @param {unsigned short} length
 * @return {unsigned char}
 */
unsigned char dp_process(unsigned char dpid,const unsigned char value[], unsigned short length)
{
  /* all DP only report */
  return TY_SUCCESS;
}

/**
 * @description: Upload all DP status of the current device.
 * @param {*}
 * @return {*}
 */
void dp_update_all(void)
{
  my_device.mcu_dp_update(DPID_TEMP_CURRENT, temp, 1);
  my_device.mcu_dp_update(DPID_HUMIDITY_CURRENT, hum, 1);
}
