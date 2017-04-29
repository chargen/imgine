
#ifndef _IMG_CORE_HPP
#define _IMG_CORE_HPP

#include <string>
#include <vector>

namespace img_core {

class ImgineContext {

public:
    static ImgineContext& singleton();
    ~ImgineContext();

    void execute(std::string, std::vector<std::string>);

private:
    // Don't implement these:
    ImgineContext() {}
    ImgineContext(ImgineContext const&) = delete;
    ImgineContext& operator=(ImgineContext const&) = delete;

};

} // namespace img_core

#endif // _IMG_CORE_HPP
