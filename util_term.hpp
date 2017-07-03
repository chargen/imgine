
#ifndef _UTIL_TERM_HPP
#define _UTIL_TERM_HPP

#include <cstdarg>

namespace util_term {

bool check_ansi();
bool check_truecolor();

namespace log {

int vfecho(const char *, va_list);
int vfinfo(const char *, va_list);
int vfwarn(const char *, va_list);
int vferr(const char *, va_list);

int fecho(const char *, ...);
int finfo(const char *, ...);
int fwarn(const char *, ...);
int ferr(const char *, ...);

} // namespace util_term::log

} // namespace util_term

#endif // _UTIL_TERM_HPP
