#ifndef VKWRAPPER_LIBRARY_HPP
#define VKWRAPPER_LIBRARY_HPP

#include "SymbolTable.hpp"
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
  /**
   *
   *  Here is used a messy hack to pass pointer of the function that
   *  must be compiled by user of this library. The matter is that
   *  library does not define a platform specific vulkan macros
   *  (e.g. VK_USE_PLATFORM_XLIB_KHR) itself. This enables cross-platform
   *  usage of this library and do not have a copy for each window platform.
   *  This function purpose is described below.
   *
   *
   *  Do not modify first constructor argument!
   *
   */

  using PFN_m_fill_extension_map = void (*)();
  Library(PFN_m_fill_extension_map p = &m_fill_extension_map);

  ~Library();

  PFN_vkCreateInstance vkCreateInstance;
  PFN_vkEnumerateInstanceExtensionProperties
      vkEnumerateInstanceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
  PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;

  ApiVersion instanceAPIVersion() const;

private:
  /**
   *
   * This function fills extension table. There are some platform-specific
   * extensions.
   *
   */
  static void m_fill_extension_map() {
#define VKW_INSTANCE_MAP_ENTRY(X, Y) m_instanceExtInitializers[X] = Y;
#define VKW_DEVICE_MAP_ENTRY(X, Y) m_deviceExtInitializers[X] = Y;
#define VKW_DUMP_EXTENSION_INITIALIZERS_MAP_DEFINITION
#include "SymbolTable.inc"
#undef VKW_DEVICE_MAP_ENTRY
#undef VKW_INSTANCE_MAP_ENTRY
#undef VKW_DUMP_EXTENSION_INITIALIZERS_MAP_DEFINITION
  }
  std::unique_ptr<DynamicLoader> m_loader;
};
} // namespace vkw
#endif // VKWRAPPER_LIBRARY_HPP
