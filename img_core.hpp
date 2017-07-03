
#ifndef _IMG_CORE_HPP
#define _IMG_CORE_HPP

#include <opencv2/opencv.hpp>

namespace img_core {

const std::unordered_map<int, std::tuple<int, int, std::string> >
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

    std::string id;
    cv::Mat *mat;

};

/** Canvas maintains the working session of a canvas, including its historic
 *  states.
 */
class Canvas {

public:
    Canvas(std::string, int, int, int);
    Canvas(std::string);
    ~Canvas();

    std::string id, name;
    int rows, cols, cv_type;
    CanvasState *current = nullptr;
    std::list<CanvasState *> history = {};

};

/** ImgineContext is a singleton that maintains all canvases in the workspace.
 */
class ImgineContext {

public:
    static ImgineContext& singleton();
    ~ImgineContext();

    struct {
        bool is_console_ansi = false;
        int verbosity = 0;
    } config;
    int canvas_counter = 0;
    Canvas *active_canvas = nullptr;
    std::list<Canvas *> canvases = {};

    void debug(const char *, ...);
    void warn(const char *, ...);
    void err(const char *, ...);
    void wtf(const char *, ...);

    void execute(std::vector<std::string>);

    void execute_status(std::vector<std::string>);
    void execute_list(std::vector<std::string>);
    void execute_rename(std::vector<std::string>);
    void execute_switch_to(std::vector<std::string>);
    void execute_new_canvas(std::vector<std::string>);
    void execute_properties(std::vector<std::string>);

    void new_canvas(int, int, int);
    void new_canvas();

private:
    // Never implement these!
    ImgineContext() {}
    ImgineContext(ImgineContext const&) = delete;
    ImgineContext& operator=(ImgineContext const&) = delete;

};

} // namespace img_core

#endif // _IMG_CORE_HPP
