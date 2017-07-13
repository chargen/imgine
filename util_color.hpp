
#ifndef _UTIL_COLOR_HPP
#define _UTIL_COLOR_HPP

#include <opencv2/opencv.hpp>

#include <string>

using namespace cv;

using std::string;

namespace util_color {

enum Colorspace {
    RGB, BGR, HSV, HLS, YCrCb, CIEXYZ, CIELAB, LMS, Ruderman_lab
};

const std::unordered_map<string, Colorspace>
COLORSPACE_STRINGS = {
    {"RGB", RGB},
    {"BGR", BGR},
    {"HSV", HSV},
    {"HLS", HLS}, {"HSL", HLS},
    {"YCrCb", YCrCb}, {"YCbCr", YCrCb},
    {"CIEXYZ", CIEXYZ}, {"XYZ", CIEXYZ},
    {"CIELAB", CIELAB},
    {"LMS", LMS},
    {"Ruderman_lab", Ruderman_lab}
};

string rgb_to_hex(unsigned char, unsigned char, unsigned char);
float alpha_to_opacity(unsigned char);
string alpha_to_opacity_percentage(unsigned char);

Mat convert_BGR_to_CIEXYZ(Mat, bool);
Mat convert_CIEXYZ_to_LMS(Mat, bool);
Mat convert_LMS_to_Ruderman_lab(Mat, bool);
Mat convert_colorspace(Mat, Colorspace, Colorspace);



} // namespace util_color

#endif // _UTIL_COLOR_HPP
