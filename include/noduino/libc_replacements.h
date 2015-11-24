/*
 *  Copyright (c) 2015 - 2025 MaiKe Labs
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
*/
#ifndef __LIBC_REPACEMENTS__
#define __LIBC_REPACEMENTS__

void *malloc(size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
int puts(const char *str);
int printf(const char *format, ...);
int sprintf(char *buffer, const char *format, ...);
int snprintf(char *buffer, size_t size, const char *format, ...);
int vprintf(const char *format, va_list arg);
int vsnprintf(char *buffer, size_t size, const char *format, va_list arg);
size_t strnlen(const char *s, size_t len);
char *strchr(const char *str, int character);
char *strrchr(const char *str, int character);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
char *strtok_r(char *s, const char *delim, char **last);
char *strtok(char *s, const char *delim);
int strcasecmp(const char *str1, const char *str2);
char *strdup(const char *str);
double strtod(const char *str, char **endptr);
int isalnum(int c);
int isalpha(int c);
int iscntrl(int c);
int isdigit(int c);
int isgraph(int c);
int islower(int c);
int isprint(int c);
int ispunct(int c);
int isspace(int c);
int isupper(int c);
int isxdigit(int c);
int tolower(int c);
int toupper(int c);
int isblank(int c);
int *__errno(void);
size_t strlcpy(char *dst, const char *src, size_t size);
long strtol(const char *nptr, char **endptr, int base);
unsigned long strtoul(const char *nptr, char **endptr, int base);

#endif
