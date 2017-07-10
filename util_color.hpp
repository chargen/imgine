
#ifndef _UTIL_COLOR_HPP
#define _UTIL_COLOR_HPP

#include <opencv2/opencv.hpp>

#include <string>

using namespace cv;

using std::string;

namespace util_color {

enum Colorspace {
    RGB, BGR, HSV, CIEXYZ, CIELAB, Ruderman_lab
};

const std::unordered_map<string, Colorspace>
COLORSPACE_STRINGS = {
    {"RGB", RGB},
    {"BGR", BGR},
    {"HSV", HSV},
    {"CIEXYZ", CIEXYZ},
    {"CIELAB", CIELAB},
    {"Ruderman_lab", Ruderman_lab}
};

string rgb_to_hex(unsigned char, unsigned char, unsigned char);
float alpha_to_opacity(unsigned char);

Mat space_bgr_to_xyz(Mat);
Mat space_xyz_to_lms(Mat);
Mat space_lms_to_xyz(Mat);
Mat space_lms_to_lalphabeta(Mat);
Mat space_lalphabeta_to_lms(Mat);



} // namespace util_color

#endif // _UTIL_COLOR_HPP
