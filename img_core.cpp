
#include "img_core.hpp"
#include "util_color.hpp"
#include "util_term.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <opencv2/opencv.hpp>

#include <cstdarg>
#include <thread>

using namespace cv;
using namespace util_color;
using namespace util_term;

using std::cout;
using std::endl;
using std::exception;
using std::flush;
using std::get;
using std::list;
using std::string;
using std::stringstream;
using std::thread;
using std::to_string;
using std::vector;

namespace img_core {

/** Constructor of CanvasState. (given matrix size and type)
 */
CanvasState::CanvasState(int rows, int cols, int cv_type)
{
    // TODO: assign id
    this->mat = new Mat(rows, cols, cv_type, Scalar::all(0));
    this->roi = Rect2d(0, 0, cols, rows);
}

/** Constructor of CanvasState.
 */
CanvasState::CanvasState()
{
    // TODO: assign id
    this->mat = new Mat();
    this->roi = Rect2d();
}

/** Destructor of CanvasState.
 */
CanvasState::~CanvasState()
{
    this->mat->release();
    delete this->mat;
}

/** Constructor of Canvas. (given matrix size and type)
 */
Canvas::Canvas(string id, int rows, int cols, int cv_type)
{
    this->id = id;
    this->name = id;
    this->rows = rows;
    this->cols = cols;
    this->cv_type = cv_type;
    // A new canvas starts with an initial state.
    this->current = new CanvasState(rows, cols, cv_type);
    this->history.push_back(this->current);
}

/** Constructor of Canvas.
 */
Canvas::Canvas(string id)
{
    this->id = id;
    this->name = id;
    this->rows = 0;
    this->cols = 0;
    this->cv_type = CV_8UC3;
    // A new canvas starts with an initial state.
    this->current = new CanvasState();
    this->history.push_back(this->current);
}

/** Destructor of Canvas.
 */
Canvas::~Canvas()
{
    for (auto &state : history) {
        delete state;
    }
}

/** Singleton instantiator of ImgineContext.
 */
ImgineContext& ImgineContext::singleton()
{
    static ImgineContext instance;
    return instance;
}

/** Destructor of ImgineContext.
 */
ImgineContext::~ImgineContext()
{
    destroyAllWindows();
    debug("Waiting for all threads to terminate... ");
    for (auto &thread : threads) {
        thread.join();
    }
    debug("Done.\n");
    for (auto &canvas : canvases) {
        delete canvas;
    }
    // TODO: save workspace
}

/** Create a new canvas. (given matrix size and type)
 */
void ImgineContext::new_canvas(int rows, int cols, int cv_type)
{
    ++canvas_counter;
    string id = "C" + to_string(canvas_counter);
    this->active_canvas = new Canvas(id, rows, cols, cv_type);
    this->canvases.push_back(this->active_canvas);
}

/** Create a new canvas.
 */
void ImgineContext::new_canvas()
{
    ++canvas_counter;
    string id = "C" + to_string(canvas_counter);
    this->active_canvas = new Canvas(id);
    this->canvases.push_back(this->active_canvas);
}

/** Return a pointer to the canvas with the specified name.
 */
Canvas *ImgineContext::get_canvas_by_name(string canvas_name)
{
    if (canvas_name == "@")
        return active_canvas;

    for (auto &canvas : canvases) {
        if (canvas->name == canvas_name) {
            return canvas;
        }
    }
    return nullptr;
}

/** Colored printf for debugging-only log message.
 */
void ImgineContext::debug(const char *fmt, ...)
{
    if (!config.verbosity) return;

    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        log::vfinfo(fmt, args);
    else
        log::vfecho(fmt, args);
    va_end(args);
}

/** Colored printf for warning log message.
 */
void ImgineContext::warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        log::vfwarn(fmt, args);
    else
        log::vfecho(fmt, args);
    va_end(args);
}

/** Colored printf for error log message.
 */
void ImgineContext::err(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        log::vferr(fmt, args);
    else
        log::vfecho(fmt, args);
    va_end(args);
}

/** Colored printf for fatal error log message. (exit at once)
 */
