#include "vkw/Library.hpp"
#include "loader/DynamicLoader.hpp"
#include "vkw/Exception.hpp"
#include <cassert>

namespace vkw {

std::unordered_map<std::string, InstanceExtensionInitializerBase const *>
    m_instanceExtInitializers{};
std::unordered_map<std::string, DeviceExtensionInitializerBase const *>
    m_deviceExtInitializers{};

Library::Library(PFN_m_fill_extension_map p) {
  static constexpr const char *libName =
#ifdef _WIN32
      "vulkan-1.dll";
#elif defined __linux__
      "libvulkan.so.1";
#endif
  m_loader = std::make_unique<DynamicLoader>(libName);

  vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
      m_loader->getSymbol("vkCreateInstance"));
  vkEnumerateInstanceExtensionProperties =
      reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
          m_loader->getSymbol("vkEnumerateInstanceExtensionProperties"));
  vkEnumerateInstanceLayerProperties =
      reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
          m_loader->getSymbol("vkEnumerateInstanceLayerProperties"));
  vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      m_loader->getSymbol("vkGetInstanceProcAddr"));
  try {
    vkEnumerateInstanceVersion =
        reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
            m_loader->getSymbol("vkEnumerateInstanceVersion"));
  } catch (Error &e) {
    if (e.code() == ErrorCode::DYNAMIC_LIBRARY_SYMBOL_MISSING) {
      // it's okay, because Vulkan 1.0 doesn't have this function yet
    } else
      throw;
  }
  p(); // fill extension table
}

ApiVersion::ApiVersion(uint32_t encoded)
    : major(VK_API_VERSION_MAJOR(encoded)),
      minor(VK_API_VERSION_MINOR(encoded)),
      revision(VK_API_VERSION_PATCH(encoded)) {}
ApiVersion Library::instanceAPIVersion() const {
  if (this->vkEnumerateInstanceVersion == nullptr)
    return {1, 0, 0};

  uint32_t encoded;
  this->vkEnumerateInstanceVersion(&encoded);
  return ApiVersion{encoded};
}
Library::~Library() {}

} // namespace vkw
