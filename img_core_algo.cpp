
#include "img_core.hpp"

#include <opencv2/opencv.hpp>

using namespace cv;

using std::cout;
using std::endl;

namespace img_core {

/** Color Transfer.
 *  (E. Reinhard et al., "Color Transfer between Images". 2001.)
 */
void ImgineContext::algo_color_transfer(Canvas *src_canvas, Canvas *ref_canvas)
{
    // TODO: handle non-3-channel images

    // convert images to logarithmic, floating-point Lab space
    Mat src_mat, ref_mat, dst_mat;
    cvtColor(*(src_canvas->current->mat), src_mat, CV_BGR2Lab);
    cvtColor(*(ref_canvas->current->mat), ref_mat, CV_BGR2Lab);
    src_mat.convertTo(src_mat, CV_32FC3);
    ref_mat.convertTo(ref_mat, CV_32FC3);
    log(src_mat, src_mat);
    log(ref_mat, ref_mat);

    // compute partial statistics of swatches (ROIs)
    Mat src_s(src_mat, src_canvas->current->roi);
    Mat ref_s(ref_mat, ref_canvas->current->roi);
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

    // merge and generate the target BGR image
    merge(dst_comp, dst_mat);
    exp(dst_mat, dst_mat);
    dst_mat.convertTo(dst_mat, CV_8UC3);
    cvtColor(dst_mat, dst_mat, CV_Lab2BGR);

    new_canvas();
    *active_canvas->current->mat = dst_mat;
    //active_canvas->current->mat = new Mat(dst_mat);
    if (active_canvas->current->mat->data) {
        int rows = active_canvas->current->mat->rows;
        int cols = active_canvas->current->mat->cols;
        int cv_type = active_canvas->current->mat->type();
        active_canvas->current->roi = Rect2d(0, 0, cols, rows);
        // TODO: rename canvas
        active_canvas->rows = rows;
        active_canvas->cols = cols;
        active_canvas->cv_type = cv_type;
        cout << "  Canvas name:\t" << active_canvas->name << endl;
    }
}

} // namespace img_core
