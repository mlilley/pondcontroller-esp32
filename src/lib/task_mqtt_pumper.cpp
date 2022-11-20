#include "task_mqtt_pumper.h"

#include <Arduino.h>
#include <PubSubClient.h>

#define MQTT_PUMPER_INTERVAL_MS 500L

extern PubSubClient mqtt;

void taskMqttPumper(void *params) {
    for (;;) {
        mqtt.loop();
        vTaskDelay(500L / portTICK_PERIOD_MS);
    }
}
