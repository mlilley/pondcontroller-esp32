#include "commands.h"

#include <stdlib.h>
#include <string.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "config.h"
#include "topics.h"

#define MAX_MSG_LEN 128
#define MAX_MSG_ID_LEN 20
#define MAX_MSG_TYPE_LEN 30
#define MAX_DEVICE_ID_LEN 20

const float MAX_SETPOINT = 30.0f;
const float MIN_SETPOINT = 10.0f;
const float MAX_DEVIANCE = 5.0f;
const float MIN_DEVIANCE = 0.25f;
const unsigned long MAX_CONTROL_MS = 3600000L; // 1 hour
const unsigned long MIN_CONTROL_MS = 100L;     // 0.1 secs
const unsigned long MAX_PUBLISH_MS = 3600000L; // 1 hour
const unsigned long MIN_PUBLISH_MS = 1000L;    // 1 sec

typedef struct Cmd {
    char *id;
    char *type;
    char *value;
} Cmd_t;

extern Config_t config;
extern Topics_t topics;
extern PubSubClient mqtt;

char bufPayload[MAX_MSG_LEN+1];

// void HomeAssistantDiscovery() {
//     const char discoveryPayload = ""
//         "{"
//           "\"name\":\"" + config.deviceId + "\","
//           "\"unit_of_measurement\":\"C\","
//           "\"icon\":\"mdi:temperature\","
//           "\"state_topic\":\"mlhome/sensor/" + config.deviceId + "/sensor1\","
//           "\"device\":{"
//             "\name\":\"" + config.deviceId + "\","
//             "\"identifiers\":[\"" + config.deviceId + "\"]"
//           "}"
//         "}";

//     if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {
//         mqtt.publish("homeassistant/sensor/")
//     }
// }

bool parseCmd(char *buf, Cmd_t *cmd) {
    cmd->id = NULL;
    cmd->type = NULL;
    cmd->value = NULL;

    char *token = strtok(buf, ":");
    if (token == NULL) return false;
    if (strlen(token) > MAX_MSG_ID_LEN) return false;
    cmd->id = token;

    token = strtok(NULL, ":");
    if (token == NULL) return false;
    if (strlen(token) > MAX_MSG_TYPE_LEN) return false;
    cmd->type = token;
    
    token = strtok(NULL, ":");
    cmd->value = token;
    return true;
}

bool respondToCmd(Cmd_t *cmd, bool isSuccess) {
    char buf[50];
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {
        sprintf(buf, "%s:%s", cmd->id, isSuccess ? "OK" : "ERROR");
        mqtt.publish(topics.pubCmdResponse, buf);
        return true;
    }
    return false;
}

bool respondToGetSettings(Cmd_t *cmd) {
    // id(9) + ":" + "SP:" + "00.0:" + "DV:" + "00:0:" + "CI:" + "000000000:" + "PI:" + "000000000"
    //   9   +  1  +   3   +    5    +   3   +    5    +   3   +      8       +   3   +     8       = 48
    char buf[50];
    if (WiFi.status() == WL_CONNECTED && mqtt.connected()) {
        Serial.printf("About to send: id='%s:SP'\n", cmd->id);

        sprintf(buf, "%s:SP:%.1f:DV:%.1f:CI:%u:PI:%u", cmd->id, config.setpoint, config.deviance, config.controlIntervalMs, config.publishIntervalMs);
        mqtt.publish(topics.pubCmdResponse, buf);
        return true;
    }
    return false;
}

void handleCommandSetSetpoint(Cmd_t *cmd) {
    if (cmd->value != NULL) {
        float value = atof(cmd->value);
        if (value >= MIN_SETPOINT && value <= MAX_SETPOINT) {
            config.setpoint = value;
            writeConfig();
            respondToCmd(cmd, true);
            return;
        }
    }
    respondToCmd(cmd, false);   
}

void handleCommandSetDeviance(Cmd_t *cmd) {
    if (cmd->value != NULL) {
        float value = atof(cmd->value);
        if (value >= MIN_DEVIANCE && value <= MAX_DEVIANCE) {
            config.deviance = value;
            writeConfig();
            respondToCmd(cmd, true);
            return;
        }
    }
    respondToCmd(cmd, false);   
}

void handleCommandSetControlInterval(Cmd_t *cmd) {
    if (cmd->value != NULL) {
        unsigned long value = strtoul(cmd->value, NULL, 10);
        if (value >= MIN_CONTROL_MS && value <= MAX_CONTROL_MS) {
            config.controlIntervalMs = value;
            writeConfig();
            respondToCmd(cmd, true);
            return;
        }
    }
    respondToCmd(cmd, false);   
}

void handleCommandSetPublishInterval(Cmd_t *cmd) {
    if (cmd->value != NULL) {
        unsigned long value = strtoul(cmd->value, NULL, 10);
        if (value >= MIN_PUBLISH_MS && value <= MAX_PUBLISH_MS) {
            config.publishIntervalMs = value;
            writeConfig();
            respondToCmd(cmd, true);
            return;
        }
    }
    respondToCmd(cmd, false);
}

void handleCommandGetSettings(Cmd_t *cmd) {
    if (cmd->value == NULL) {
        respondToGetSettings(cmd);
        return;
    }
    respondToCmd(cmd, false);
}

void mqttCallbackHandler(char *topic, uint8_t *msg, unsigned int msgLen) {
    Serial.printf("Received: %s\n", topic);

    if (msgLen > MAX_MSG_LEN) {
        return;
    }

    // we only listen to the command topic
    if (0 != strcmp(topic, topics.subCmd)) {
        return;
    }
    
    // parse the command payload
    memcpy(bufPayload, msg, msgLen);
    bufPayload[msgLen] = 0;
    Cmd_t cmd;
    if (!parseCmd(bufPayload, &cmd)) {
        return;
    }

    // determine specific type of command and handle it
    if (0 == strcmp(cmd.type, "SET_SETPOINT")) {
        handleCommandSetSetpoint(&cmd);
    } else if (0 == strcmp(cmd.type, "SET_DEVIANCE")) {
        handleCommandSetDeviance(&cmd);
    } else if (0 == strcmp(cmd.type, "SET_CONTROL_INTERVAL")) {
        handleCommandSetControlInterval(&cmd);
    } else if (0 == strcmp(cmd.type, "SET_PUBLISH_INTERVAL")) {
        handleCommandSetPublishInterval(&cmd);
    } else if (0 == strcmp(cmd.type, "GET_SETTINGS")) {
        handleCommandGetSettings(&cmd);
    } else {
        respondToCmd(&cmd, false);
    }
}
