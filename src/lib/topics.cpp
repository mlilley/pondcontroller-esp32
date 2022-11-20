#include "topics.h"

#include "stdlib.h"
#include "string.h"
#include "stdio.h"

bool initialized = false;

void updateTopics(char *deviceId) {
    if (initialized) {
        free(topics.subCmd);
        free(topics.pubCmdResponse);
        free(topics.pubAnnounce);
        free(topics.pubSensor1);
        free(topics.pubSensor1);
        free(topics.pubHeater);
        free(topics.pubData);
    }

    size_t n = strlen(deviceId);

    topics.subCmd         = (char*)malloc((18+n+1)*sizeof(char)); // mlhome/sensor/<deviceId>/cmd       18 + n + 1
    topics.pubCmdResponse = (char*)malloc((23+n+1)*sizeof(char)); // mlhome/sensor/<deviceId>/response  23 + n + 1
    topics.pubAnnounce    = (char*)malloc((23+n+1)*sizeof(char)); // mlhome/sensor/<deviceId>/announce  23 + n + 1
    topics.pubSensor1     = (char*)malloc((22+n+1)*sizeof(char)); // mlhome/sensor/<deviceId>/sensor1   22 + n + 1
    topics.pubSensor2     = (char*)malloc((22+n+1)*sizeof(char)); // mlhome/sensor/<deviceId>/sensor2   22 + n + 1
    topics.pubHeater      = (char*)malloc((21+n+1)*sizeof(char)); // mlhome/sensor/<deviceId>/heater    21 + n + 1 
    topics.pubData        = (char*)malloc((19+n+1)*sizeof(char)); // mlhome/sensor/<deviceId>/data      19 + n + 1

    sprintf(topics.subCmd,         "mlhome/sensor/%s/cmd",      deviceId);
    sprintf(topics.pubCmdResponse, "mlhome/sensor/%s/response", deviceId);
    sprintf(topics.pubAnnounce,    "mlhome/sensor/%s/announce", deviceId);
    sprintf(topics.pubSensor1,     "mlhome/sensor/%s/sensor1",  deviceId);
    sprintf(topics.pubSensor2,     "mlhome/sensor/%s/sensor2",  deviceId);
    sprintf(topics.pubHeater,      "mlhome/sensor/%s/heater",   deviceId);
    sprintf(topics.pubData,        "mlhome/sensor/%s/data",     deviceId);
    
    initialized = true;
}