void ImgineContext::wtf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        log::vferr(fmt, args);
    else
        log::vfecho(fmt, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

/** Execute a command.
 */
void ImgineContext::execute(vector<string> params)
{
    string cmd = params.at(0);

    if (cmd == ":status") {
        execute_status(params);

    } else if (cmd == ":list" || cmd == ":l") {
        execute_list(params);

    } else if (cmd == ":switch_to" || cmd == ":to") {
        execute_switch_to(params);

    } else if (cmd == ":new" || cmd == ":n") {
        execute_new(params);

    } else if (cmd == ":delete" || cmd == ":del") {
        execute_delete(params);

    } else if (cmd == ":rename" || cmd == ":ren") {
        execute_rename(params);

    } else if (cmd == ":import" ||
               cmd == ":read" || cmd == ":r") {
        execute_import(params);

    } else if (cmd == ":export" ||
               cmd == ":write" || cmd == ":w") {
        execute_export(params);

    } else if (cmd == ":properties" || cmd == ":prop" || cmd == ":p") {
        execute_properties(params);

    } else if (cmd == ":roi") {
        execute_roi(params);

    } else if (cmd == ":dump") {
        execute_dump(params);

    } else if (cmd == ":dump_roi") {
        execute_dump_roi(params);

    } else if (cmd == ":statistics" || cmd == ":stat" || cmd == ":st") {
        execute_statistics(params);

    } else if (cmd == ":show" || cmd == ":sh") {
        execute_show(params);

    } else if (cmd == ":histogram" || cmd == ":hist" || cmd == ":hi") {
        execute_histogram(params);

    } else if (cmd == ":inspect" || cmd == ":i") {
        execute_inspect(params, false);

    } else if (cmd == ":inspect_histogram" || cmd == ":inspect_hist" ||
               cmd == ":I") {
        execute_inspect(params, true);

    } else if (cmd == ":procedure" || cmd == ":proc" || cmd == ":P") {
        execute_procedure(params);

    } else if (cmd == ":Pi") { // shortcut to ":proc then :inspect"
        execute_procedure(params);
        execute_inspect({}, false);

    } else if (cmd == ":PI") { // shortcut to ":proc then :inspect_hist"
        execute_procedure(params);
        execute_inspect({}, true);

    } else {
        // TODO: more commands
        err("Unknown command.\n");
    }
}

/** Return a string list presenting the image properties of the canvas.
 */
vector<string> ImgineContext::show_properties(Canvas *canvas)
{
    vector<string> ret;
    int bitdepth = get<1>(IMG_CV_TYPES.at(canvas->cv_type));
    int channels = get<0>(IMG_CV_TYPES.at(canvas->cv_type));
    string channel_type;
    switch (channels) {
    case 4: channel_type = "RGBA"; break;
    case 3: channel_type = "RGB"; break;
    default: channel_type = "monochrome";
    }
    int kib = canvas->cols * canvas->rows * channels * bitdepth / 8 / 1024;
    int mib = kib / 1024;

    ret.push_back("  Canvas name:\t" + canvas->name);
    ret.push_back("  Canvas size:\t[" + to_string(canvas->cols) + " x " +
                  to_string(canvas->rows) + "]");
    ret.push_back("  Channels:\t" + to_string(channels) + " (" +
                  channel_type + ")");
    ret.push_back("  Color depth:\t" + to_string(bitdepth) + " bpc");
    ret.push_back("  Memory size:\t" +
                  (mib ? to_string(mib) + " MiB" : to_string(kib) + " KiB"));

    return ret;
}

/** Return a string list presenting some basic statistics of the matrix.
 */
vector<string> ImgineContext::show_statistics(Mat *mat)
{
    vector<string> ret;
    stringstream mat_mean_buf, mat_stddev_buf;
    Scalar mat_mean, mat_stddev;
    meanStdDev(*mat, mat_mean, mat_stddev);
    mat_mean_buf << mat_mean;
    mat_stddev_buf << mat_stddev;

    unsigned char r, g, b;
    if (mat->channels() >= 3) {
        b = mat_mean.val[0];
        g = mat_mean.val[1];
        r = mat_mean.val[2];
    } else {
        b = g = r = mat_mean.val[0];
    }

    ret.push_back("  Mean:\t\t" + mat_mean_buf.str());
    ret.push_back("  Mean RGB:\t" + rgb_to_hex(r, g, b));
    ret.push_back("  Std Dev:\t" + mat_stddev_buf.str());

    return ret;
}

/** Return a string list presenting a pixel value in the matrix.
 *  The last element is a "color-line" for visual color preview.
 */
vector<string> ImgineContext::show_pixel(Mat *mat, int x, int y)
{
    vector<string> ret;
    stringstream pixel_buf;
    unsigned char r, g, b, a = 255;
    if (mat->type() == CV_8UC4) {
        Vec4b pixel = mat->at<Vec4b>(y, x);
        b = pixel.val[0];
        g = pixel.val[1];
        r = pixel.val[2];
        a = pixel.val[3]; // has alpha channel
        pixel_buf << pixel;
    } else if (mat->type() == CV_8UC3) {
        Vec3b pixel = mat->at<Vec3b>(y, x);
        b = pixel.val[0];
        g = pixel.val[1];
        r = pixel.val[2];
        pixel_buf << pixel;
    } else if (mat->type() == CV_8UC2) {
        Vec2b pixel = mat->at<Vec2b>(y, x);
        b = g = r = pixel.val[0];
        a = pixel.val[1]; // has alpha channel
        pixel_buf << pixel;
    } else { // default: CV_8UC1
        Vec<uchar, 1> pixel = mat->at<uchar>(y, x);
        b = g = r = pixel.val[0];
        pixel_buf << pixel;
    }

    ret.push_back("  Pixel:\t(" + to_string(x) + string(", ") +
                  to_string(y) + string(")"));
    ret.push_back("  Value:\t" + pixel_buf.str());
    ret.push_back("  RGB hex:\t" + rgb_to_hex(r, g, b));
    ret.push_back("  Opacity:\t" + alpha_to_opacity_percentage(a));

    // color-line
    if (config.is_console_truecolor) {
        string full_line = string(config.console_columns, ' ');
        ret.push_back(sgr_background_rgb(r, g, b, full_line));
    } else { // no true-color, omit
        ret.push_back("");
    }

    return ret;
}

/** Return a histogram image of the matrix.
 */
Mat ImgineContext::draw_histogram(Mat *mat)
{
    Mat src_mat = mat->clone();

    // histogram display colors
    Scalar b, g, r;
    if (src_mat.channels() == 1) {
        // handle grayscale images
        cvtColor(src_mat, src_mat, COLOR_GRAY2BGR);
        b = g = r = Scalar(255, 255, 255);
    } else {
        b = Scalar(255, 0, 0);
        g = Scalar(0, 255, 0);
        r = Scalar(0, 0, 255);
    }

    // split images into single-channel matrices
    vector<Mat> src_comp;
    split(src_mat, src_comp);

    // calculate the histogram per channel
    Mat b_hist, g_hist, r_hist;
    int hist_size = 256;
    float range[] = {0, 256};
    const float *hist_range = {range};
    bool uniform = true, accumulate = false;
    calcHist(&src_comp[0], 1, 0, Mat(),
             b_hist, 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&src_comp[1], 1, 0, Mat(),
             g_hist, 1, &hist_size, &hist_range, uniform, accumulate);
    calcHist(&src_comp[2], 1, 0, Mat(),
             r_hist, 1, &hist_size, &hist_range, uniform, accumulate);

    // normalize the histogram per channel
    int hist_w = 512, hist_h = 256;
    int bin_w = cvRound((double)hist_w / hist_size);
    Mat hist_image(hist_h, hist_w, CV_8UC3, Scalar(0, 0, 0));
    normalize(b_hist, b_hist, 0, hist_image.rows, NORM_MINMAX, -1, Mat());
    normalize(g_hist, g_hist, 0, hist_image.rows, NORM_MINMAX, -1, Mat());
    normalize(r_hist, r_hist, 0, hist_image.rows, NORM_MINMAX, -1, Mat());

    // draw the histogram image
    for (int i = 1; i < hist_size; i++) {
        line(hist_image,
             Point(bin_w*(i-1), hist_h - cvRound(b_hist.at<float>(i-1))),
             Point(bin_w*(i), hist_h - cvRound(b_hist.at<float>(i))),
             b, 2, 8, 0);
        line(hist_image,
             Point(bin_w*(i-1), hist_h - cvRound(g_hist.at<float>(i-1))),
             Point(bin_w*(i), hist_h - cvRound(g_hist.at<float>(i))),
             g, 2, 8, 0);
        line(hist_image,
             Point(bin_w*(i-1), hist_h - cvRound(r_hist.at<float>(i-1))),
             Point(bin_w*(i), hist_h - cvRound(r_hist.at<float>(i))),
             r, 2, 8, 0);
    }
    return hist_image;
}

/** Keyboard event handler (blocking).
 */
void ImgineContext::wait_key_press(ImgineContext *context)
{
    bool is_looping = true;
    do {
        int code = waitKey(0);

        if (code == 255 // window close -- make sure the thread halts
            || code == 27 // ESC
            )
            is_looping = false;
    } while (is_looping);

    // Must call this explicitly, otherwise windows would hang.
    destroyAllWindows();

    context->state.is_gui_on = false;
    context->debug("All windows closed. GUI off.\n");
}

/** Mouse event handler (callback).
 */
void ImgineContext::on_mouse_event(int ev, int x, int y, int flags, void *c)
{
    ImgineContext *context = (ImgineContext *)c;
    Mat *mat = context->active_canvas->current->mat;
    Rect2d *roi = &context->active_canvas->current->roi;

    // Limit position to canvas area.
    x = min(mat->cols - 1, max(0, x));
    y = min(mat->rows - 1, max(0, y));

    switch (ev) {
    case EVENT_MOUSEMOVE:
    {
        vector<string> s_pixel = context->show_pixel(mat, x, y);
        cout << cpl(s_pixel.size() - 1); // no newline after color-line

        if (context->state.is_dragging) {
            int topleft_x = min(x, context->state.dragging_start_x);
            int topleft_y = min(y, context->state.dragging_start_y);
            int w = abs(x - context->state.dragging_start_x) + 1;
            int h = abs(y - context->state.dragging_start_y) + 1;
            *roi = Rect2d(topleft_x, topleft_y, w, h);

            Mat masked_mat = mat->clone();
            rectangle(masked_mat, *roi, Scalar(0, 0, 255), 1);
            imshow(context->active_canvas->name, masked_mat);

            Mat roi_mat(*mat, *roi);
            if (context->state.is_histogram_enabled) {
                Mat hist_image = context->draw_histogram(&roi_mat);
                imshow(get_histogram_name(context->active_canvas->name),
                       hist_image);
            }

            vector<string> s_statistics = context->show_statistics(&roi_mat);
            cout << cpl(s_statistics.size() + 1);

            cout << el(0) << "  Current ROI:\t" << *roi << endl;
            for (string &line : s_statistics)
                cout << el(0) << line << endl;
        }

        for (int i = 0; i < s_pixel.size() - 1; i++)
            cout << el(0) << s_pixel[i] << endl;
        cout << el(0) << s_pixel.back() << flush; // color-line
    }
    break;

    case EVENT_LBUTTONDOWN:
    {
        if (!context->state.is_dragging) {
            vector<string> s_pixel = context->show_pixel(mat, x, y);
            cout << cpl(s_pixel.size() - 1); // no newline after color-line

            context->state.is_dragging = true;
            context->state.dragging_start_x = x;
            context->state.dragging_start_y = y;

            // Reset ROI selection.
            *roi = Rect2d(0, 0, context->active_canvas->cols,
                          context->active_canvas->rows);

            imshow(context->active_canvas->name, *mat);

            Mat roi_mat(*mat, *roi);
            if (context->state.is_histogram_enabled) {
                Mat hist_image = context->draw_histogram(&roi_mat);
                imshow(get_histogram_name(context->active_canvas->name),
                       hist_image);
            }

            vector<string> s_statistics = context->show_statistics(&roi_mat);
            cout << cpl(s_statistics.size() + 1);

            cout << el(0) << "  Current ROI:\t" << *roi << endl;
            for (string &line : s_statistics)
                cout << el(0) << line << endl;

            for (int i = 0; i < s_pixel.size() - 1; i++)
                cout << el(0) << s_pixel[i] << endl;
            cout << el(0) << s_pixel.back() << flush; // color-line
        }
    }
    break;

    case EVENT_LBUTTONUP:
    {
        if (context->state.is_dragging) { // finish ROI selection
            context->state.is_dragging = false;
        }
    }
    break;

    // TODO: EVENT_RBUTTONDOWN EVENT_MBUTTONDOWN
    }
}

/**
 */
string ImgineContext::get_histogram_name(string canvas_name)
{
    return canvas_name + " (ROI histogram)";
}

/** Status:
 */
void ImgineContext::execute_status(vector<string> params)
{
    cout << "  Number of canvases:\t" << canvases.size() << endl;
}

/** List:
 */
void ImgineContext::execute_list(vector<string> params)
{
    if (params.size() == 2) {
        string scmd = params.at(1);

        if (scmd == "canvases" || scmd == "c") {
            for (const auto &canvas : canvases) {
                cout << (active_canvas && active_canvas->id == canvas->id ?
                         "@ " : "  ") << canvas->name << "\t";
                cout << "[" << canvas->cols << " x " << canvas->rows << "] ";
                int channels = get<0>(IMG_CV_TYPES.at(canvas->cv_type));
                int depth = get<1>(IMG_CV_TYPES.at(canvas->cv_type));
                cout << channels << " channels x "
                     << depth << " bits / px" << endl;
            }
        } else {
            err("Unknown subcommand.\n");
        }
    } else {
        warn("? :list SUBCOMMAND\n");
    }
}

/** Switch To:
 *  Switches to a canvas (sets it to active).
 */
void ImgineContext::execute_switch_to(vector<string> params)
{
    if (params.size() == 2) {
        string canvas_name = params.at(1);

        for (auto &canvas : canvases) {
            if (canvas->name == canvas_name) {
                active_canvas = canvas;
                cout << "  Canvas name:\t" << canvas_name << endl;
            }
        }
    } else {
        warn("? :switch_to CANVAS_NAME\n");
    }
}

/** New:
 *  Creates a new canvas.
 */
void ImgineContext::execute_new(vector<string> params)
{
    int cols = 0, rows = 0, channels = 3;
    int cv_type = CV_8UC(channels);

    if (params.size() == 1) {
        new_canvas();

    } else if (params.size() == 2) {
        stringstream(params.at(1)) >> cols;
        if (cols)
            new_canvas(cols, cols, cv_type);
        else
            err("Invalid parameter(s).\n");

    } else if (params.size() == 3) {
        stringstream(params.at(1)) >> cols;
        stringstream(params.at(2)) >> rows;
        if (cols && rows)
            new_canvas(rows, cols, cv_type);
        else
            err("Invalid parameter(s).\n");

    } else if (params.size() == 4) {
        stringstream(params.at(1)) >> cols;
        stringstream(params.at(2)) >> rows;
        stringstream(params.at(3)) >> channels;
        if (cols && rows && 1 <= channels && channels <= 4) {
            cv_type = CV_8UC(channels);
            new_canvas(rows, cols, cv_type);
        } else {
            err("Invalid parameter(s).\n");
        }
    } else {
        err("Incorrect number of parameters.\n");
    }
}

/** Delete:
 *  Deletes a canvas by name.
 */
void ImgineContext::execute_delete(vector<string> params)
{
    if (params.size() == 2) {
        string canvas_name = params.at(1);

        for (auto &canvas : canvases) {
            if (canvas->name == canvas_name) {
                if (active_canvas == canvas)
                    active_canvas = nullptr;
                canvases.remove(canvas);
                delete canvas;
                return;
            }
        }
        err("Canvas not found.\n");
    } else {
        warn("? :delete CANVAS_NAME\n");
    }
}

/** Rename:
 *  Renames the active canvas.
 */
void ImgineContext::execute_rename(vector<string> params)
{
    if (params.size() == 2) {
        string canvas_name = params.at(1);

        // TODO: disallow duplicate names and "@"!
        if (active_canvas) {
            active_canvas->name = canvas_name;
            cout << "  Canvas name:\t" << canvas_name << endl;
        } else {
            err("No active canvas.\n");
        }
    } else {
        warn("? :rename CANVAS_NAME\n");
    }
}

/** Import:
 *  Imports an image from a file into a new canvas.
 */
void ImgineContext::execute_import(vector<string> params)
{
    if (params.size() > 1) {
        string file_name = params.at(1);
        int cv_flag = -1; // default: load image as is, incl. alpha channel
        if (params.size() > 2) {
            int channels = 0;
            stringstream(params.at(2)) >> channels;
            if (channels == 4) {
                cv_flag = -1; // <0 return the loaded image as is, incl. alpha
            } else if (channels == 3) {
                cv_flag = 1; // >0 return a 3-channel color image
            } else if (channels == 1) {
                cv_flag = 0; // =0 return a grayscale image
            } else {
                err("Invalid parameter(s).\n");
                return;
            }
        }

        new_canvas();
        *active_canvas->current->mat = imread(file_name, cv_flag);
        if (active_canvas->current->mat->data) {
            int rows = active_canvas->current->mat->rows;
            int cols = active_canvas->current->mat->cols;
            int cv_type = active_canvas->current->mat->type();
            active_canvas->current->roi = Rect2d(0, 0, cols, rows);
            // TODO: rename canvas
            active_canvas->rows = rows;
            active_canvas->cols = cols;
            active_canvas->cv_type = cv_type;
            cout << "  Imported file:\t" << file_name << endl;

        } else {
            canvases.remove(active_canvas);
            delete active_canvas;
            active_canvas = nullptr;
            err("Import failed.\n");
        }
    } else {
        warn("? :import FILE_NAME [CHANNELS]\n");
    }
}

/** Export:
 *  Exports the image from the active canvas to a file.
 */
void ImgineContext::execute_export(vector<string> params)
{
    if (params.size() > 1) {
        string file_name = params.at(1);
        vector<int> cv_params = {};
        if (params.size() > 2) {
            int param_key, param_val;
            const char *temp = params.at(2).c_str();
            try {
                if (boost::starts_with(temp, "JPEG_QUALITY=")) {
                    // 0 to 100 (default: 95)
                    param_key = CV_IMWRITE_JPEG_QUALITY;
                    param_val = boost::lexical_cast<int>(temp + 13);
                } else if (boost::starts_with(temp, "WEBP_QUALITY=")) {
                    // 1 to 100 (default: 100)
                    param_key = CV_IMWRITE_WEBP_QUALITY;
                    param_val = boost::lexical_cast<int>(temp + 13);
                } else if (boost::starts_with(temp, "PNG_COMPRESSION=")) {
                    // 0 to 9 (default: 3)
                    param_key = CV_IMWRITE_PNG_COMPRESSION;
                    param_val = boost::lexical_cast<int>(temp + 16);
                } else if (boost::starts_with(temp, "PXM_BINARY=")) {
                    // 0 or 1 (default: 1)
                    param_key = CV_IMWRITE_PXM_BINARY;
                    param_val = boost::lexical_cast<int>(temp + 11);
                } else {
                    err("Invalid subparameter(s).\n");
                    return;
                }
                cv_params.push_back(param_key);
                cv_params.push_back(param_val);
            } catch (boost::bad_lexical_cast) {
                err("Invalid subparameter(s).\n");
                return;
            }
        }

        if (active_canvas) {
            try {
                imwrite(file_name, *(active_canvas->current->mat), cv_params);
                cout << "  Exported file:\t" << file_name << endl;
            } catch (exception &e) {
                err("Export failed:\n%s", e.what());
            }
        } else {
            err("No active canvas.\n");
        }
    } else {
        warn("? :export FILE_NAME "
             "[JPEG_QUALITY=<int> | WEBP_QUALITY=<int> |"
             " PNG_COMPRESSION=<int> | PXM_BINARY=<int>]\n");
    }
}

/** Properties:
 *  Prints the image properties of the canvas.
 */
void ImgineContext::execute_properties(vector<string> params)
{
    if (params.size() > 1) {
        for (int i = 1; i < params.size(); i++) {
            string canvas_name = params.at(i);
            Canvas *target_canvas;
            target_canvas = get_canvas_by_name(canvas_name);
            if (target_canvas) {
                for (string &line : show_properties(target_canvas))
                    cout << line << endl;
            } else {
                err("Canvas not found: %s\n", canvas_name.c_str());
            }
        }
    } else if (active_canvas) {
        for (string &line : show_properties(active_canvas))
            cout << line << endl;
    } else {
        err("No active canvas.\n");
    }
}

/** ROI:
 *  Prints the selected ROI (Region of Interest) of the canvas.
 */
void ImgineContext::execute_roi(vector<string> params)
{
    if (params.size() > 1) {
        for (int i = 1; i < params.size(); i++) {
            string canvas_name = params.at(i);
            Canvas *target_canvas;
            target_canvas = get_canvas_by_name(canvas_name);
            if (target_canvas) {
                cout << "  Current ROI:\t" << target_canvas->current->roi
                     << endl;
            } else {
                err("Canvas not found: %s\n", canvas_name.c_str());
            }
        }
    } else if (active_canvas) {
        cout << "  Current ROI:\t" << active_canvas->current->roi
             << endl;
    } else {
        err("No active canvas.\n");
    }
}

/** Dump:
 *  Prints the data matrix of the canvas.
 */
void ImgineContext::execute_dump(vector<string> params)
{
    if (params.size() > 1) {
        for (int i = 1; i < params.size(); i++) {
            string canvas_name = params.at(i);
            Canvas *target_canvas;
            target_canvas = get_canvas_by_name(canvas_name);
            if (target_canvas) {
                cout << format(*(target_canvas->current->mat),
                               Formatter::FMT_PYTHON) << endl;
            } else {
                err("Canvas not found: %s\n", canvas_name.c_str());
            }
        }
    } else if (active_canvas) {
        cout << format(*(active_canvas->current->mat),
                       Formatter::FMT_PYTHON) << endl;
    } else {
        err("No active canvas.\n");
    }
}

/** Dump ROI:
 *  Prints the data matrix of the selected ROI of the canvas.
 */
void ImgineContext::execute_dump_roi(vector<string> params)
{
    if (params.size() > 1) {
        for (int i = 1; i < params.size(); i++) {
            string canvas_name = params.at(i);
            Canvas *target_canvas;
            target_canvas = get_canvas_by_name(canvas_name);
            if (target_canvas) {
                Mat roi(*(target_canvas->current->mat),
                        target_canvas->current->roi);
                cout << format(roi, Formatter::FMT_PYTHON) << endl;
            } else {
                err("Canvas not found: %s\n", canvas_name.c_str());
            }
        }
    } else if (active_canvas) {
        Mat roi(*(active_canvas->current->mat),
                active_canvas->current->roi);
        cout << format(roi, Formatter::FMT_PYTHON) << endl;
    } else {
        err("No active canvas.\n");
    }
}

/** Statistics:
 *  Prints some basic statistics of the selected ROI of the canvas.
 */
void ImgineContext::execute_statistics(vector<string> params)
{
    if (params.size() > 1) {
        for (int i = 1; i < params.size(); i++) {
            string canvas_name = params.at(i);
            Canvas *target_canvas;
            target_canvas = get_canvas_by_name(canvas_name);
            if (target_canvas) {
                cout << "  Current ROI:\t" << target_canvas->current->roi
                     << endl;

                Mat roi(*(target_canvas->current->mat),
                        target_canvas->current->roi);
                for (string &line : show_statistics(&roi))
                    cout << line << endl;
            } else {
                err("Canvas not found: %s\n", canvas_name.c_str());
            }
        }
    } else if (active_canvas) {
        cout << "  Current ROI:\t" << active_canvas->current->roi
             << endl;

        Mat roi(*(active_canvas->current->mat),
                active_canvas->current->roi);
        for (string &line : show_statistics(&roi))
            cout << line << endl;
    } else {
        err("No active canvas.\n");
    }
}

/** Show:
 *  Opens a HighGUI window displaying the image of the canvas.
 */
void ImgineContext::execute_show(vector<string> params)
{
    // No multiple windows -- buggy!
    destroyAllWindows();

    if (params.size() > 1) {
        for (int i = 1; i < params.size(); i++) {
            string canvas_name = params.at(i);
            Canvas *target_canvas;
            target_canvas = get_canvas_by_name(canvas_name);
            if (target_canvas) {
                // FIXME: resizable window using CV_WINDOW_NORMAL
                namedWindow(target_canvas->name, WINDOW_AUTOSIZE);
                imshow(target_canvas->name, *(target_canvas->current->mat));
            } else {
                err("Canvas not found: %s\n", canvas_name.c_str());
            }
        }

        if (!state.is_gui_on) {
            state.is_gui_on = true;
            threads.push_back(thread(wait_key_press, this));
        }
    } else if (active_canvas) {
        // FIXME: resizable window using CV_WINDOW_NORMAL
        namedWindow(active_canvas->name, WINDOW_AUTOSIZE);
        imshow(active_canvas->name, *(active_canvas->current->mat));

        if (!state.is_gui_on) {
            state.is_gui_on = true;
            threads.push_back(thread(wait_key_press, this));
        }
    } else {
        err("No active canvas.\n");
    }
}

/** Histogram:
 *  Displays the histogram of the selected ROI of the canvas.
 */
void ImgineContext::execute_histogram(vector<string> params)
{

    if (params.size() > 1) {
        for (int i = 1; i < params.size(); i++) {
            string canvas_name = params.at(i);
            Canvas *target_canvas;
            target_canvas = get_canvas_by_name(canvas_name);
            if (target_canvas) {
                Mat roi(*(target_canvas->current->mat),
                        target_canvas->current->roi);
                Mat hist_image = draw_histogram(&roi);

                // FIXME: resizable window using CV_WINDOW_NORMAL
                imshow(get_histogram_name(target_canvas->name), hist_image);

            } else {
                err("Canvas not found: %s\n", canvas_name.c_str());
            }
        }

        if (!state.is_gui_on) {
            state.is_gui_on = true;
            threads.push_back(thread(wait_key_press, this));
        }
    } else if (active_canvas) {
        Mat roi(*(active_canvas->current->mat),
                active_canvas->current->roi);
        Mat hist_image = draw_histogram(&roi);

        // FIXME: resizable window using CV_WINDOW_NORMAL
        imshow(get_histogram_name(active_canvas->name), hist_image);

        if (!state.is_gui_on) {
            state.is_gui_on = true;
            threads.push_back(thread(wait_key_press, this));
        }
    } else {
        err("No active canvas.\n");
    }
}

/** Inspect:
 *  Opens an interactive HighGUI window displaying the image of the canvas.
 *  (Switches to it if not active.)
 */
void ImgineContext::execute_inspect(vector<string> params,
                                    bool has_histogram = false)
{
    // No multiple windows -- buggy!
    destroyAllWindows();

    if (params.size() == 2) {
        execute_switch_to(params); // switch to the canvas
    } else if (params.size() > 2) {
        warn("? :inspect CANVAS_NAME\n");
        return;
    }

    if (active_canvas) {
        // FIXME: resizable window using CV_WINDOW_NORMAL
        namedWindow(active_canvas->name, WINDOW_AUTOSIZE);
        imshow(active_canvas->name, *(active_canvas->current->mat));

        for (string &line : show_properties(active_canvas))
            cout << line << endl;

        cout << "  Current ROI:\t" << active_canvas->current->roi << endl;
        Mat roi(*(active_canvas->current->mat), active_canvas->current->roi);
        for (string &line : show_statistics(&roi))
            cout << line << endl;

        vector<string> s_pixel = show_pixel(active_canvas->current->mat,
                                            0, 0);
        for (int i = 0; i < s_pixel.size() - 1; i++)
            cout << s_pixel[i] << endl;
        cout << s_pixel.back() << flush; // color-line

        setMouseCallback(active_canvas->name, on_mouse_event, this);

        // Draw histogram if required.
        if (has_histogram) {
            state.is_histogram_enabled = true;

            Mat hist_image = draw_histogram(&roi);
            imshow(get_histogram_name(active_canvas->name), hist_image);
        } else {
            state.is_histogram_enabled = false;
        }

        state.is_gui_on = true;
        wait_key_press(this); // blocking

        cout << el(1) << endl; // remove color-line

    } else {
        err("No active canvas.\n");
    }
}

/** Procedure:
 */
void ImgineContext::execute_procedure(vector<string> params)
{
    if (params.size() > 1) {
        string scmd = params.at(1);
        Mat result;

        if (scmd == "grayscale") {
            if (params.size() > 2) {
                Canvas *src_canvas = get_canvas_by_name(params.at(2));

                if (src_canvas) {
                    result = algo_grayscale(src_canvas);
                } else {
                    err("Canvas not found.\n");
                    return;
                }
            } else {
                warn("? :procedure grayscale SRC_CANVAS\n");
                return;
            }

        } else if (scmd == "equalize_hist") {
            if (params.size() > 2) {
                Canvas *src_canvas = get_canvas_by_name(params.at(2));
                Colorspace space = CIELAB;
                if (params.size() > 3)
                    try {
                        space = COLORSPACE_STRINGS.at(params.at(3));
                    } catch (const std::out_of_range &e) {
                        err("Unknown colorspace.\n");
                        return;
                    }

                if (src_canvas) {
                    result = algo_equalize_hist(src_canvas, space);
                } else {
                    err("Canvas not found.\n");
                    return;
                }
            } else {
                warn("? :procedure equalize_hist SRC_CANVAS [COLORSPACE]\n");
                return;
            }

        } else if (scmd == "color_transfer") {
            if (params.size() > 3) {
                Canvas *src_canvas = get_canvas_by_name(params.at(2));
                Canvas *ref_canvas = get_canvas_by_name(params.at(3));
                Colorspace space = Ruderman_lab;
                if (params.size() > 4)
                    try {
                        space = COLORSPACE_STRINGS.at(params.at(4));
                    } catch (const std::out_of_range &e) {
                        err("Unknown colorspace.\n");
                        return;
                    }

                if (src_canvas && ref_canvas) {
                    result = algo_color_transfer(src_canvas, ref_canvas, space);
                } else {
                    err("Canvas not found.\n");
                    return;
                }
            } else {
                warn("? :procedure color_transfer SRC_CANVAS REF_CANVAS [COLORSPACE]\n");
                return;
            }

        } else {
            // TODO: more subcommands
            err("Unknown subcommand.\n");
            return;
        }

        // put result into a new canvas
        new_canvas();
        *active_canvas->current->mat = result;
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

        } else {
            canvases.remove(active_canvas);
            delete active_canvas;
            active_canvas = nullptr;
            err("Import failed.\n");
        }
    } else {
        warn("? :procedure ALGORITHM [PARAMS]\n");
    }
}

} // namespace img_core
