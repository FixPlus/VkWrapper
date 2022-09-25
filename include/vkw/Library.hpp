#ifndef VKWRAPPER_LIBRARY_HPP
#define VKWRAPPER_LIBRARY_HPP

#include "SymbolTable.hpp"
#include <memory>
#include <vector>
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

enum class ext;
enum class layer;

class Library final {
public:
  Library();

  ~Library();

  bool hasLayer(layer layerName) const;

  VkLayerProperties layerProperties(layer layerName) const;

  bool hasInstanceExtension(ext extensionId) const;

  VkExtensionProperties instanceExtensionProperties(ext extensionId) const;

  PFN_vkCreateInstance vkCreateInstance;
  PFN_vkEnumerateInstanceExtensionProperties
      vkEnumerateInstanceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
  PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;

  ApiVersion instanceAPIVersion() const;

  static const char *ExtensionName(ext id);

  static ext ExtensionId(std::string_view extensionName);

  static const char *LayerName(layer id);

  static layer LayerId(std::string_view extensionName);

private:
  std::vector<VkLayerProperties> m_layer_properties;
  std::vector<VkExtensionProperties> m_instance_extension_properties;
  std::unique_ptr<DynamicLoader> m_loader;
};
} // namespace vkw
#endif // VKWRAPPER_LIBRARY_HPP
