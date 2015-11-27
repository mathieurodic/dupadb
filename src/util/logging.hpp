#ifndef __INCLUDED__util__logging__hpp__
#define __INCLUDED__util__logging__hpp__

#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ham/hamsterdb.h>

#define COLOR_BLACK     "\x1B[30m"
#define COLOR_RED       "\x1B[31m"
#define COLOR_GREEN     "\x1B[32m"
#define COLOR_YELLOW    "\x1B[33m"
#define COLOR_BLUE      "\x1B[34m"
#define COLOR_MAGENTA   "\x1B[35m"
#define COLOR_CYAN      "\x1B[36m"
#define COLOR_WHITE     "\x1B[37m"
#define COLOR_NORMAL    "\x1B[39m"

#define STYLE_NORMAL    "\x1b[0m"
#define STYLE_BOLD      "\x1b[1m"
#define STYLE_INVERSE   "\x1b[7m"

#define GRAY            COLOR_BLACK STYLE_BOLD
#define RESET           COLOR_NORMAL STYLE_NORMAL

#ifndef LOG_OUTPUT
#define LOG_OUTPUT stderr
#endif

inline double millitime() {
    double t;
    timeval tv;
    gettimeofday(&tv, NULL);
    t = (double)tv.tv_sec + 1e-6 * (double)tv.tv_usec;
    return t;
}

static double _log_t0 = millitime();
static double _log_t1 = 0;
char* _log_last_color = COLOR_MAGENTA;

inline void log_reset_time() {
    _log_t0 = millitime();
}

#define _log(TEXT, TYPE, COLOR, ...) { \
    double _log_T = millitime(); \
    if (_log_t1) { \
        fprintf(LOG_OUTPUT, "dt = " STYLE_BOLD "%013.6f", _log_T - _log_t1); \
    } \
    fprintf(LOG_OUTPUT, "\n" COLOR TYPE " "); \
    fprintf(LOG_OUTPUT, STYLE_NORMAL COLOR); \
    for (int i=0, n=12-strlen(TYPE); i<n; i++) { \
        fprintf(LOG_OUTPUT, "-"); \
    } \
    char _log_buffer[1024]; \
    snprintf(_log_buffer, 1023, TEXT, ##__VA_ARGS__); \
    size_t _log_buffer_length = strlen(_log_buffer); \
    for (int p=0; p<_log_buffer_length; p+=64) { \
        if (p) { \
            fprintf(LOG_OUTPUT, "\n-----------------"); \
        } \
        fprintf(LOG_OUTPUT, STYLE_BOLD " %.*s " STYLE_NORMAL COLOR, 64, _log_buffer + p); \
    } \
    for (int i=0, n=64-(_log_buffer_length%64); i<n; i++) { \
        fprintf(LOG_OUTPUT, "-"); \
    } \
    snprintf(_log_buffer, 63, "%s : %d", __FILE__, __LINE__); \
    fprintf(LOG_OUTPUT, STYLE_BOLD " %s " STYLE_NORMAL COLOR ":" STYLE_BOLD " %d " STYLE_NORMAL COLOR, __FILE__, __LINE__); \
    for (int i=0, n=64-strlen(_log_buffer); i<n; i++) { \
        fprintf(LOG_OUTPUT, "-"); \
    } \
    fprintf(LOG_OUTPUT, " t = %013.6f ---- ", _log_T - _log_t0); \
    fflush(LOG_OUTPUT); \
    _log_t0 += millitime() - _log_T; \
    _log_t1 = millitime(); \
    _log_last_color = COLOR; \
}

#define fatal(TEXT, ...)    _log(TEXT, "FATAL",   "\x1B[41m",      ##__VA_ARGS__); fprintf(LOG_OUTPUT, STYLE_NORMAL COLOR_NORMAL "\n"); exit(1);
#define error(TEXT, ...)    _log(TEXT, "ERROR",   COLOR_RED,       ##__VA_ARGS__)
#define warning(TEXT, ...)  _log(TEXT, "WARNING", COLOR_YELLOW,    ##__VA_ARGS__)
#define message(TEXT, ...)  _log(TEXT, "MESSAGE", COLOR_GREEN,     ##__VA_ARGS__)
#define notice(TEXT, ...)   _log(TEXT, "NOTICE",  COLOR_CYAN,      ##__VA_ARGS__)
#define debug(TEXT, ...)    _log(TEXT, "DEBUG",   COLOR_BLUE,      ##__VA_ARGS__)

#define start() _log("let the program begin!", "START", COLOR_MAGENTA)
#define finish(ACTION) _log("this is the end!", "FINISH", COLOR_MAGENTA); ACTION(0)

#define _ham_log(ST, FUNCTION) if (ST != HAM_SUCCESS) { FUNCTION("HamsterDB error %d, %s", ST, ham_strerror(ST)); }
#define ham_fatal(ST) _ham_log(ST, fatal);
#define ham_error(ST) _ham_log(ST, error);
#define ham_warning(ST) _ham_log(ST, warning);
#define ham_message(ST) _ham_log(ST, message);
#define ham_notice(ST) _ham_log(ST, notice);
#define ham_debug(ST) _ham_log(ST, debug);


#endif // __INCLUDED__util__logging__hpp__
