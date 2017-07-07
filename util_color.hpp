
#ifndef _UTIL_COLOR_HPP
#define _UTIL_COLOR_HPP

#include <opencv2/opencv.hpp>

#include <string>

using namespace cv;

using std::string;

namespace util_color {

string rgb_to_hex(unsigned char, unsigned char, unsigned char);
float alpha_to_opacity(unsigned char);

Mat space_bgr_to_xyz(Mat);
Mat space_xyz_to_lms(Mat);



} // namespace util_color

#endif // _UTIL_COLOR_HPP
