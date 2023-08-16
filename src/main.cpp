
#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#endif
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ESP8266mDNS.h>
#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ArduinoOTA.h>
#include "TickTwo.h"
#include "RTClib.h"
#include <Wire.h>
#include <Ticker.h>

#include <SPI.h>
const char *ssid = "Alarm WiFi";
const char *password = "alarm1234";

uint8_t completed, next_timer = 0, current_min;
uint8_t minute, hour, day;
ulong trigger_delay = 10000;
bool exam_mode = false;
long time_now = 0;
String current_time;

String timers[9], exam_timers[2];
void trigger();
void checkTrigger();
void restartTrigger();
void compareTimings(String a[]);
void checkExamTimings();
/* AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/timings", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                       {

  JsonObject jsonObj = json.as<JsonObject>();
  Serial.println(jsonObj); }); */

// Webserver
AsyncWebServer server(80);
// Timer 1
TickTwo class_alarm(trigger, trigger_delay, 2);
// check every minute
TickTwo check_trigger(checkTrigger, 5000, 0, MILLIS);
// restart check trigger
TickTwo restart_trigger(restartTrigger, 50000, 1, MILLIS);

// check exam timers
TickTwo check_exam(checkExamTimings, 5000, 0, MILLIS);
// rtc
RTC_DS1307 rtc;

const int LED_PIN = D5; // LED_BUILTIN;

void checkExamTimings()
{

  Serial.println(WiFi.localIP());

  compareTimings(exam_timers);
}

AsyncCallbackJsonWebHandler *examHandler = new AsyncCallbackJsonWebHandler("/examTimings", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                           {
                                                                              Serial.flush();

  Serial.print("new timings..\nparsing..");
  StaticJsonDocument<420> data;
  if (json.is<JsonObject>())
  {
    data = json.as<JsonObject>();
    String tmp ;
    String temp_arr[2] =  {"sTime","eTime"};
    
    for(size_t i = 0;i < 2; i++){

    const char* temp = data[temp_arr[i]];
    Serial.println(temp);
    exam_timers[i] = temp;
        
     }
    Serial.println("\nStarting exam trigger loop");
    Serial.print(exam_timers[0] + exam_timers[1]);
     check_trigger.pause();
     exam_mode = true;
     check_exam.start();

  }
  
  String response;
  serializeJson(data, response);
  request->send(200, "application/json", response);
  Serial.println(response); });

AsyncCallbackJsonWebHandler *timeHandler = new AsyncCallbackJsonWebHandler("/setTime", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                           {
                                                                              Serial.flush();

  Serial.print("set time..\nparsing..");
  StaticJsonDocument<420> data;
  int tm;   
  if (json.is<JsonObject>())
  {
    data = json.as<JsonObject>();
    String tmp ;
    
    const char* temp = data["time"];
    Serial.println(temp);
     tm = atoi(temp);
   
    
    DateTime t = tm;
     rtc.adjust(DateTime(t));
    Serial.println("\nnew time : " + String(rtc.now().secondstime()));


  }
  
  String response;
  serializeJson(data, response);
  request->send(200, "application/json", response);
  Serial.println(response); });

AsyncCallbackJsonWebHandler *delayHandler = new AsyncCallbackJsonWebHandler("/setDelay", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                            {
                                                                              Serial.flush();

  Serial.print("set delay..\nparsing..");
  StaticJsonDocument<420> data;
  if (json.is<JsonObject>())
  {
    data = json.as<JsonObject>();
    String tmp ;
    
    const char* temp = data["delay"];
    Serial.println(temp);
    trigger_delay = atoi(temp);
    Serial.println("\nnew time : " + trigger_delay);
  }
  
  String response;
  serializeJson(data, response);
  request->send(200, "application/json", response);
  Serial.println(response); });

void restartTrigger()
{
  Serial.flush();

  if (!exam_mode)
  {
    Serial.println("\nRestarting trigger loop");
    check_trigger.start();
  }
  else
  {
    Serial.println("\nRestarting exam trigger loop");

    check_exam.start();
  }
}

void changeState()
{
  digitalWrite(LED_PIN, !(digitalRead(LED_PIN))); // Invert Current State of LED
}
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}
boolean status = false;
void trigger()
{
  Serial.flush();

  status = !status;
  // String serverName = serverName + String(rtc.now().unixtime()) + ".json";
  const String payload = String(rtc.now().unixtime()) + "," + status; //"{\"time\":\"" + String(rtc.now().unixtime()) + "\"," + "\"state\":\"" + test + "\"}";
  /*
    http.begin(client, serverName);


    // If you need Node-RED/server authentication, insert user and password below
    // http.setAuthorization("REPLACE_WITH_SERVER_USERNAME", "REPLACE_WITH_SERVER_PASSWORD");
    //  http.addHeader("Content-Type", "application/json");
    Serial.println(payload);

    int httpResponseCode = http.POST(payload);
    delay(200);
    Serial.println(httpResponseCode);
    http.end(); */
  digitalWrite(LED_PIN, !(digitalRead(LED_PIN)));
  File file = LittleFS.open("/logs.json", "a"); // Create new timings file
  file.println(payload);
  file.close();
  Serial.println(WiFi.localIP());
  restart_trigger.start();
}

