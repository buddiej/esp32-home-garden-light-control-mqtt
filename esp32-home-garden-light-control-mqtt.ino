#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <stdlib.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "_authentification.h"  /* credentials for WIFI and mqtt. Located in libraries folder */


/* user include */
#include "led.h"

/*****************************************************************************************/
/*                                    GENERAL DEFINE                                     */
/*****************************************************************************************/
#define TRUE  1
#define FALSE 0

#define STATE_OFF           0
#define STATE_ON            1

/*****************************************************************************************/
/*                                    PROJECT DEFINE                                     */
/*****************************************************************************************/


#define MQTT_PAYLOAD_MAX 250

/* Receive topics */
#define TOPIC_WGARDEN_LIGHT01_PERCENT   "wgarden/lights/1/set/brightness"
#define TOPIC_WGARDEN_LIGHT02_PERCENT   "wgarden/lights/2/set/brightness"
#define TOPIC_WGARDEN_LIGHT03_PERCENT   "wgarden/lights/3/set/brightness"
#define TOPIC_WGARDEN_LIGHT04_PERCENT   "wgarden/lights/4/set/brightness"
#define TOPIC_WGARDEN_LIGHT05_PERCENT   "wgarden/lights/5/set/brightness"
#define TOPIC_WGARDEN_LIGHT06_PERCENT   "wgarden/lights/6/set/brightness"
#define TOPIC_WGARDEN_LIGHT07_PERCENT   "wgarden/lights/7/set/brightness"
#define TOPIC_WGARDEN_LIGHT08_PERCENT   "wgarden/lights/8/set/brightness"
#define TOPIC_WGARDEN_LIGHT09_PERCENT   "wgarden/lights/9/set/brightness"
#define TOPIC_WGARDEN_LIGHT10_PERCENT   "wgarden/lights/10/set/brightness"
#define TOPIC_WGARDEN_LIGHT11_PERCENT   "wgarden/lights/11/set/brightness"
#define TOPIC_WGARDEN_LIGHT12_PERCENT   "wgarden/lights/12/set/brightness"
#define TOPIC_WGARDEN_LIGHT13_PERCENT   "wgarden/lights/13/set/brightness"
#define TOPIC_WGARDEN_LIGHT14_PERCENT   "wgarden/lights/14/set/brightness"
#define TOPIC_WGARDEN_LIGHT15_PERCENT   "wgarden/lights/15/set/brightness"
#define TOPIC_WGARDEN_LIGHT16_PERCENT   "wgarden/lights/16/set/brightness"
#define TOPIC_WGARDEN_LIGHT17_PERCENT   "wgarden/lights/17/set/brightness"
#define TOPIC_WGARDEN_LIGHT18_PERCENT   "wgarden/lights/18/set/brightness"

/* send topics*/
#define TOPIC_WGARDEN_GET_INFO_LIGHTS   "wgarden/lights/all/get/"



/*****************************************************************************************/
/*                                     TYPEDEF ENUM                                      */
/*****************************************************************************************/


/*****************************************************************************************/
/*                                   TYPEDEF STRUCT                                      */
/*****************************************************************************************/


/*****************************************************************************************/
/*                                         VARIABLES                                     */
/*****************************************************************************************/
/* create an instance of WiFiClientSecure */
WiFiClient espClient;
PubSubClient client(espClient);


int mqttRetryAttempt = 0;
int wifiRetryAttempt = 0;
             
long lastMsg = 0;
long loop_2sec_counter = 0;
long loop_2sec_counterOld = 0;


static void receivedCallback(char* topic, byte* payload, unsigned int length);
static void mqttconnect(void);

#define DEBUG_MQTT_RECEIVER   Serial.print("Message received: ");  \
                              Serial.print(topic); \
                              Serial.print("\t"); \
                              Serial.print("payload: "); \
                              Serial.println(PayloadString);

/*****************************************************************************************/
/*                                         VARIABLES  user                               */
/*****************************************************************************************/



/*****************************************************************************************/
/*                                     end VARIABLES  user                               */
/*****************************************************************************************/

/**************************************************************************************************
Function: ArduinoOta_Init()
Argument: void
return: void
**************************************************************************************************/
void ArduinoOta_Init(void)
{
  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("esp32-home-garden-light-control-mqtt");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}                              
/*************************************************************************************************/
/**************************************************************************************************
Function: setup()
return: void
**************************************************************************************************/
/*************************************************************************************************/
void setup()
{
  
  Serial.begin(115200);

  Serial.println(" ");
  Serial.println("########################################################");
  Serial.println("# Program esp32-home-garden-light-control-mqtt 0.4     #");
  Serial.println("########################################################");
  Serial.println(__FILE__);
  Serial.println(" ");
  Serial.println("Starting ...");
  Serial.println(" ");

  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
    wifiRetryAttempt++;
    if (wifiRetryAttempt > 5) 
    {
      Serial.println("Restarting!");
      ESP.restart();
    }
  }

  ArduinoOta_Init();
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("IP address of server: ");
  Serial.println(serverHostname);
  /* set SSL/TLS certificate */
  /* configure the MQTT server with IPaddress and port */
  client.setServer(serverHostname, 1883);
  /* this receivedCallback function will be invoked
    when client received subscribed topic */
  client.setCallback(receivedCallback);

  /******************/
  /* user init here */
  /******************/
  Led_Init();

  Serial.println("Setup finished ... ");
}

