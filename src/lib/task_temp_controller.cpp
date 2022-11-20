#include "task_temp_controller.h"

#include <Arduino.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>
#include "common.h"
#include "config.h"

extern DallasTemperature sensors;
extern DeviceAddress sensor1;
extern DeviceAddress sensor2;
extern Config_t config;
extern Data_t data;

void taskTempController(void *param) {
    unsigned long readStart;
    for (;;) {
        readStart = millis();

        sensors.requestTemperatures();
        data.temp1 = sensors.getTempC(sensor1);
        data.temp2 = sensors.getTempC(sensor2);
        data.hasReading = true;

        if (data.temp1 > config.setpoint + config.deviance && data.isHeating) {
            data.isHeating = false;
            digitalWrite(PIN_RELAY, !data.isHeating);
        } else if (data.temp1 < config.setpoint - config.deviance && !data.isHeating) {
            data.isHeating = true;
            digitalWrite(PIN_RELAY, !data.isHeating);
        }

        Serial.printf("Temp1: %.3f, Temp2: %.3f, Heating: %s\n", data.temp1, data.temp2, data.isHeating ? "ON" : "OFF");

        vTaskDelay((config.controlIntervalMs - (millis() - readStart)) / portTICK_PERIOD_MS);
    }
}
