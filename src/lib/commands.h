#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include <stdint.h>

void mqttCallbackHandler(char *topic, uint8_t *msg, unsigned int msgLen);

#endif
