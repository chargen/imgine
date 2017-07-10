
#ifndef _IMG_CORE_HPP
#define _IMG_CORE_HPP

#include "util_color.hpp"

#include <opencv2/opencv.hpp>

#include <thread>

using namespace cv;
using namespace util_color;

using std::list;
using std::string;
using std::thread;
using std::vector;

namespace img_core {

const std::unordered_map< int, std::tuple<int, int, string> >
IMG_CV_TYPES = {
    {CV_8UC1, {1, 8, "uchar"}},
    {CV_8UC2, {2, 8, "uchar"}},
    {CV_8UC3, {3, 8, "uchar"}},
    {CV_8UC4, {4, 8, "uchar"}}
};

/** CanvasState maintains the visual state of a canvas, including its image
 *  matrix.
 */
class CanvasState {

public:
    CanvasState(int, int, int);
    CanvasState();
    ~CanvasState();

    string id;
    Mat *mat = nullptr;
    Rect2d roi;

};

/** Canvas maintains the working session of a canvas, including its historic
 *  states.
 */
class Canvas {

public:
    Canvas(string, int, int, int);
    Canvas(string);
    ~Canvas();

    string id, name;
    int rows, cols, cv_type;
    CanvasState *current = nullptr;
    list<CanvasState *> history = {};

};

/** ImgineContext is a singleton that maintains all canvases in the workspace.
 */
class ImgineContext {

public:
    static ImgineContext& singleton();
    ~ImgineContext();

    struct {
        bool is_console_ansi = false;
        bool is_console_truecolor = false;
        int console_columns = 80;
        int verbosity = 0;
    } config;
    Canvas *active_canvas = nullptr;
    list<Canvas *> canvases = {};

    void new_canvas(int, int, int);
    void new_canvas();
    Canvas *get_canvas_by_name(string);

    void debug(const char *, ...);
    void warn(const char *, ...);
    void err(const char *, ...);
    void wtf(const char *, ...);

    void execute(vector<string>);

private:
    // (not to be implemented)
    ImgineContext() {}
    ImgineContext(ImgineContext const&) = delete;
    ImgineContext& operator=(ImgineContext const&) = delete;

    int canvas_counter = 0;
    list<thread> threads = {};

    // callback handler functions
    static void onMouseInspection(int, int, int, int, void *);

    // command executor functions
    void execute_status(vector<string>);
    void execute_list(vector<string>);
    void execute_switch_to(vector<string>);
    void execute_new(vector<string>);
    void execute_delete(vector<string>);
    void execute_rename(vector<string>);
    void execute_properties(vector<string>);
    void execute_dump(vector<string>);
    void execute_import(vector<string>);
    void execute_export(vector<string>);
    void execute_show(vector<string>);
    void execute_inspect(vector<string>);
    void execute_select(vector<string>);
    void execute_statistics(vector<string>);
    void execute_procedure(vector<string>);

};

// experimental procedures
Mat algo_color_transfer(Canvas *, Canvas *, Colorspace);



} // namespace img_core

#endif // _IMG_CORE_HPP
