#ifndef MACROS_H
#define MACROS_H

#define ARRAY_SIZE(x)     (sizeof(x) / sizeof(*x))

#define MAX(x, y)         (((x) > (y)) ? (x) : (y))
#define MIN(x, y)         (((x) < (y)) ? (x) : (y))

#define IS_ALPHA(x)       ((((x) >= 'A') && ((x) <= 'Z')) || (((x) >= 'a') && ((x) <= 'z')))
#define IS_DIGIT(x)       (((x) >= '0') && ((x) <= '9'))
#define IS_XDIGIT(x)      ((IS_DIGIT(x)) || (((x) >= 'A') && ((x) <= 'F')) || (((x) >= 'a') && ((x) <= 'f')))
#define IS_WHITE_SPACE(x) (((x) == ' ') || ((x) == '\t'))

#endif // MACROS_H
