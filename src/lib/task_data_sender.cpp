#include "task_data_sender.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "topics.h"

extern PubSubClient mqtt;
extern Config_t config;
extern Data_t data;
extern Topics_t topics;

void taskDataSender(void *params) {
    char bufSensor1Payload[10];
    char bufSensor2Payload[10];
    char bufData[50];

    for (;;) {
        if (WiFi.status() == WL_CONNECTED && mqtt.connected() && data.hasReading) {

            sprintf(bufSensor1Payload, "%.3f", data.temp1);
            sprintf(bufSensor2Payload, "%.3f", data.temp2);
            sprintf(bufData, "{\"sensor1\":%.3f,\"sensor2\":%.3f,\"heater\":%d}", data.temp1, data.temp2, data.isHeating ? 1 : 0);

            mqtt.publish(topics.pubSensor1, bufSensor1Payload);
            mqtt.publish(topics.pubSensor2, bufSensor2Payload);
            mqtt.publish(topics.pubHeater, data.isHeating ? "1" : "0");

            mqtt.publish(topics.pubData, bufData);
        }
        vTaskDelay(config.publishIntervalMs / portTICK_PERIOD_MS);
    }
}
