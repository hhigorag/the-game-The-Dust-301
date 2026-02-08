#ifndef ARC_TERMINAL_ARC_UTILS_H
#define ARC_TERMINAL_ARC_UTILS_H

#include <stddef.h>

size_t arc_strnlen(const char* s, size_t maxLen);
void arc_strtrim_left(char* s);
void arc_strlower(char* s);

unsigned long long arc_get_total_ram(void);
void arc_get_cpu_name(char* buffer);

void Arc_GetFormattedDateTime(char* out, size_t outSize);

#endif
