
#include "img_core.hpp"
#include "util_color.hpp"
#include "util_term.hpp"

#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>

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
    // TODO: id
    this->mat = new Mat(rows, cols, cv_type, Scalar::all(0));
    this->roi = Rect2d(0, 0, cols, rows);
}

/** Constructor of CanvasState.
 */
CanvasState::CanvasState()
{
    // TODO: id
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
    this->cv_type = CV_8UC1;
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
    for (auto &thread : threads) {
        thread.join();
    }
    for (auto &canvas : canvases) {
        delete canvas;
    }
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

/** Return a pointer to a canvas with the specified name.
 */
Canvas *ImgineContext::get_canvas_by_name(string canvas_name)
{
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

    } else if (cmd == ":properties" || cmd == ":prop" || cmd == ":p") {
        execute_properties(params);

    } else if (cmd == ":dump") {
        execute_dump(params);

    } else if (cmd == ":import" ||
               cmd == ":read" || cmd == ":r") {
        execute_import(params);

    } else if (cmd == ":export" ||
               cmd == ":write" || cmd == ":w") {
        execute_export(params);

    } else if (cmd == ":show") {
        execute_show(params);

    } else if (cmd == ":inspect" || cmd == ":i") {
        execute_inspect(params);

    } else if (cmd == ":select" || cmd == ":sel") {
        execute_select(params);

    } else if (cmd == ":statistics" || cmd == ":stat" || cmd == ":st") {
        execute_statistics(params);

    } else if (cmd == ":run") {
        execute_run(params);

    } else {
        // TODO: more commands
        err("Unknown command.\n");
    }
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
                         "* " : "  ") << canvas->name << "\t";
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
 *  Deletes a canvas.
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

        // TODO: disallow duplicate names
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

/** Properties:
 *  Prints the basic information of the active canvas.
 */
void ImgineContext::execute_properties(vector<string> params)
{
    if (active_canvas) {
        cout << "  Canvas name:\t" << active_canvas->name << endl;
        cout << "  Canvas size:\t[" << active_canvas->cols << " x "
             << active_canvas->rows << "]" << endl;
        int channels = get<0>(IMG_CV_TYPES.at(active_canvas->cv_type));
        int depth = get<1>(IMG_CV_TYPES.at(active_canvas->cv_type));
        cout << "  Channels:\t" << channels << endl;
        cout << "  Color depth:\t" << depth << " bpc" << endl;

        cout << "  ROI:\t\t" << active_canvas->current->roi << endl;

    } else {
        err("No active canvas.\n");
    }
}

/** Dump:
 *  Prints the actual matrix of the active canvas.
 */
void ImgineContext::execute_dump(vector<string> params)
{
    if (active_canvas) {
        cout << format(*(active_canvas->current->mat), Formatter::FMT_PYTHON)
             << endl;
    } else {
        err("No active canvas.\n");
    }
}

/** Import:
 *  Imports an image from a file into a new canvas.
 */
