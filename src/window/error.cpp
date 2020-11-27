#include "window/error.hpp"

#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

namespace bty {

void window_error(int error_code, const char *description)
{
    (void)error_code;
    spdlog::error("glfw: {}", description);
}

}    // namespace bty
