#ifndef LOG_H
#define LOG_H

void msg_set(const char *fmt, ...);
char *msg_get(void);
void msg_log(const char *fmt, ...);
void msg_fail(const char *fmt, ...);

#endif
