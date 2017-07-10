
#include "img_core.hpp"
#include "util_color.hpp"

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace util_color;

using std::cout;
using std::endl;

namespace img_core {

/** Color Transfer.
 *  References:
 *    E. Reinhard et al., "Color Transfer between Images". 2001.
 *    E. Reinhard and T. Pouli, "Colour Spaces for Colour Transfer". 2011.
 */
Mat algo_color_transfer(Canvas *src_canvas, Canvas *ref_canvas, Colorspace space)
{
    // TODO: handle non-CV_8UC3 images

    Mat src_mat = src_canvas->current->mat->clone();
    Mat src_s(src_mat, src_canvas->current->roi);
    Mat ref_s(*(ref_canvas->current->mat), ref_canvas->current->roi);
    Mat dst_mat;

    src_mat.convertTo(src_mat, CV_32FC3);
    src_s.convertTo(src_s, CV_32FC3);
    ref_s.convertTo(ref_s, CV_32FC3);
    // scale down to [0,1]
    src_mat *= 1. / 255;
    src_s *= 1. / 255;
    ref_s *= 1. / 255;

    if (space == RGB || space == BGR) {
        cvtColor(src_mat, src_mat, CV_BGR2RGB);
        cvtColor(src_s, src_s, CV_BGR2RGB);
        cvtColor(ref_s, ref_s, CV_BGR2RGB);
    } else if (space == HSV) {
        cvtColor(src_mat, src_mat, CV_BGR2HSV);
        cvtColor(src_s, src_s, CV_BGR2HSV);
        cvtColor(ref_s, ref_s, CV_BGR2HSV);
    } else if (space == CIEXYZ) {
        cvtColor(src_mat, src_mat, CV_BGR2XYZ);
        cvtColor(src_s, src_s, CV_BGR2XYZ);
        cvtColor(ref_s, ref_s, CV_BGR2XYZ);
    } else if (space == CIELAB) {
        cvtColor(src_mat, src_mat, CV_BGR2Lab);
        cvtColor(src_s, src_s, CV_BGR2Lab);
        cvtColor(ref_s, ref_s, CV_BGR2Lab);
    } else { // default: Ruderman lαβ
        cvtColor(src_mat, src_mat, CV_BGR2XYZ);
        cvtColor(src_s, src_s, CV_BGR2XYZ);
        cvtColor(ref_s, ref_s, CV_BGR2XYZ);
        src_mat = space_lms_to_lalphabeta(space_xyz_to_lms(src_mat));
        src_s = space_lms_to_lalphabeta(space_xyz_to_lms(src_s));
        ref_s = space_lms_to_lalphabeta(space_xyz_to_lms(ref_s));
    }

    // compute partial statistics of swatches (ROIs)
    vector<double> src_s_mean, ref_s_mean, src_s_stddev, ref_s_stddev;
    meanStdDev(src_s, src_s_mean, src_s_stddev);
    meanStdDev(ref_s, ref_s_mean, ref_s_stddev);

    // split images into single-channel matrices
    vector<Mat> src_comp, dst_comp;
    split(src_mat, src_comp);
    split(src_mat, dst_comp);

    // color transfer per channel
    for (int i = 0; i < 3; i++) {
        dst_comp[i] = (ref_s_stddev[i] / src_s_stddev[i])
            * (src_comp[i] - src_s_mean[i]) + ref_s_mean[i];
    }

    // merge into a multi-channel matrix
    merge(dst_comp, dst_mat);

    if (space == RGB || space == BGR) {
        cvtColor(dst_mat, dst_mat, CV_RGB2BGR);
    } else if (space == HSV) {
        cvtColor(dst_mat, dst_mat, CV_HSV2BGR);
    } else if (space == CIEXYZ) {
        cvtColor(dst_mat, dst_mat, CV_XYZ2BGR);
    } else if (space == CIELAB) {
        cvtColor(dst_mat, dst_mat, CV_Lab2BGR);
    } else { // default: Ruderman lαβ
        dst_mat = space_lms_to_xyz(space_lalphabeta_to_lms(dst_mat));
        cvtColor(dst_mat, dst_mat, CV_XYZ2BGR);
    }

    // scale up to [0,255]
    dst_mat *= 255;
    dst_mat.convertTo(dst_mat, CV_8UC3);

    return dst_mat;
}



} // namespace img_core
