#include "task_connection_monitor.h"

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "common.h"

#define WIFI_CONNECT_POLL_MS       500L
#define WIFI_CONNECT_TIMEOUT_MS  20000L
#define WIFI_CONNECT_BACKOFF_MS  10000L
#define WIFI_MONITOR_INTERVAL_MS 10000L
#define MQTT_MONITOR_INTERVAL_MS 10000L
#define MQTT_CONNECT_BACKOFF_MS   5000L

extern PubSubClient mqtt;
extern Topics_t topics;

void taskConnectionMonitor(void *params) {
    unsigned long wifiStart;
    for (;;) {
        // Ensure wifi is connected
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("Wifi connecting");

            wifiStart = millis();
            WiFi.mode(WIFI_STA);
            WiFi.setHostname(config.deviceId);
            WiFi.begin(config.wifiSsid, config.wifiPass);        
            while (WiFi.status() != WL_CONNECTED && millis() - wifiStart < WIFI_CONNECT_TIMEOUT_MS) {
                vTaskDelay(WIFI_CONNECT_POLL_MS / portTICK_PERIOD_MS);
            }
            
            if (WiFi.status() != WL_CONNECTED) {
                Serial.println("Wifi failed to connect (timeout)");
                WiFi.disconnect();
                if (WiFi.status() != WL_DISCONNECTED) {
                    Serial.println("WARN: wifi status not WL_DISCONNECTED after disconnect()");
                }
                vTaskDelay(WIFI_CONNECT_BACKOFF_MS / portTICK_PERIOD_MS);
                continue;
            }

            Serial.print("Wifi connected: ");
            Serial.println(WiFi.localIP());
        }

        // Ensure Mqtt is connected
        if (!mqtt.connected()) {
            Serial.println("Mqtt connecting");
            if (!mqtt.connect("pondcontroller")) {
                Serial.print("Mqtt failed to connect (");
                Serial.print(mqtt.state());
                Serial.println(")");
                vTaskDelay(MQTT_CONNECT_BACKOFF_MS / portTICK_PERIOD_MS);
                continue;
            }

            Serial.println("Mqtt connected");

            // TODO: if we allow device_id to vary at runtime, then we have to implement updating subscriptions
            mqtt.subscribe(topics.subCmd);
            mqtt.publish(topics.pubAnnounce, "pondcontroller1 says hello");
        }

        vTaskDelay(WIFI_MONITOR_INTERVAL_MS / portTICK_PERIOD_MS);
    }
}