/*************************************************************************************************/
/**************************************************************************************************
Function: loop()
return: void
**************************************************************************************************/
/*************************************************************************************************/
void loop() 
{

  ArduinoOTA.handle();
  
  /* if client was disconnected then try to reconnect again */
  if (!client.connected()) 
  {
    mqttconnect();
  }
  /* this function will listen for incomming
  subscribed topic-process-invoke receivedCallback */
  client.loop();

  /* Update PWM LED Driver */
  Led_UpdateDriver();
 
  /* we increase counter every 5 secs we count until 5 secs reached to avoid blocking program if using delay()*/
  long now = millis();
  
  /* calling every 2 sec. */
  if (now - lastMsg > 2000)
  {
    /* store timer value */
    lastMsg = now;

    loop_2sec_counter++;
  }

  if(loop_2sec_counter != loop_2sec_counterOld)
  {
    if(loop_2sec_counter % 2 == 0)
    {
      /* call every 4 sec. */
      //Serial.println("1st call every 4 sec.");
     
      /********************************************************************************************/
      /************************      HANDLING OF Send MQTT TOPICS for led        ******************/ 
      /********************************************************************************************/
      /*250 Byte of RAM used for json Object */
      StaticJsonDocument<MQTT_PAYLOAD_MAX> doc;
      char Buffer[MQTT_PAYLOAD_MAX];
        

       /* Add values in the document */
       doc["light1"] = Led_GetValue(LED_0);
       doc["light2"] = Led_GetValue(LED_1);
       doc["light3"] = Led_GetValue(LED_2);
       doc["light4"] = Led_GetValue(LED_3);
       doc["light5"] = Led_GetValue(LED_4);
       doc["light6"] = Led_GetValue(LED_5);
       doc["light7"] = Led_GetValue(LED_6);
       doc["light8"] = Led_GetValue(LED_7);
       doc["light9"] = Led_GetValue(LED_8);
       doc["light10"] = Led_GetValue(LED_9);
       doc["light11"] = Led_GetValue(LED_10);
       doc["light12"] = Led_GetValue(LED_11);
       doc["light13"] = Led_GetValue(LED_12);
       doc["light14"] = Led_GetValue(LED_13);
       doc["light15"] = Led_GetValue(LED_14);
       doc["light16"] = Led_GetValue(LED_15);
       doc["light17"] = Led_GetValue(LED_16);
       doc["light18"] = Led_GetValue(LED_17);
       /* serialize the content */
       serializeJson(doc, Buffer);
       /* publish the buffer content */
       client.publish(TOPIC_WGARDEN_GET_INFO_LIGHTS, Buffer, false);
     }
  }


  /* store counter value */
  loop_2sec_counterOld = loop_2sec_counter;
}

/**************************************************************************************************
Function: receivedCallback()
Argument: char* topic ; received topic
          byte* payload ; received payload
          unsigned int length ; received length
return: void
**************************************************************************************************/
void receivedCallback(char* topic, byte* payload, unsigned int length) 
{
  uint8_t Loc_Brightness;

 
  StaticJsonDocument<MQTT_PAYLOAD_MAX> doc;

  char PayloadString[length + 1 ];
  /* convert payload in string */
  for(byte i=0;i<length;i++)
  {
    PayloadString[i] = payload[i];
  }
  PayloadString[length] = '\0';
  
  /* Debug */
  DEBUG_MQTT_RECEIVER


  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, PayloadString);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  /********************************************************************************************/
  /********************      HANDLING OF Received MQTT TOPICS WITH JASON     ******************/ 
  /********************************************************************************************/
  
  /*+++++++++++++++++++++++++++++ Set control +++++++++++++++++++++++++++++++++++++++*/ 
 
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT01_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 1 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_0, Loc_Brightness); 
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT02_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 2 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_1, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT03_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 3 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_2, Loc_Brightness);   
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT04_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 4 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_3, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT05_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 5 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_4, Loc_Brightness);     
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT06_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 6 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_5, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT07_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 7 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_6, Loc_Brightness);     
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT08_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 8 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_7, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT09_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 9 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_8, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT10_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 10 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_9, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT11_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 11 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_10, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT12_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 12 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_11, Loc_Brightness);   
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT13_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 13 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_12, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT14_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 14 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_13, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT15_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 15 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_14, Loc_Brightness);   
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT16_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 16 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_15, Loc_Brightness);   
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT17_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 17 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_16, Loc_Brightness);    
    }
  }
  if(strcmp(topic, TOPIC_WGARDEN_LIGHT18_PERCENT)==0)
  {
    if(doc.containsKey("value")) 
    {
      Loc_Brightness = doc["value"];
      Serial.print("Light 18 brightness Set:");
      Serial.println(Loc_Brightness, DEC);
      Led_SetValue(LED_17, Loc_Brightness);   
    }
  }
}


/**************************************************************************************************
Function: mqttconnect()
Argument: void
return: void
**************************************************************************************************/
void mqttconnect(void)
{
  /* Loop until reconnected */
  while (!client.connected()) 
  {
    Serial.print("MQTT connecting ...");
    /* client ID */
    String clientId = "esp32-home-garden-light-control-mqtt";
    /* connect now */
    if (client.connect(clientId.c_str(), serverUsername.c_str(), serverPassword.c_str()))
    {
      Serial.println("connected");

      /*********************/
      /* subscribe topic's */
      /*********************/
      client.subscribe(TOPIC_WGARDEN_LIGHT01_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT02_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT03_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT04_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT05_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT06_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT07_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT08_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT09_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT10_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT11_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT12_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT13_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT14_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT15_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT16_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT17_PERCENT);
      client.subscribe(TOPIC_WGARDEN_LIGHT18_PERCENT);
    
    } 
    else 
    {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      /* Wait 5 seconds before retrying */
      delay(5000);
      mqttRetryAttempt++;
      if (mqttRetryAttempt > 5) 
      {
        Serial.println("Restarting!");
        ESP.restart();
      }
    }
  }
}
