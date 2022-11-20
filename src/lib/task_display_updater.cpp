#include "task_display_updater.h"

#include <Arduino.h>
#include <Wifi.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>
#include "common.h"

#define DISPLAY_UPDATER_INTERVAL_MS 1000L

extern Adafruit_SSD1306 display;
extern PubSubClient mqtt;
extern Config_t config;
extern Data_t data;

#define OFF_R 80

void taskDisplayUpdater(void *params) {
    for (;;) {
        display.clearDisplay();
        display.cp437(true);

        display.drawFastVLine(75, 0, 64, SSD1306_WHITE);

        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);

        display.setCursor(OFF_R, 0);
        display.printf("WIFI: %s", WiFi.status() == WL_CONNECTED ? "OK" : "__");
        display.setCursor(OFF_R, 10);
        display.printf("MQTT: %s", mqtt.connected() ? "OK" : "__");
        display.setCursor(0, 0);
        display.printf("    AIR");
        display.setCursor(0, 40);
        display.printf("   WATER");
        display.setCursor(60, 10);
        display.printf("\xF8""C");
        display.setCursor(60, 50);
        display.printf("\xF8""C");

        display.setCursor(OFF_R, 26);
        display.printf("SP:%*.1f", 5, config.setpoint);
        display.setCursor(OFF_R, 36);
        display.printf(" \xF1:%*.1f", 5, config.deviance);

        display.setTextSize(2);

        display.setCursor(0, 10);
        display.printf("%*.2f", 5, data.temp1);
        display.setCursor(0, 50);
        display.printf("%*.2f", 5, data.temp2);

        if (data.isHeating) {
            display.setTextSize(1);
            display.fillRect(OFF_R-1, 53, 128-OFF_R+1, 12, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
            display.setCursor(OFF_R, 55);
            display.printf("\x10HEATER\x11");
        }

        display.display();

        vTaskDelay(DISPLAY_UPDATER_INTERVAL_MS / portTICK_PERIOD_MS);
    }
}
