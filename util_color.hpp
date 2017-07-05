
#ifndef _UTIL_COLOR_HPP
#define _UTIL_COLOR_HPP

#include <string>

using std::string;

namespace util_color {

string rgb_to_hex(unsigned char, unsigned char, unsigned char);
float alpha_to_opacity(unsigned char);



} // namespace util_color

#endif // _UTIL_COLOR_HPP
