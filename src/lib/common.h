#ifndef __COMMON_H__
#define __COMMON_H__

#define PIN_SENSORBUS 5
#define PIN_BUTTON 4
#define PIN_RELAY 6
#define SENSOR_RESOLUTION 12

#define MAXLEN_CONFIG_DEVICE_ID 50
#define MAXLEN_CONFIG_WIFI_SSID 64
#define MAXLEN_CONFIG_WIFI_PASS 50
#define MAXLEN_CONFIG_MQTT_HOST 128

typedef struct {
    char deviceId[MAXLEN_CONFIG_DEVICE_ID+1];
    char wifiSsid[MAXLEN_CONFIG_WIFI_SSID+1];
    char wifiPass[MAXLEN_CONFIG_WIFI_PASS+1];
    char mqttHost[MAXLEN_CONFIG_MQTT_HOST+1];
    unsigned int mqttPort;
    float setpoint;
    float deviance;
    unsigned long publishIntervalMs;
    unsigned long controlIntervalMs;
} Config_t;

typedef struct {
    float temp1;
    float temp2;
    bool isHeating;
    bool hasReading;
} Data_t;

typedef struct {
    char *subCmd;
    char *pubCmdResponse;
    char *pubAnnounce;
    char *pubSensor1;
    char *pubSensor2;
    char *pubHeater;
    char *pubData;
    char *msgHADiscovery;
} Topics_t;

#endif