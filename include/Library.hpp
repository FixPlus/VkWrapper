#ifndef VKWRAPPER_LIBRARY_HPP
#define VKWRAPPER_LIBRARY_HPP

#include <memory>
#include <vulkan/vulkan.h>
namespace vkw {

class DynamicLoader;

struct ApiVersion {
  uint32_t major;
  uint32_t minor;
  uint32_t revision;

  ApiVersion(uint32_t maj, uint32_t min, uint32_t rev)
      : major(maj), minor(min), revision(rev) {}
  ApiVersion(uint32_t encoded);
  ApiVersion() = default;

  operator std::string() const {
    return std::to_string(major) + "." + std::to_string(minor) + "." +
           std::to_string(revision);
  }
  auto operator<=>(ApiVersion const &another) const = default;
};

class Library final {
public:
  Library();

  ~Library() = default;

#define VKW_VK_GLOBAL_FUNCTION_1_0(X) PFN_vk##X vk##X;
#define VKW_VK_GLOBAL_FUNCTION_1_1(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"
  ApiVersion instanceAPIVersion() const;

private:
  std::unique_ptr<DynamicLoader> m_loader;
};
} // namespace vkw
#endif // VKWRAPPER_LIBRARY_HPP
