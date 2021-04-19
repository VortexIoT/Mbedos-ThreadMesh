#include "mbed.h"
#include "time.h"


//static struct tm time;
static char time_buffer[sizeof(struct tm)];
uint32_t time_in_seconds(char *time_buffer);

 