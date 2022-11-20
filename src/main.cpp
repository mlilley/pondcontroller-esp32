#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#define SSD1306_NO_SPLASH true
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#define REQUIRESNEW false
#define REQUIRESALARMS false
#include <DallasTemperature.h>
#include <PubSubClient.h>

#include "lib/common.h"
#include "lib/config.h"
#include "lib/topics.h"
#include "lib/commands.h"
#include "lib/task_connection_monitor.h"
#include "lib/task_data_sender.h"
#include "lib/task_temp_controller.h"
#include "lib/task_mqtt_pumper.h"
#include "lib/task_display_updater.h"

#define MAINLOOP_INTERVAL_MS 1000L
#define DISPLAY_WAKE_MS 5000L

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire, -1);
OneWire oneWire(PIN_SENSORBUS);
WiFiClient wifiClient;

DallasTemperature sensors = DallasTemperature(&oneWire);
DeviceAddress sensor1, sensor2; 
PubSubClient mqtt(wifiClient);

Config_t config;
Data_t data;
Topics_t topics;

volatile bool displayWake = false;
volatile unsigned long displayWakeAt = 0;
static TaskHandle_t taskDisplayWaker_h = NULL;

void IRAM_ATTR buttonISR() {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    vTaskNotifyGiveFromISR(taskDisplayWaker_h, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void taskDisplayWaker(void *param) {
    for (;;) {
        uint32_t ulNotificationValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        if (ulNotificationValue == 1) {
            display.dim(false);
            displayWakeAt = millis();
            displayWake = true;
        } else {
            Serial.println("taskDisplayWaker notificationTake timeout");
        }
    }
}

void taskDisplaySleeper(void *param) {
    for (;;) {
        if (displayWake && (millis() - displayWakeAt) >= DISPLAY_WAKE_MS) {
            display.dim(true);
            displayWake = false;
            displayWakeAt = 0;
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void setup() {
    // Setup GPIO for relay, and set it initially to OFF
    pinMode(PIN_RELAY, OUTPUT);
    digitalWrite(PIN_RELAY, HIGH);

    // Setup GPIO for button, as input with pullup
    pinMode(PIN_BUTTON, INPUT_PULLUP);

    // Initialize serial
    Serial.begin(115200);

    // load config from rom
    loadConfig();
    Serial.printf("Config:\n  deviceId: %s\n  wifiSsid: %s\n  wifiPass: %s\n  mqttHost: %s\n  mqttPort: %u\n  setpoint: %0.2f\n  deviance: %0.2f\n  publishMs: %u\n  controlMs: %u\n",
        config.deviceId, config.wifiSsid, config.wifiPass, config.mqttHost, config.mqttPort, config.setpoint, config.deviance, config.publishIntervalMs, config.controlIntervalMs);

    // Initialize display
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C, true, true)) Serial.print("Warn: display init failed");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setTextWrap(false);
    display.cp437(true);
    display.dim(true);
    display.display();

    // Initialize i2c temp sensor
    sensors.begin();
    if (sensors.getDeviceCount() != 2) throw "Sensors missing";
    sensors.getAddress(sensor1, 0);
    sensors.getAddress(sensor2, 1);
    sensors.setResolution(sensor1, SENSOR_RESOLUTION); 
    sensors.setResolution(sensor2, SENSOR_RESOLUTION);

    // Initialize mqtt client
    mqtt.setServer(config.mqttHost, config.mqttPort);
    mqtt.setCallback(mqttCallbackHandler);
    updateTopics(config.deviceId);

    // Initialize tasks
    xTaskCreatePinnedToCore(taskConnectionMonitor, "ConnectionMonitor", 5000, NULL, 1, NULL, CONFIG_ARDUINO_RUNNING_CORE); // must run on same core as arduino
    xTaskCreate(taskDataSender, "DataSender", 5000, NULL, 1, NULL);
    xTaskCreate(taskTempController, "TempController", 5000, NULL, 1, NULL);
    xTaskCreate(taskMqttPumper, "MqttPumper", 5000, NULL, 1, NULL);
    xTaskCreate(taskDisplayUpdater, "DisplayUpdater", 5000, NULL, 1, NULL);
    xTaskCreate(taskDisplayWaker, "DisplayWaker", 5000, NULL, 1, &taskDisplayWaker_h);
    xTaskCreate(taskDisplaySleeper, "DisplaySleeper", 5000, NULL, 1, NULL);
    
    // button ISR
    attachInterrupt(PIN_BUTTON, buttonISR, FALLING);
}

void loop() {
    vTaskDelay(MAINLOOP_INTERVAL_MS / portTICK_PERIOD_MS);
}
