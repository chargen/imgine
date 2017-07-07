
#include "util_color.hpp"

#include <opencv2/opencv.hpp>

#include <iomanip>
#include <sstream>

using namespace cv;

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

/** Convert a 3-channel matrix from BGR space to CIE XYZ space.
 *  Reference:
 *    <http://docs.opencv.org/3.0-beta/modules/imgproc/doc/miscellaneous_transformations.html>
 */
Mat space_bgr_to_xyz(Mat src)
{
    Mat trans_mat = (Mat_<float>(3,3) <<
                     0.412453, 0.357580, 0.180423,
                     0.212671, 0.715160, 0.072169,
                     0.019334, 0.119193, 0.950227);
    Mat dst = src.clone();
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            Vec3f& s = src.at<Vec3f>(i, j);
            Mat t = trans_mat * (Mat_<float>(3,1) <<
                                 s.val[2], s.val[1], s.val[0]); // BGR to RGB
            dst.at<Vec3f>(i, j).val[0] = t.at<float>(0, 0);
            dst.at<Vec3f>(i, j).val[1] = t.at<float>(1, 0);
            dst.at<Vec3f>(i, j).val[2] = t.at<float>(2, 0);
        }
    }
    return dst;
}

/** Convert a 3-channel matrix from CIE XYZ space to LMS space.
 *  (von Kries transformation)
 *  Reference:
 *    <https://en.wikipedia.org/wiki/LMS_color_space>
 */
Mat space_xyz_to_lms(Mat src)
{
    Mat trans_mat = (Mat_<float>(3,3) <<
                     0.38971, 0.68898, -0.07868,
                     -0.22981, 1.18340, 0.04641,
                     0.00000, 0.00000, 1.00000);
    Mat dst = src.clone();
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            Vec3f& s = src.at<Vec3f>(i, j);
            Mat t = trans_mat * Mat(s);
            dst.at<Vec3f>(i, j).val[0] = t.at<float>(0, 0);
            dst.at<Vec3f>(i, j).val[1] = t.at<float>(1, 0);
            dst.at<Vec3f>(i, j).val[2] = t.at<float>(2, 0);
        }
    }
    return dst;
}



} // namespace util_color