void ImgineContext::execute_import(vector<string> params)
{
    if (params.size() >= 2) {
        string file_name = params.at(1);
        int cv_flag = -1; // default: load image as is, incl. alpha channel
        if (params.size() >= 3) {
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
    if (params.size() >= 2) {
        string file_name = params.at(1);
        vector<int> cv_params;
        if (params.size() >= 3) {
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

/** Show:
 *  Open a HighGUI window displaying the image of the active canvas.
 */
void ImgineContext::execute_show(vector<string> params)
{
    if (active_canvas) {
        // FIXME: two HighGUI windows can't coexist -- why?
        destroyAllWindows();
        // FIXME: resizable window using CV_WINDOW_NORMAL
        namedWindow(active_canvas->name, WINDOW_AUTOSIZE);
        imshow(active_canvas->name, *(active_canvas->current->mat));
        threads.push_back(thread(waitKey, 0));

    } else {
        err("No active canvas.\n");
    }
}

/** Inspect:
 *  Open a HighGUI window displaying the image of the active canvas.
 *  (Mouse inspection enabled)
 */
void ImgineContext::execute_inspect(vector<string> params)
{
    if (active_canvas) {
        // FIXME: two HighGUI windows can't coexist -- why?
        destroyAllWindows();
        // FIXME: resizable window using CV_WINDOW_NORMAL
        namedWindow(active_canvas->name, WINDOW_AUTOSIZE);
        imshow(active_canvas->name, *(active_canvas->current->mat));
        setMouseCallback(active_canvas->name, onMouseInspection, this);
        execute_properties(params);
        cout << string(5, '\n') << flush;
        waitKey(0);
        cout << EL(1) << endl;

    } else {
        err("No active canvas.\n");
    }
}

/** Mouse callback handler for inspection.
 */
void ImgineContext::onMouseInspection(int ev, int x, int y, int flags, void *context)
{
    // TODO: EVENT_MOUSEMOVE EVENT_LBUTTONDOWN EVENT_RBUTTONDOWN EVENT_MBUTTONDOWN
    ImgineContext *image_context = (ImgineContext *)context;
    Mat *mat = image_context->active_canvas->current->mat;

    cout << CPL(4);

    cout << "  Pixel:\t"
         << "(" << x << ", " << y << ")" << string(6, ' ') << endl;

    unsigned char r, g, b, a = 255;
    if (mat->type() == CV_8UC1) {
        Vec<uchar, 1> pixel = mat->at<uchar>(y, x);
        b = g = r = pixel.val[0];

        cout << "  Value (gray):\t"
             << pixel << string(12, ' ') << endl;

    } else if (mat->type() == CV_8UC3) {
        Vec3b pixel = mat->at<Vec3b>(y, x);
        b = pixel.val[0];
        g = pixel.val[1];
        r = pixel.val[2];

        cout << "  Value (BGR):\t"
             << pixel << string(12, ' ') << endl;

    } else if (mat->type() == CV_8UC4) {
        Vec4b pixel = mat->at<Vec4b>(y, x);
        b = pixel.val[0];
        g = pixel.val[1];
        r = pixel.val[2];
        a = pixel.val[3];

        cout << "  Value (BGRA):\t"
             << pixel << string(12, ' ') << endl;

    }
    cout << "  RGB hex:\t" << rgb_to_hex(r, g, b) << endl;
    cout << "  Opacity:\t" << alpha_to_opacity(a) << endl;

    if (image_context->config.is_console_truecolor)
        cout << sgr_background_rgb(r, g, b,
                                   string(image_context->config.console_columns, ' '))
             << flush;
}

/** Select:
 *  Open a HighGUI window for selecting ROI in the active canvas.
 */
void ImgineContext::execute_select(vector<string> params)
{
    if (active_canvas) {
        bool showCrosshair = false;
        bool fromCenter = false;
        active_canvas->current->roi =
            selectROI(active_canvas->name, *(active_canvas->current->mat),
                      showCrosshair, fromCenter);
        destroyWindow(active_canvas->name);
        cout << "  ROI:\t\t" << active_canvas->current->roi << endl;
        // TODO: ROI out of bounds

    } else {
        err("No active canvas.\n");
    }
}

/** Statistics:
 */
void ImgineContext::execute_statistics(vector<string> params)
{
    if (active_canvas) {
        Mat roi(*(active_canvas->current->mat), active_canvas->current->roi);
        cout << "  ROI:\t\t" << active_canvas->current->roi << endl;

        Scalar roi_mean, roi_stddev;
        meanStdDev(roi, roi_mean, roi_stddev);
        cout << "  (BGRA)" << endl;
        cout << "    Mean:\t" << roi_mean << endl;
        cout << "    Std dev:\t" << roi_stddev << endl;

    } else {
        err("No active canvas.\n");
    }
}

/** Run:
 */
void ImgineContext::execute_run(vector<string> params)
{
    if (params.size() >= 2) {
        string scmd = params.at(1);

        if (scmd == "color_transfer") {
            if (params.size() >= 4) {
                Canvas *src_canvas = get_canvas_by_name(params.at(2));
                Canvas *ref_canvas = get_canvas_by_name(params.at(3));

                if (src_canvas && ref_canvas) {
                    algo_color_transfer(src_canvas, ref_canvas);
                } else {
                    err("Canvas not found.\n");
                }
            } else {
                warn("? :run color_transfer SRC_CANVAS REF_CANVAS\n");
            }
        } else {
            // TODO: more commands
            err("Unknown subcommand.\n");
        }
    } else {
        warn("? :run ALGORITHM [PARAMS]\n");
    }
}



} // namespace img_core
