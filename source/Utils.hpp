#ifndef VKRENDERER_UTILS_HPP
#define VKRENDERER_UTILS_HPP

#include "Exception.hpp"
#include <string>
namespace vkw {

#define VK_CHECK_RESULT(f)                                                     \
  {                                                                            \
    VkResult res = (f);                                                        \
    if (res != VK_SUCCESS)                                                     \
      throw VulkanError(res, __FILE__, __LINE__);                              \
  }

} // namespace vkr
#endif // VKRENDERER_UTILS_HPP
