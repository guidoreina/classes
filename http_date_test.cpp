#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include "net/http/date.h"

enum date_format {
  kRfc1123,
  kRfc850,
  kAnsiC
};

static void serialize(const struct tm& timestamp,
                      date_format format,
                      char* str);

int main()
{
  struct tm timestamp;
  timestamp.tm_year = 115;
  timestamp.tm_mon = 2;
  timestamp.tm_mday = 6;
  timestamp.tm_wday = 5;
  timestamp.tm_hour = 12;
  timestamp.tm_min = 34;
  timestamp.tm_sec = 56;
  timestamp.tm_isdst = 0;

  date_format formats[] = {
    kRfc1123,
    kRfc850,
    kAnsiC
  };

  for (size_t i = 0; i < 3; i++) {
    char str[128];
    serialize(timestamp, formats[i], str);
    printf("Timestamp: '%s'.\n", str);

    time_t t0;
    if ((t0 = net::http::date::parse(str, strlen(str))) ==
        static_cast<time_t>(-1)) {
      fprintf(stderr, "Couldn't parse date '%s'.\n", str);
      return -1;
    }

    time_t t1;
    if ((t1 = mktime(&timestamp)) != t0) {
      fprintf(stderr, "Timestamps are different (%ld <--> %ld).\n", t0, t1);
      return -1;
    }
  }

  return 0;
}

void serialize(const struct tm& timestamp, date_format format, char* str)
{
  static const char* months[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
  };

  static const char* days_short[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
  };

  static const char* days_long[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
  };

  switch (format) {
    case kRfc1123:
      sprintf(str,
              "%s, %02u %s %u %02u:%02u:%02u GMT",
              days_short[timestamp.tm_wday],
              timestamp.tm_mday,
              months[timestamp.tm_mon],
              1900 + timestamp.tm_year,
              timestamp.tm_hour,
              timestamp.tm_min,
              timestamp.tm_sec);

      break;
    case kRfc850:
      sprintf(str,
              "%s, %02u-%s-%02u %02u:%02u:%02u GMT",
              days_long[timestamp.tm_wday],
              timestamp.tm_mday,
              months[timestamp.tm_mon],
              timestamp.tm_year - 100,
              timestamp.tm_hour,
              timestamp.tm_min,
              timestamp.tm_sec);

      break;
    default:
      sprintf(str,
              "%s %s %2u %02u:%02u:%02u %u",
              days_short[timestamp.tm_wday],
              months[timestamp.tm_mon],
              timestamp.tm_mday,
              timestamp.tm_hour,
              timestamp.tm_min,
              timestamp.tm_sec,
              1900 + timestamp.tm_year);
  }
}
