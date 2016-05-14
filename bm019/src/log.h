#ifndef LOG_H
#define LOG_H

/* Set this if you need debug messages on the console;
* it will have an impact on code-size and power consumption. */

#ifndef NEED_CONSOLE_OUTPUT 
	#define NEED_CONSOLE_OUTPUT 0 
#endif

#ifndef ARDUINO
	#include "mbed.h"
#endif

#if NEED_CONSOLE_OUTPUT
#define DEBUG(...) { printf(__VA_ARGS__); }
#else
#define DEBUG(...) /* nothing */
#endif /* #if NEED_CONSOLE_OUTPUT */

#endif /* #if ndef LOG_H */

