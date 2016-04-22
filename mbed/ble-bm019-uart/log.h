
#ifndef LOG_H
#define LOG_H

#define NEED_CONSOLE_OUTPUT 0 
/* DO NOT Set to 1 !! 
 * the nrf51 has only one uart and we need it for uart com
 */

#include "mbed.h"

#if NEED_CONSOLE_OUTPUT
#define DEBUG(...) { printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */

#endif /* #if ndef LOG_H */
