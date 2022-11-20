#include "config.h"
#include <Preferences.h>

#define CONFIG_PREFS_NAMESPACE "pondcontroller"

void loadConfig() {
    Preferences prefs;

    prefs.begin(CONFIG_PREFS_NAMESPACE, true);
    
    prefs.getString("DEVICE_ID", config.deviceId, MAXLEN_CONFIG_DEVICE_ID);
    prefs.getString("WIFI_SSID", config.wifiSsid, MAXLEN_CONFIG_WIFI_SSID);
    prefs.getString("WIFI_PASS", config.wifiPass, MAXLEN_CONFIG_WIFI_PASS);
    prefs.getString("MQTT_HOST", config.mqttHost, MAXLEN_CONFIG_MQTT_HOST);
    config.mqttPort          = prefs.getUInt("MQTT_PORT", 1883);
    config.setpoint          = prefs.getFloat("TEMP_SETPOINT", 20.0f);
    config.deviance          = prefs.getFloat("TEMP_DEVIANCE", 1.0f);
    config.publishIntervalMs = prefs.getULong("PUBLISH_MS", 30000L);
    config.controlIntervalMs = prefs.getULong("CONTROL_MS", 5000L);

    prefs.end();
}

void writeConfig() {
    Preferences prefs;

    prefs.begin(CONFIG_PREFS_NAMESPACE, false);

    prefs.putString("DEVICE_ID", config.deviceId);
    prefs.putString("WIFI_SSID", config.wifiSsid);
    prefs.putString("WIFI_PASS", config.wifiPass);
    prefs.putString("MQTT_HOST", config.mqttHost);
    prefs.putUInt("MQTT_PORT", config.mqttPort);
    prefs.putFloat("TEMP_SETPOINT", config.setpoint);
    prefs.putFloat("TEMP_DEVIANCE", config.deviance);
    prefs.putULong("PUBLISH_MS", config.publishIntervalMs);
    prefs.putULong("CONTROL_MS", config.controlIntervalMs);
    
    prefs.end();
}
