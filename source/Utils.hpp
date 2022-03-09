#ifndef VKRENDERER_UTILS_HPP
#define VKRENDERER_UTILS_HPP

#include "vkw/Exception.hpp"
#include <string>
namespace vkw {

#define VK_CHECK_RESULT(...)                                                   \
  {                                                                            \
    VkResult res = (__VA_ARGS__);                                              \
    if (res != VK_SUCCESS)                                                     \
      throw VulkanError(res, __FILE__, __LINE__);                              \
  }

} // namespace vkw
#endif // VKRENDERER_UTILS_HPP
