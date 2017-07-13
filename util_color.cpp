
#include "util_color.hpp"

#include <opencv2/opencv.hpp>

#include <iomanip>
#include <sstream>

using namespace cv;

using std::to_string;

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

/** Convert alpha value to opacity percentage (as a string).
 */
string alpha_to_opacity_percentage(unsigned char a)
{
    return to_string(int(a / 255. * 100)) + "%";
}

/** Convert a 3-channel matrix from BGR space to CIE XYZ space.
 *  Reference:
 *    <http://docs.opencv.org/3.0-beta/modules/imgproc/doc/miscellaneous_transformations.html>
 */
Mat convert_BGR_to_CIEXYZ(Mat src, bool forwardDirection = true)
{
    Mat trans_mat = (Mat_<float>(3,3) <<
                     0.412453, 0.357580, 0.180423,
                     0.212671, 0.715160, 0.072169,
                     0.019334, 0.119193, 0.950227);
    Mat dst = src.clone();
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            Vec3f& s = src.at<Vec3f>(i, j);
            Mat t = (forwardDirection ? trans_mat : trans_mat.inv()) *
                (Mat_<float>(3,1) << s.val[2], s.val[1], s.val[0]); // BGR to RGB
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
Mat convert_CIEXYZ_to_LMS(Mat src, bool forwardDirection = true)
{
    Mat trans_mat = (Mat_<float>(3,3) <<
                     0.38971, 0.68898, -0.07868,
                     -0.22981, 1.18340, 0.04641,
                     0.00000, 0.00000, 1.00000);
    Mat dst = src.clone();
    for (int i = 0; i < src.rows; i++) {
        for (int j = 0; j < src.cols; j++) {
            Vec3f& s = src.at<Vec3f>(i, j);
            Mat t = (forwardDirection ? trans_mat : trans_mat.inv()) * Mat(s);
            dst.at<Vec3f>(i, j).val[0] = t.at<float>(0, 0);
            dst.at<Vec3f>(i, j).val[1] = t.at<float>(1, 0);
            dst.at<Vec3f>(i, j).val[2] = t.at<float>(2, 0);
        }
    }
    return dst;
}

/** Convert a 3-channel matrix from LMS space to Ruderman's lαβ space.
 *  Reference:
 *    E. Reinhard and T. Pouli, "Colour Spaces for Colour Transfer". 2011.
 */
Mat convert_LMS_to_Ruderman_lab(Mat src, bool forwardDirection = true)
{
    Mat trans_mat1 = (Mat_<float>(3,3) <<
                      1, 1, 1,
                      1, 1, -2,
                      1, -1, 0);
    Mat trans_mat2 = (Mat_<float>(3,3) <<
                      1. / sqrt(3), 0, 0,
                      0, 1. / sqrt(6), 0,
                      0, 0, 1. / sqrt(2));
    Mat dst = src.clone();
    if (forwardDirection)
        log(dst, dst);
    for (int i = 0; i < dst.rows; i++) {
        for (int j = 0; j < dst.cols; j++) {
            Vec3f& s = dst.at<Vec3f>(i, j);
            Mat t = (forwardDirection ? trans_mat2 * (trans_mat1 * Mat(s))
                     : trans_mat1.inv() * (trans_mat2.inv() * Mat(s)));
            dst.at<Vec3f>(i, j).val[0] = t.at<float>(0, 0);
            dst.at<Vec3f>(i, j).val[1] = t.at<float>(1, 0);
            dst.at<Vec3f>(i, j).val[2] = t.at<float>(2, 0);
        }
    }
    if (!forwardDirection)
        exp(dst, dst);
    return dst;
}

/** General colorspace conversion funtion.
 */
Mat convert_colorspace(Mat src, Colorspace src_space, Colorspace dst_space)
{
    if (src_space == CIEXYZ && dst_space == LMS) {
        return convert_CIEXYZ_to_LMS(src, true);

    } else if (src_space == LMS && dst_space == CIEXYZ) {
        return convert_CIEXYZ_to_LMS(src, false);

    } else if (src_space == LMS && dst_space == Ruderman_lab) {
        return convert_LMS_to_Ruderman_lab(src, true);

    } else if (src_space == Ruderman_lab && dst_space == LMS) {
        return convert_LMS_to_Ruderman_lab(src, false);

    } else {
        return src; // FIXME:
    }
}



} // namespace util_color
