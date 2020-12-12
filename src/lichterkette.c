#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "MQTTClient.h"

#include "ir-slinger/irslinger.h" 

#if !defined MQTT_ADDRESS
#define MQTT_ADDRESS              "t20:1883"      // Address of the mqtt broker
#endif

#if !defined MQTT_TOPIC_ROOT
#define MQTT_TOPIC_ROOT           "voltronic/"            // i.e. the name of the ac
#endif

#if !defined FAN_NAME  
#define FAN_NAME                  "Voltronic Lichterkette"            // i.e. the name of the ac
#endif

// IR Pin on Raspi
#define OUT_PIN                   24

// MQTT Connection Configuration
#define CLIENTID                  "<<clientId>>" // FIXME
#define CLIENT_USERNAME           "<<tenant_ID>>/<<username>>"
#define CLIENT_PASSWORD           "<<password>>"

// Define topics
#define MQTT_TOPIC_TOGGLE_POWER         MQTT_TOPIC_ROOT "fan/toggle/power"
#define MQTT_TOPIC_TOGGLE_WAVE          MQTT_TOPIC_ROOT "fan/toggle/wave"
#define MQTT_TOPIC_TOGGLE_FAN           MQTT_TOPIC_ROOT "fan/toggle/fan"
#define MQTT_TOPIC_TOGGLE_TURN          MQTT_TOPIC_ROOT "fan/toggle/turn"
#define MQTT_TOPIC_TOGGLE_TIMER         MQTT_TOPIC_ROOT "fan/toggle/timer"

#define MQTT_TOPIC_POWER                MQTT_TOPIC_ROOT "fan/power"
#define MQTT_TOPIC_TURN                 MQTT_TOPIC_ROOT "fan/turn"
#define MQTT_TOPIC_WAVE                 MQTT_TOPIC_ROOT "fan/wave"
#define MQTT_TOPIC_FAN                  MQTT_TOPIC_ROOT "fan/fan"
#define MQTT_TOPIC_TIMER                MQTT_TOPIC_ROOT "fan/timer"


#define UPDATE_INTERVAL               3       // Update MQTT every N seconds

// Define modes
#define WAVE_WOOD                     2
#define WAVE_MOON                     3
#define WAVE_NONE                     1
#define WAVE_WOOD_NAME                "Wood"
#define WAVE_MOON_NAME                "Moon"
#define WAVE_NONE_NAME                "None"

#define FAN_LOW                       1
#define FAN_MID                       2 
#define FAN_HIGH                      3
#define FAN_LOW_NAME                  "Low"
#define FAN_MID_NAME                  "Mid"
#define FAN_HIGH_NAME                 "High"




typedef struct {
  bool on;
  bool turn;
  int timer;
  int wave;
  int fan;
} dl_aircon_msg_t;


dl_aircon_msg_t msg;


void publish(MQTTClient client, char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = 2;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, 1000L);
    printf("Message '%s' with delivery token %d delivered\n", payload, token);
}


int send_power(void)
{
	uint32_t outPin = OUT_PIN;            // The Broadcom pin number the signal will be sent on
	int frequency = 38000;          // The frequency of the IR signal in Hz
	double dutyCycle = 0.5;         // The duty cycle of the IR signal. 0.5 means for every cycle,
	                                // the LED will turn on for half the cycle time, and off the other half

	int codes[] = { 1258 , 383 , 1293 , 390 , 454 , 1226 , 1292 , 390 , 1293 , 396 , 460 , 1234 , 459 , 1238 , 456 , 1237 , 457 , 1236 , 461 , 1234 , 456 , 1238 , 1293 , 7173 , 1293 , 387 , 1288 , 391 , 457 , 1228 , 1294 , 389 , 1286 , 397 , 459 , 1236 , 459 , 1234 , 458 , 1238 , 456 , 1237 , 459 , 1235 , 460 , 1234 , 1293 , 7213 , 1294 , 384 , 1293 , 383 , 461 , 1227 , 1292 , 396 , 1283 , 400 , 458 , 1238 , 456 , 1235 , 458 , 1238 , 459 , 1236 , 458 , 1234 , 463 , 1229 , 1293 , 7189 , 1292 , 387 , 1288 , 392 , 457 , 1227 , 1292 , 388 , 1289 , 399 , 457 , 1236 , 457 , 1240 , 455 , 1243 , 453 , 1240 , 454 , 1242 , 451 , 1235 , 1295 , 7188 , 1291 , 389 , 1288 , 389 , 459 , 1227 , 1290 , 392 , 1286 , 399 , 458 , 1235 , 458 , 1243 , 452 , 1241 , 453 , 1242 , 451 , 1243 , 458 , 1230 , 1298 };

	int result = irSlingRaw(
		outPin,
		frequency,
		dutyCycle,
		codes,
		sizeof(codes) / sizeof(int));
	
	return result;
}


