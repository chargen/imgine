
#include "util_term.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <string>

extern "C" {
#include <sys/ioctl.h>
#include <unistd.h>
}

using std::getenv;
using std::string;
using std::to_string;

namespace util_term {

/** Check if the current stdout writes to an ANSI-compatible terminal.
 */
bool check_ansi()
{
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
    if (isatty(fileno(stdout))) {
        const char *term_p = getenv("TERM");
        if (term_p) {
            string term(term_p);
            if (term != "" && term != "dumb") { // not a dumb terminal
                return true;
            }
        }
    }
#endif
    return false;
}

/** Check if the terminal supports 24-bit true color.
 */
bool check_truecolor()
{
    const char *colorterm_p = getenv("COLORTERM");
    if (colorterm_p) {
        string colorterm(colorterm_p);
        if (colorterm == "truecolor" || colorterm == "24bit") {
            return true;
        }
    }
    return false;
}

/** Get the current width of the terminal.
 */
int get_width()
{
    int columns_default = 80;
#if _POSIX_C_SOURCE >= 1 || _XOPEN_SOURCE || _POSIX_SOURCE
    if (isatty(fileno(stdout))) {
        struct winsize size;
        ioctl(fileno(stdout), TIOCGWINSZ, &size);
        return size.ws_col;
    }
#endif
    return columns_default;
}

/** Apply SGR foreground RGB (true color) code to a string.
 */
string sgr_rgb(unsigned char r, unsigned char g, unsigned char b, string s)
{
    return "\33[38;2;" +
        to_string(r) + ";" + to_string(g) + ";" + to_string(b) + "m" +
        s + SGR_RESET;
}

/** Apply SGR background RGB (true color) code to a string.
 */
string sgr_background_rgb(unsigned char r, unsigned char g, unsigned char b, string s)
{
    return "\33[48;2;" +
        to_string(r) + ";" + to_string(g) + ";" + to_string(b) + "m" +
        s + SGR_RESET;
}

/** ANSI control code: Cursor Next Line.
 */
string cnl(int n)
{
    return "\33[" + to_string(n) + "E";
}

/** ANSI control code: Cursor Previous Line.
 */
string cpl(int n)
{
    return "\33[" + to_string(n) + "F";
}

/** ANSI control code: Erase in Line.
 */
string el(int n)
{
    return "\33[" + to_string(n) + "K";
}



namespace log {

int vfecho(const char *fmt, va_list args)
{
    return vfprintf(stderr, fmt, args);
}

int vfinfo(const char *fmt, va_list args)
{
    fprintf(stderr, SGR_WHITE);
    int ret = vfprintf(stderr, fmt, args);
    fprintf(stderr, SGR_RESET);
    return ret;
}

int vfwarn(const char *fmt, va_list args)
{
    fprintf(stderr, SGR_YELLOW SGR_BOLD);
    int ret = vfprintf(stderr, fmt, args);
    fprintf(stderr, SGR_RESET);
    return ret;
}

int vferr(const char *fmt, va_list args)
{
    fprintf(stderr, SGR_RED SGR_BOLD);
    int ret = vfprintf(stderr, fmt, args);
    fprintf(stderr, SGR_RESET);
    return ret;
}

int fecho(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vfecho(fmt, args);
    va_end(args);
    return ret;
}

int finfo(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vfinfo(fmt, args);
    va_end(args);
    return ret;
}

int fwarn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vfwarn(fmt, args);
    va_end(args);
    return ret;
}

int ferr(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int ret = vferr(fmt, args);
    va_end(args);
    return ret;
}

} // namespace util_term::log

} // namespace util_term
