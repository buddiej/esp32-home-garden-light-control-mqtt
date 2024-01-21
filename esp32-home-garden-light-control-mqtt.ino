#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <stdlib.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "_authentification.h"  /* credentials for WIFI and mqtt. Located in libraries folder */


/* user include */
#include <PCA9685_LED_DRIVER.h>


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
#define TOPIC_GET_GARDEN_LED_MESSAGE   "garden_led/get/led"
#define TOPIC_GET_GARDEN_CURRENT       "garden_current/get/current"

/* Send topics */
#define TOPIC_SET_GARDEN_LED            "garden_led/set/led"

#define CURRENT_PIN 17     // Digital pin connected to the current sensor 

#define PWM_PCA9685_MOD1_ADDRESS     0x40
#define PWM_PCA9685_MOD2_ADDRESS     0x41

#define FREQUENCY   1000     //min 24Hz, max 1524Hz



/*****************************************************************************************/
/*                                     TYPEDEF ENUM                                      */
/*****************************************************************************************/


/*****************************************************************************************/
/*                                   TYPEDEF STRUCT                                      */
/*****************************************************************************************/
typedef struct LED_STRUCT
{
  uint brighness;
  uint state;
}T_LED_STRUCT;

typedef struct GARDEN_LED
{
  char name[12];
  char version[4];
  T_LED_STRUCT garden_Led[18];
}T_GARDEN_LED;




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


/* Class Constructor */
PCA9685 PWMPCA9685_Module1 = (PWM_PCA9685_MOD1_ADDRESS);
PCA9685 PWMPCA9685_Module2 = (PWM_PCA9685_MOD2_ADDRESS);

T_GARDEN_LED garden_led;

uint8_t version[4] = {'0','.','1','\n'};
uint8_t name[12] = {'g','a','r','d','e','n','_','L','E','D','s','\n'};



/* Variables */ 
uint16_t garden_pwmValModule1[16];
uint16_t garden_pwmValModule2[16];

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
  Serial.println("# Program esp32-home-garden-light-control-mqtt %s      #, version");
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

  /* Initialize the pwm servo devices. */
  PWMPCA9685_Module1.begin(FREQUENCY);
  PWMPCA9685_Module2.begin(FREQUENCY);

  Serial.println(F("Init of PWM 12 Bit modules..."));

  memset(&garden_pwmValModule1,0x00,16);
  memset(&garden_pwmValModule2,0x00,16);

  memcpy(&version, &garden_led.version,4);
  memcpy(&name, &garden_led.name,12);



 
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
      /************************      HANDLING OF Send MQTT TOPICS for garden led ******************/ 
      /********************************************************************************************/
      /*200 Byte of RAM used for json Object */
      StaticJsonDocument<MQTT_PAYLOAD_MAX> doc;
      char Buffer[MQTT_PAYLOAD_MAX];
        

       /* Add values in the document */
       doc["name"] = garden_led.name;
       doc["version"] = garden_led.version;

       /* serialize the content */
       serializeJson(doc, Buffer);
       /* publish the buffer content */
       client.publish(TOPIC_GET_GARDEN_LED_MESSAGE, Buffer, false);
     }
     else
     {
       /* call every 4 sec. */
       //Serial.println("2nd call every 4 sec.");
    



      /********************************************************************************************/
      /************************      HANDLING OF Send MQTT TOPICS for INA 219  ********************/ 
      /********************************************************************************************/
      /*200 Byte of RAM used for json Object */
      StaticJsonDocument<MQTT_PAYLOAD_MAX> doc;
      char Buffer[MQTT_PAYLOAD_MAX];
        
      doc["current_mA"] = 0;
      
      
      /* serialize the content */
      serializeJson(doc, Buffer);
      /* publish the buffer content */
      client.publish(TOPIC_GET_GARDEN_CURRENT, Buffer, false);
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
  uint8_t Loc_state;

 
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

  long time = doc["time"];
  Serial.println(time);


  /********************************************************************************************/
  /********************      HANDLING OF Received MQTT TOPICS WITH JASON     ******************/ 
  /********************************************************************************************/
  
  /*+++++++++++++++++++++++++++++ Set control +++++++++++++++++++++++++++++++++++++++*/ 
 
  if(strcmp(topic, TOPIC_SET_GARDEN_LED)==0)
  {
    if(doc.containsKey("state")) 
    {
      Loc_state = doc["state"];
      Serial.print("state garden LED received set: ");
      Serial.println(Loc_state, DEC);

      
      for(int i=0;i<16;i++)
      {
        PWMPCA9685_Module1.setPWM(i, i*i*16);   //Set PWM for output 0 to 1000_ON to 3095_OFF. 4095/10Bit.
        //gpio.setPWM(pwmValues, sizeof(pwmValues));    //Use an uint16_t array for updating multiple PWM values with one function
        PWMPCA9685_Module1.update();  //Send the PWM value from the RAM to the PCA9685 over I2C.

        PWMPCA9685_Module2.setPWM(i, i*i*16);   //Set PWM for output 0 to 1000_ON to 3095_OFF. 4095/10Bit.
        //gpio.setPWM(pwmValues, sizeof(pwmValues));    //Use an uint16_t array for updating multiple PWM values with one function
        PWMPCA9685_Module2.update();  //Send the PWM value from the RAM to the PCA9685 over I2C.
      }

           
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
      client.subscribe(TOPIC_SET_GARDEN_LED);
    
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