int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
  char* payload = message->payload;
  printf("Received operation %s\n", payload);

  unsigned long data;
  char *result;
 
  if (strcmp(topicName,MQTT_TOPIC_TOGGLE_POWER)==0)
  {
    bool state = true;
    if(strcmp(payload,"on")==0) 
    { state=true; }
    if(strcmp(payload,"off")==0) 
    { state=false; }
    if (state != msg.on) { msg.on = state; send_power(); }
  }

  if (strcmp(topicName,MQTT_TOPIC_TOGGLE_WAVE)==0)
  {
    int wave=0;
    if (strcmp(payload,WAVE_MOON_NAME)==0)
    { wave=WAVE_MOON; }
    if (strcmp(payload,WAVE_WOOD_NAME)==0)
    { wave=WAVE_WOOD; }
    if (strcmp(payload,WAVE_NONE_NAME)==0)
    { wave=WAVE_NONE; }
    if (wave != 0) 
    {
      int i;
      while (msg.wave != wave)
      {
        msg.wave++;
        if (msg.wave>3) {msg.wave=1;}
        send_wave();
      }
    }
  }

  if (strcmp(topicName,MQTT_TOPIC_TOGGLE_FAN)==0)
  {
    int state=0;
    if (strcmp(payload,FAN_HIGH_NAME) == 0)
    { state=FAN_HIGH; }
    if (strcmp(payload,FAN_MID_NAME) == 0)
    { state=FAN_MID; }
    if (strcmp(payload,FAN_LOW_NAME) == 0)
    { state=FAN_LOW; }
    if (state != 0)
    {
      while (state != msg.fan)
      {
        msg.fan++;
        if (msg.fan>3) {msg.fan=1;}
        send_fan();
      }
    }
  }

  if (strcmp(topicName,MQTT_TOPIC_TOGGLE_TURN)==0)
  {
    msg.turn = !msg.turn;
    send_turn();
  }

  if (strcmp(topicName,MQTT_TOPIC_TOGGLE_TIMER)==0)
  {
    int state=-1;
    if (strcmp(payload,TIMER_30_NAME) == 0)
    { state=TIMER_30; }
    if (strcmp(payload,TIMER_60_NAME) == 0)
    { state=TIMER_60; }
    if (strcmp(payload,TIMER_120_NAME) == 0)
    { state=TIMER_120; }
    if (strcmp(payload,TIMER_240_NAME) == 0)
    { state=TIMER_240; }    
    if (strcmp(payload,TIMER_NONE_NAME) == 0)
    { state=TIMER_NONE; }
    if (state != -1)
    {
      while (state != msg.timer)
      {
        msg.timer++;
        if (msg.timer>5) {msg.timer=1;}
        send_timer();
      }
    }
  }

  MQTTClient_freeMessage(&message);
  MQTTClient_free(topicName);

  return 1;
}


int main (void)
{
  MQTTClient client;
  MQTTClient_create(&client, MQTT_ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  conn_opts.username = CLIENT_USERNAME; 
  conn_opts.password = CLIENT_PASSWORD;

  MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);

  int rc;
  if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
      printf("Failed to connect, return code %d\n", rc);
      exit(-1);
  }

  //create device
  publish(client, "fan/name", FAN_NAME); 

  //listen for operation
  MQTTClient_subscribe(client, MQTT_TOPIC_TOGGLE_POWER, 0);
  MQTTClient_subscribe(client, MQTT_TOPIC_TOGGLE_WAVE, 0);
  MQTTClient_subscribe(client, MQTT_TOPIC_TOGGLE_FAN, 0);
  MQTTClient_subscribe(client, MQTT_TOPIC_TOGGLE_TURN, 0);
  MQTTClient_subscribe(client, MQTT_TOPIC_TOGGLE_TIMER, 0);

  //Default settings
  msg.on = false;
  msg.turn = false;
  msg.timer = TIMER_NONE;
  msg.wave=WAVE_NONE;
  msg.fan=FAN_LOW;

  char timer[8];
  
  for (;;) {
    //send temperature measurement
    if (msg.on == true) { publish(client, MQTT_TOPIC_POWER, "on"); } else { publish(client, MQTT_TOPIC_POWER, "off"); }
    if (msg.turn == true) { publish(client, MQTT_TOPIC_TIMER, "on");  } else { publish(client, MQTT_TOPIC_TIMER, "off");  }
    if (msg.wave == WAVE_NONE) { publish(client, MQTT_TOPIC_WAVE, WAVE_MOON_NAME); }
    else if (msg.wave == WAVE_WOOD) { publish(client, MQTT_TOPIC_WAVE, WAVE_WOOD_NAME); }
    else if (msg.wave == WAVE_MOON)       { publish(client, MQTT_TOPIC_WAVE, WAVE_NONE_NAME); }

    if (msg.fan == FAN_LOW)       { publish(client, MQTT_TOPIC_FAN, FAN_LOW_NAME); }
    else if (msg.fan == FAN_MID)  { publish(client, MQTT_TOPIC_FAN, FAN_MID_NAME);  }
    else if (msg.fan == FAN_HIGH) { publish(client, MQTT_TOPIC_FAN, FAN_HIGH_NAME); }

    sprintf(timer, "%d", msg.timer);
    publish(client, MQTT_TOPIC_TIMER, timer); 

    sleep(UPDATE_INTERVAL);
  }


  MQTTClient_disconnect(client, 1000);
  MQTTClient_destroy(&client);

  return rc;
}
