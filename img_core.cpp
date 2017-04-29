
#include "img_core.hpp"

#include <opencv2/opencv.hpp>

using namespace cv;

using std::exception;
using std::string;
using std::vector;

namespace img_core {

    /** Create the singleton object of ImgineContext.
     */
    ImgineContext& ImgineContext::singleton()
    {
        static ImgineContext instance;
        return instance;
    }

    /** Destructor of ImgineContext.
     */
    ImgineContext::~ImgineContext() {
        // TODO: free the memory, etc.
    }

    /** Execute a command (with parameters).
     */
    void ImgineContext::execute(string command, vector<string> tokens)
    {

    }

} // namespace img_core
