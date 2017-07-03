
#include "img_core.hpp"
#include "util_term.hpp"

#include <opencv2/opencv.hpp>

#include <cstdarg>

using namespace cv;

using std::cerr;
using std::cout;
using std::endl;
using std::exception;
using std::get;
using std::list;
using std::string;
using std::stringstream;
using std::vector;

namespace img_core {

/** Constructor of CanvasState. (given matrix size and type)
 */
CanvasState::CanvasState(int rows, int cols, int cv_type)
{
    // TODO: id
    this->mat = new Mat(rows, cols, cv_type);
}

/** Constructor of CanvasState.
 */
CanvasState::CanvasState()
{
    // TODO: id
    this->mat = new Mat();
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
    for (auto &canvas : canvases) {
        delete canvas;
    }
}

/** Colored printf for debugging-only log message.
 */
void ImgineContext::debug(const char *fmt, ...)
{
    if (!config.verbosity) return;

    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        util_term::log::vfinfo(fmt, args);
    else
        util_term::log::vfecho(fmt, args);
    va_end(args);
}

/** Colored printf for warning log message.
 */
void ImgineContext::warn(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        util_term::log::vfwarn(fmt, args);
    else
        util_term::log::vfecho(fmt, args);
    va_end(args);
}

/** Colored printf for error log message.
 */
void ImgineContext::err(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        util_term::log::vferr(fmt, args);
    else
        util_term::log::vfecho(fmt, args);
    va_end(args);
}

/** Colored printf for fatal error log message. (exit at once)
 */
void ImgineContext::wtf(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (config.is_console_ansi)
        util_term::log::vferr(fmt, args);
    else
        util_term::log::vfecho(fmt, args);
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

    } else if (cmd == ":rename" || cmd == ":ren") {
        execute_rename(params);

    } else if (cmd == ":switch_to" || cmd == ":to") {
        execute_switch_to(params);

    } else if (cmd == ":new" || cmd == ":n") {
        execute_new(params);

    } else if (cmd == ":delete" || cmd == ":del") {
        execute_delete(params);

    } else if (cmd == ":properties" || cmd == ":prop" || cmd == ":p") {
        execute_properties(params);

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
        err("Incorrect number of parameters.\n");
    }
}

/** Switch To (Canvas):
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
        err("Incorrect number of parameters.\n");
    }
}

/** New (Canvas):
 */
void ImgineContext::execute_new(vector<string> params)
{
    int cols = 0, rows = 0;
    int channels = 1, cv_type = CV_8UC1;

    if (params.size() == 1) {
        new_canvas();

    } else if (params.size() == 2) {
        stringstream(params.at(1)) >> cols;
        if (cols)
            new_canvas(cols, cols, cv_type);
        else
            err("Incorrect parameter(s).\n");

    } else if (params.size() == 3) {
        stringstream(params.at(1)) >> cols;
        stringstream(params.at(2)) >> rows;
        if (cols && rows)
            new_canvas(rows, cols, cv_type);
        else
            err("Incorrect parameter(s).\n");

    } else if (params.size() == 4) {
        stringstream(params.at(1)) >> cols;
        stringstream(params.at(2)) >> rows;
        stringstream(params.at(3)) >> channels;
        if (cols && rows && 1 <= channels && channels <= 4) {
            cv_type = CV_8UC(channels);
            new_canvas(rows, cols, cv_type);
        } else {
            err("Incorrect parameter(s).\n");
        }
    } else {
        err("Incorrect number of parameters.\n");
    }
}

/** Delete (Canvas):
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
        err("Incorrect number of parameters.\n");
    }
}

/** Rename (Canvas):
 */
void ImgineContext::execute_rename(vector<string> params)
{
    if (params.size() == 2) {
        string canvas_name = params.at(1);

        if (active_canvas) {
            active_canvas->name = canvas_name;
            cout << "  Canvas name:\t" << canvas_name << endl;
        } else {
            err("No active canvas.\n");
        }
    } else {
        err("Incorrect number of parameters.\n");
    }
}

/** (Canvas) Properties:
 */
void ImgineContext::execute_properties(vector<string> params)
{
    if (active_canvas) {
        cout << "  Canvas name:\t" << active_canvas->name << endl;
        cout << "  Canvas size:\t[" << active_canvas->cols << " x "
             << active_canvas->rows << "]" << endl;
        int channels = get<0>(IMG_CV_TYPES.at(active_canvas->cv_type));
        int depth = get<1>(IMG_CV_TYPES.at(active_canvas->cv_type));
        cout << "  Canvas type:\t" << channels << " channels x "
             << depth << " bits / px" << endl;

    } else {
        err("No active canvas.\n");
    }
}

/** Create a new canvas. (given matrix size and type)
 */
void ImgineContext::new_canvas(int rows, int cols, int cv_type)
{
    ++canvas_counter;
    string id = "Canvas_" + std::to_string(canvas_counter);
    this->active_canvas = new Canvas(id, rows, cols, cv_type);
    this->canvases.push_back(this->active_canvas);
}

/** Create a new canvas.
 */
void ImgineContext::new_canvas()
{
    ++canvas_counter;
    string id = "Canvas_" + std::to_string(canvas_counter);
    this->active_canvas = new Canvas(id);
    this->canvases.push_back(this->active_canvas);
}

} // namespace img_core
