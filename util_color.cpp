
#include "util_color.hpp"

#include <iomanip>
#include <sstream>

namespace util_color {

/** Convert RGB values to a hexadecimal string ('#' prefixed).
 */
string rgb_to_hex(unsigned char r, unsigned char g, unsigned char b)
{
    std::stringstream ss;
    ss << "#" << std::hex << std::uppercase
       << std::setw(2) << std::setfill('0') << (int)r
       << std::setw(2) << std::setfill('0') << (int)g
       << std::setw(2) << std::setfill('0') << (int)b;
    return ss.str();
}

/** Convert alpha value to opacity value (between 0 and 1).
 */
float alpha_to_opacity(unsigned char a)
{
    return int(a / 255. * 100) / 100.;
}



} // namespace util_color
