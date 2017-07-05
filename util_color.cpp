
#include "util_color.hpp"

#include <iomanip>
#include <sstream>

namespace util_color {

/** Convert RGB values to a hexadecimal string ('#' prefixed).
 */
string rgb_to_hex(unsigned char r, unsigned char g, unsigned char b)
{
    std::stringstream ss;
    ss << "#" << std::hex
       << std::setw(2) << std::setfill('0') << (int)r
       << std::setw(2) << std::setfill('0') << (int)g
       << std::setw(2) << std::setfill('0') << (int)b;
    return ss.str();
}

/** Convert alpha value to opacity (a double between 0 and 1).
 */
double alpha_to_opacity(unsigned char a)
{
    return a / 255.;
}



} // namespace util_color
