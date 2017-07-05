
#ifndef _UTIL_TERM_HPP
#define _UTIL_TERM_HPP

#include <cstdarg>
#include <string>

/** SGR (Select Graphic Rendition) Parameters.
 *  Reference:
 *    Standard ECMA-48 (Control Functions for Coded Character Sets)
 *    <http://www.ecma-international.org/publications/standards/Ecma-048.htm>
 *    ANSI escape code
 *    <https://en.wikipedia.org/wiki/ANSI_escape_code>
 */
#define SGR(code)                    "\33[" code "m"
#define SGR_RESET                    SGR("0")
#define SGR_BOLD                     SGR("1")
#define SGR_FAINT                    SGR("2")   // not widely supported
#define SGR_ITALICIZED               SGR("3")   // not widely supported
#define SGR_UNDERLINED               SGR("4")
#define SGR_BLINK                    SGR("5")
#define SGR_BLINK_RAPID              SGR("6")   // not widely supported
#define SGR_INVERSE                  SGR("7")
#define SGR_INVISIBLE                SGR("8")   // not widely supported
#define SGR_CROSSED_OUT              SGR("9")   // not widely supported
#define SGR_DOUBLY_UNDERLINED        SGR("21")  // not widely supported
#define SGR_NORMAL                   SGR("22")  // i.e. neither bold nor faint
#define SGR_NOT_ITALICIZED           SGR("23")
#define SGR_NOT_UNDERLINED           SGR("24")
#define SGR_STEADY                   SGR("25")  // i.e. not blinking
#define SGR_POSITIVE                 SGR("27")  // i.e. not inverse
#define SGR_VISIBLE                  SGR("28")
#define SGR_NOT_CROSSED_OUT          SGR("29")
#define SGR_BLACK                    SGR("30")
#define SGR_RED                      SGR("31")
#define SGR_GREEN                    SGR("32")
#define SGR_YELLOW                   SGR("33")
#define SGR_BLUE                     SGR("34")
#define SGR_MAGENTA                  SGR("35")
#define SGR_CYAN                     SGR("36")
#define SGR_WHITE                    SGR("37")
#define SGR_256(s)                   SGR("38;5;" s)
#define SGR_DEFAULT                  SGR("39")
#define SGR_BACKGROUND_BLACK         SGR("40")
#define SGR_BACKGROUND_RED           SGR("41")
#define SGR_BACKGROUND_GREEN         SGR("42")
#define SGR_BACKGROUND_YELLOW        SGR("43")
#define SGR_BACKGROUND_BLUE          SGR("44")
#define SGR_BACKGROUND_MAGENTA       SGR("45")
#define SGR_BACKGROUND_CYAN          SGR("46")
#define SGR_BACKGROUND_WHITE         SGR("47")
#define SGR_BACKGROUND_256(s)        SGR("48;5;" s)
#define SGR_BACKGROUND_DEFAULT       SGR("49")
// 16-color support: aixterm colors are the bright versions of ISO colors
#define SGR_LIGHT_BLACK              SGR("90")  // i.e. dark gray
#define SGR_LIGHT_RED                SGR("91")
#define SGR_LIGHT_GREEN              SGR("92")
#define SGR_LIGHT_YELLOW             SGR("93")
#define SGR_LIGHT_BLUE               SGR("94")
#define SGR_LIGHT_MAGENTA            SGR("95")
#define SGR_LIGHT_CYAN               SGR("96")
#define SGR_LIGHT_WHITE              SGR("97")
#define SGR_BACKGROUND_LIGHT_BLACK   SGR("100") // i.e. dark gray
#define SGR_BACKGROUND_LIGHT_RED     SGR("101")
#define SGR_BACKGROUND_LIGHT_GREEN   SGR("102")
#define SGR_BACKGROUND_LIGHT_YELLOW  SGR("103")
#define SGR_BACKGROUND_LIGHT_BLUE    SGR("104")
#define SGR_BACKGROUND_LIGHT_MAGENTA SGR("105")
#define SGR_BACKGROUND_LIGHT_CYAN    SGR("106")
#define SGR_BACKGROUND_LIGHT_WHITE   SGR("107")

/** Cursor Next Line.
 */
#define CNL(n)                       "\33[" BOOST_PP_STRINGIZE(n) "E"
/** Cursor Previous Line.
 */
#define CPL(n)                       "\33[" BOOST_PP_STRINGIZE(n) "F"
/** Erase in Line.
 */
#define EL(n)                        "\33[" BOOST_PP_STRINGIZE(n) "K"

using std::string;

namespace util_term {

bool check_ansi();
bool check_truecolor();
int get_width();

string sgr_rgb(unsigned char, unsigned char, unsigned char, string);
string sgr_background_rgb(unsigned char, unsigned char, unsigned char, string);

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
