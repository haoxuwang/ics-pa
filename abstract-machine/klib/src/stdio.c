#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int vsprintf(char *out, const char *fmt, va_list ap);
int vsnprintf(char *out, size_t n, const char *fmt, va_list ap);

int printf(const char *fmt, ...) {
	char buffer[2048];
	va_list arg;
	va_start (arg, fmt);
	int done = vsprintf(buffer, fmt, arg);
	putstr(buffer);
	va_end(arg);
	return done;
}

//static char HEX_CHARACTERS[] = "0123456789ABCDEF";
#define BIT_WIDE_HEX 8

int vsprintf(char *out, const char *fmt, va_list ap) {
  	return vsnprintf(out, -1, fmt, ap);
}

int sprintf(char *out, const char *fmt, ...) {
	va_list valist;
	va_start(valist, fmt);
	int res = vsprintf(out ,fmt, valist);
	va_end(valist);
	return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
	va_list arg;
	va_start (arg, fmt);
	int done = vsnprintf(out, n, fmt, arg);
	va_end(arg);
	return done;
}

#define append(x) {out[j++]=x; if (j >= n) {break;}}

void reverse(char* start, char* end) {
    while (start < end) {
        char temp = *start;
        *start++ = *end;
        *end-- = temp;
    }
}

int vsnprintf(char* str, size_t len, const char* format, va_list args) {
    char* start = str;
    char padding_char;
    int min_field_width;

    for (; *format != '\0'; format++) {
        if (*format == '%') {
            padding_char = ' ';
            min_field_width = 0;

            format++;
            while (*format >= '0' && *format <= '9') {
                min_field_width *= 10;
                min_field_width += (*format - '0');
                format++;
            }
            if (*format == '0') {
                padding_char = '0';
                format++;
            }

            min_field_width = min_field_width ? min_field_width : 1;

            switch(*format) {
                case 'd': {
                    int value = va_arg(args, int);
                    char number[11];
                    char* ptr = number;

                    if (value == 0) {
                        *ptr++ = '0';
                    } else {
                        while (value > 0) {
                            *ptr++ = '0' + value % 10;
                            value /= 10;
                        }
                    }
                    int num_digits = ptr - number;
                    reverse(number, ptr - 1);

                    while (num_digits < min_field_width && start - str < len - 1) {
                        *start++ = padding_char;
                        num_digits++;
                    }

                    for (char *p = number; p < ptr && start - str < len - 1; p++) {
                        *start++ = *p;
                    }
                    break;
                }
                case 's': {
                    char* ptr = va_arg(args, char*);
                    while (*ptr != '\0' && start - str < len - 1) {
                        *start++ = *ptr++;
                    }
                    break;
                }
                default:
                    return -1;
            }
        } else if (start - str < len - 1) {
            *start++ = *format;
        }
    }

    *start = '\0';
    return start - str;
}


#endif