
#include "img_core.hpp"
#include "util_color.hpp"

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace util_color;

using std::cout;
using std::endl;

namespace img_core {

/** Convert a BGR color image to grayscale.
 *  (y = 0.299 * r + 0.587 * g + 0.114 * b)
 */
Mat algo_grayscale(Canvas *src_canvas)
{
    // TODO: handle non-BGR images

    Mat dst_mat = src_canvas->current->mat->clone();
    cvtColor(dst_mat, dst_mat, COLOR_BGR2GRAY);
    return dst_mat;
}

/** Histogram Equalization.
 */
Mat algo_equalize_hist(Canvas *src_canvas, Colorspace space)
{
    Mat dst_mat = src_canvas->current->mat->clone();

    if (dst_mat.channels() >= 3) {
        switch (space) {
        case HSV:
            cvtColor(dst_mat, dst_mat, CV_BGR2HSV);
            break;
        default: // default: CIELAB
            cvtColor(dst_mat, dst_mat, CV_BGR2Lab);
        }
    }

    // split image into single-channel matrices
    vector<Mat> dst_comp;
    split(dst_mat, dst_comp);

    // equalize the relevant component of histogram
    switch (space) {
    case HSV:
        equalizeHist(dst_comp.back(), dst_comp.back()); // V - Value
        break;
    default: // grayscale or default: CIELAB
        equalizeHist(dst_comp[0], dst_comp[0]); // L - Lightness
    }

    // merge into a multi-channel matrix
    merge(dst_comp, dst_mat);

    if (dst_mat.channels() >= 3) {
        switch (space) {
        case HSV:
            cvtColor(dst_mat, dst_mat, CV_HSV2BGR);
            break;
        default: // default: CIELAB
            cvtColor(dst_mat, dst_mat, CV_Lab2BGR);
        }
    }

    return dst_mat;
}

/** Color Transfer.
 *  References:
 *    E. Reinhard et al., "Color Transfer between Images". 2001.
 *    E. Reinhard and T. Pouli, "Colour Spaces for Colour Transfer". 2011.
 */
Mat algo_color_transfer(Canvas *src_canvas, Canvas *ref_canvas, Colorspace space)
{
    // TODO: handle non-CV_8UC3-BGR images

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

    if (space == RGB) {
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
        src_mat = convert_colorspace(convert_colorspace(src_mat, CIEXYZ, LMS), LMS, Ruderman_lab);
        src_s = convert_colorspace(convert_colorspace(src_s, CIEXYZ, LMS), LMS, Ruderman_lab);
        ref_s = convert_colorspace(convert_colorspace(ref_s, CIEXYZ, LMS), LMS, Ruderman_lab);
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

    if (space == RGB) {
        cvtColor(dst_mat, dst_mat, CV_RGB2BGR);
    } else if (space == HSV) {
        cvtColor(dst_mat, dst_mat, CV_HSV2BGR);
    } else if (space == CIEXYZ) {
        cvtColor(dst_mat, dst_mat, CV_XYZ2BGR);
    } else if (space == CIELAB) {
        cvtColor(dst_mat, dst_mat, CV_Lab2BGR);
    } else { // default: Ruderman lαβ
        dst_mat = convert_colorspace(convert_colorspace(dst_mat, Ruderman_lab, LMS), LMS, CIEXYZ);
        cvtColor(dst_mat, dst_mat, CV_XYZ2BGR);
    }

    // scale up to [0,255]
    dst_mat *= 255;
    dst_mat.convertTo(dst_mat, CV_8UC3);

    return dst_mat;
}



} // namespace img_core
