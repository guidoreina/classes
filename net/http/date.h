#ifndef NET_HTTP_DATE_H
#define NET_HTTP_DATE_H

#include <stdint.h>
#include <time.h>
#include "macros/macros.h"

namespace net {
  namespace http {
    class date {
      public:
        static time_t parse(const void* buf, size_t len, struct tm& timestamp);
        static time_t parse(const void* buf, size_t len);

      private:
        static time_t parse_ansic(const uint8_t* begin,
                                  const uint8_t* end,
                                  struct tm& timestamp);

        static bool parse_year(const uint8_t* ptr, unsigned& year);
        static bool parse_month(const uint8_t* ptr, unsigned& mon);
        static bool parse_time(const uint8_t* ptr,
                               unsigned& hour,
                               unsigned& min,
                               unsigned& sec);
    };

    inline time_t date::parse(const void* buf, size_t len)
    {
      struct tm timestamp;
      return parse(buf, len, timestamp);
    }

    inline bool date::parse_year(const uint8_t* ptr, unsigned& year)
    {
      if ((!IS_DIGIT(*ptr)) ||
          (!IS_DIGIT(*(ptr + 1))) ||
          (!IS_DIGIT(*(ptr + 2))) ||
          (!IS_DIGIT(*(ptr + 3)))) {
        return false;
      }

      year = ((*ptr - '0') * 1000) +
             ((*(ptr + 1) - '0') * 100) +
             ((*(ptr + 2) - '0') * 10) +
             (*(ptr + 3) - '0');

      return (year >= 1970);
    }
  }
}

#endif // NET_HTTP_DATE_H
