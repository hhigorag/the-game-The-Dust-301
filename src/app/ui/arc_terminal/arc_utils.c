#include "app/ui/arc_terminal/arc_utils.h"
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOGDI
#define NOUSER
#include <windows.h>
#undef DrawText
#endif

#ifndef _WIN32
#include <time.h>
#endif

size_t arc_strnlen(const char* s, size_t maxLen) {
    size_t n = 0;
    if (!s) return 0;
    while (n < maxLen && s[n] != '\0') n++;
    return n;
}

void arc_strtrim_left(char* s) {
    if (!s) return;
    size_t i = 0;
    while (s[i] && (unsigned char)s[i] <= 32) i++;
    if (i) memmove(s, s + i, strlen(s + i) + 1);
}

void arc_strlower(char* s) {
    if (!s) return;
    for (; *s; s++) *s = (char)tolower((unsigned char)*s);
}

#ifdef _WIN32
unsigned long long arc_get_total_ram(void) {
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) return 16;
    return (unsigned long long)(status.ullTotalPhys / (1024ULL * 1024 * 1024));
}

void arc_get_cpu_name(char* buffer) {
    HKEY hKey;
    if (!buffer) return;
    memset(buffer, 0, 256);
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD bufSize = 256;
        if (RegQueryValueExA(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)buffer, &bufSize) != ERROR_SUCCESS) {
            strncpy(buffer, "ARC PROTOCOL CPU", 255);
            buffer[255] = '\0';
        }
        RegCloseKey(hKey);
    } else {
        strncpy(buffer, "ARC PROTOCOL CPU", 255);
        buffer[255] = '\0';
    }
}
#else
unsigned long long arc_get_total_ram(void) { return 16; }
void arc_get_cpu_name(char* buffer) {
    if (buffer) { strncpy(buffer, "ARC PROTOCOL CPU", 255); buffer[255] = '\0'; }
}
#endif

void Arc_GetFormattedDateTime(char* out, size_t outSize) {
    if (!out || outSize < 2) return;
#ifdef _WIN32
    SYSTEMTIME st;
    GetLocalTime(&st);
    snprintf(out, outSize - 1, "%02d:%02d %02d/%02d/2226",
             (int)st.wHour, (int)st.wMinute,
             (int)st.wDay, (int)st.wMonth);
#else
    time_t t = time(NULL);
    struct tm* tm_info = localtime(&t);
    if (!tm_info) { out[0] = '\0'; return; }
    snprintf(out, outSize - 1, "%02d:%02d %02d/%02d/2226",
             tm_info->tm_hour, tm_info->tm_min,
             tm_info->tm_mday, tm_info->tm_mon + 1);
#endif
    out[outSize - 1] = '\0';
}
