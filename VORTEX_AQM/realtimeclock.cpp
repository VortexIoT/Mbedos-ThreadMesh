#include "realtimeclock.h"
#include "cli_cmd.h"
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include "string.h"
#include "mbed_mktime.h"
#include "time.h"
rtc_leap_year_support_t leap_year_type = RTC_FULL_LEAP_YEAR_SUPPORT;
char month_table[13][4] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

int rtcmonth(char *input_month) {
    int i=0;
    int month;
    for( i = 0; i < 12; i++) {
    //    printf("%s : %s ",month_table[i], input_month);
        if (!strncmp((char *)month_table[i], input_month,3)) {
            month = i;
            i = 12;
        }
    }
  //  printf("month %d\n",month);
    return month;
}

uint32_t time_in_seconds(char *time_buffer) {//1 4 12 2021 12:50:02
    char *remaining_data_after_tok; 
    char checkbyte = time_buffer[0];
    char *test;
    time_t t;
    // = 34936;
    struct tm timeinfo; 
    if(isalpha(checkbyte))
    {
     //   test = strtok_r(time_buffer, " ", &remaining_data_after_tok);
    //    printf("month %s: \n",test);
       timeinfo.tm_mon =  rtcmonth(strtok_r(time_buffer, " ", &remaining_data_after_tok));
       timeinfo.tm_mday = atoi(strtok_r(remaining_data_after_tok, " ", &time_buffer));
    }
    else {
        timeinfo.tm_mday = atoi(strtok_r(time_buffer, " ", &remaining_data_after_tok));   //after this 2021 12:50:02
        timeinfo.tm_mon = (atoi(strtok_r(remaining_data_after_tok, " ", &time_buffer)) - 1);  //after this 12 2021 12:50:02
    }
   // printf("hi\n");
 //   timeinfo.tm_wday = atol(strtok_r(time_buffer, " ", &remaining_data_after_tok));  //after this 4 12 2021 12:50:02
    timeinfo.tm_year = (atoi(strtok_r(time_buffer, " ", &remaining_data_after_tok)) - 1900);   //after this tmebuffer with 12:50:02
    timeinfo.tm_hour = atoi(strtok_r(remaining_data_after_tok, ":", &time_buffer));   //12
    timeinfo.tm_min = atoi(strtok_r(time_buffer, ":", &remaining_data_after_tok));    //50
    timeinfo.tm_sec = atoi(strtok_r(remaining_data_after_tok, ":", &time_buffer));    //02
  //  timeinfo.tm_isdst = -1;
    _rtc_maketime(&timeinfo, &t, leap_year_type);
  //  bool res =  _rtc_maketime(time, seconds, leap_year_type);
 //   printf("in sec %d  %d %d %d %d %d %u\n", timeinfo.tm_mday, timeinfo.tm_mon,timeinfo.tm_year, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, t);
    return t;
}