void checkTrigger()
{
  compareTimings(timers);
}
void compareTimings(String a[])
{
  Serial.flush();

  int time_hour_int = rtc.now().hour();
  current_time = "";
  current_time = rtc.now().hour();

  if (time_hour_int > 12)
  {
    switch (time_hour_int)
    {
    case 13:
      current_time = 1;
      break;
    case 14:
      current_time = 2;
      break;
    case 15:
      current_time = 3;
      break;
    case 16:
      current_time = 4;
      break;
    case 17:
      current_time = 5;
      break;
    case 18:
      current_time = 6;
      break;
    case 19:
      current_time = 7;
      break;
    case 20:
      current_time = 8;
      break;
    case 21:
      current_time = 9;
      break;
    case 22:
      current_time = 10;
      break;
    case 23:
      current_time = 11;
    case 0:
      current_time = 12;
      break;
    default:
      break;
    }
  }

  current_time += ":";
  current_time += rtc.now().minute();
  Serial.println(rtc.now().unixtime());
  Serial.println(current_time);
  Serial.println(current_time);
  uint len = exam_mode ? 1 : 8;
  for (uint i = 0; i <= len; i++)
  {
    // Serial.println("timer " + String(i));
    if (a[i] == current_time)
    {
      class_alarm.start();
      Serial.flush();

      Serial.printf("\nAlarm triggered for 10 seconds for alarm number : %d", i);
      Serial.printf("\nAlarm triggered for 10 seconds for alarm number : %d", i);

      if (exam_mode)
      {
        check_exam.pause();
        if (i > 0)
        {
          exam_mode = false;
          check_exam.stop();

          check_trigger.start();
        }
      }
      else
      {
        check_trigger.pause();
      }
      break;
    }
  }
}
void setup()
{

  Serial.begin(115200);

  // LED PIN
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0x1);
  delay(2000);
  digitalWrite(LED_PIN, 0x0);
  Serial.print("Starting the LittleFS Webserver");
  // Start WIFI as Access Point
  WiFi.softAP(ssid, password);
  /*   if (WiFi.softAP(ssid, password))
    {

      while (1)
      {
        Serial.begin(115200);
        Serial.println("Couldn't start WiFi");
        changeState();
        delay(1000);
      };
    }
   */
  // Begin LittleFS
  if (!LittleFS.begin())
  {
    Serial.println("An Error has occurred while mounting LittleFS");
  }
  File f = LittleFS.open("/timings.json", "r");
  String s = f.readStringUntil('}');
  f.close();

  StaticJsonDocument<420> data;

  deserializeJson(data, s);
  String tmp = data["1"];

  // Serial.println("data 0 :" + tmp);
  for (uint8_t i = 0; i <= 8; i++)
  {

    const char *temp = data[String(i).c_str()];
    Serial.println(temp);
    timers[i] = temp;

    /*   if(String(rtcNow.hour()) == String(temp[0]+ temp[1])){

          next_timer = i++;
          } */
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request)
            {   ESP.restart();
               request->send(200); });
  server.on("/logo.png", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/logo.png", "image/png"); });

  AsyncCallbackJsonWebHandler *handler = new AsyncCallbackJsonWebHandler("/timings", [](AsyncWebServerRequest *request, JsonVariant &json)
                                                                         {
  Serial.print("new timings..  \n parsing..");
  StaticJsonDocument<420> data;
  if (json.is<JsonArray>())
  {
    data = json.as<JsonArray>();
  }
  else if (json.is<JsonObject>())
  {
    data = json.as<JsonObject>();
    String tmp = data["1"];
    
  //  int j =0;
    for(size_t i = 0;i<= 8;i++){
/*     if(j == 4){
      j++;
    } */
    const char* temp = data[String(i).c_str()];
    Serial.println(temp);
    timers[i] = temp;
  //  j++;          
     }
  }
  
  String response;
  serializeJson(data, response);
  LittleFS.begin();
  LittleFS.remove("/timings.json"); //Delete the old alarm timings
  File file = LittleFS.open("/timings.json","w");   //Create new timings file
  file.print(response);
  file.close();
  request->send(200, "application/json", response);
  Serial.println(response); });
  server.addHandler(timeHandler);
  server.addHandler(handler);
  server.addHandler(examHandler);
  server.addHandler(delayHandler);
  server.onNotFound(notFound);
  server.begin();

  if (!rtc.begin())
  {
    {

      while (1)
      {
        Serial.println("Couldn't find RTC");
        changeState();
        delay(1000);
      };
    }
  }

  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", String(rtc.now().hour()) + " : " + String(rtc.now().minute())); });
  //  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // set rtc time
  Serial.flush();

  Serial.println(rtc.now().unixtime());
  Serial.println("server started at ");

  Serial.println("SSID : " + WiFi.BSSIDstr());

  check_trigger.start();
  File file = LittleFS.open("/logs.json", "r"); // Create new timings file
  Serial.println(file.readString());
  file.close();
}

void loop()
{
  check_trigger.update();
  class_alarm.update();
  restart_trigger.update();
  check_exam.update();
  // yield();
}
