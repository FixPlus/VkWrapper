#ifndef VKRENDERER_INSTANCE_HPP
#define VKRENDERER_INSTANCE_HPP

#include "Device.hpp"
#include "Exception.hpp"
#include "Library.hpp"
#include "SymbolTable.hpp"
#include <functional>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>
namespace vkw {

class DynamicLoader;

enum class ext;
enum class layer;

class InstanceCreateInfo {
public:
  void requestApiVersion(ApiVersion version) { m_apiVersion = version; }
  void requestExtension(ext ext) { m_reqExtensions.push_back(ext); }

  void requestLayer(layer layer) { m_reqLayers.push_back(layer); }

  auto requestedExtensionsBegin() const { return m_reqExtensions.begin(); }

  auto requestedExtensionsEnd() const { return m_reqExtensions.end(); }

  auto requestedLayersBegin() const { return m_reqLayers.begin(); }

  auto requestedLayersEnd() const { return m_reqLayers.end(); }

  ApiVersion requestedApiVersion() const { return m_apiVersion; }

private:
  std::vector<ext> m_reqExtensions;
  std::vector<layer> m_reqLayers;
  ApiVersion m_apiVersion = VK_API_VERSION_1_0;
  std::string_view m_appName;
};
class Instance {
public:
  Instance(Library const &library, InstanceCreateInfo const &createInfo);
  Instance(Instance const &another) = delete;
  Instance const &operator=(Instance const &another) = delete;
  Instance(Instance &&another) noexcept;
  Instance &operator=(Instance &&another) noexcept;

  std::vector<PhysicalDevice> enumerateAvailableDevices() const;

  bool isExtensionEnabled(ext extension) const {
    return m_enabledExtensions.contains(extension);
  }

  bool isLayerEnabled(layer layer) const {
    return m_enabledLayers.contains(layer);
  }

  operator VkInstance() const { return m_instance; }

  virtual ~Instance();

  template <uint32_t major, uint32_t minor>
  InstanceCore<major, minor> const &core() const {
    if (m_apiVer < ApiVersion{major, minor, 0})
      throw Error{"Cannot use core " + std::to_string(major) + "." +
                  std::to_string(minor) +
                  " vulkan symbols. Version loaded: " + std::string(m_apiVer)};

    auto *ptr = static_cast<InstanceCore<major, minor> const *>(
        m_coreInstanceSymbols.get());
    return *ptr;
  }

  Library const &parent() const { return m_vulkanLib; }

private:
  VkInstance m_instance{};

  std::set<ext> m_enabledExtensions;
  std::set<layer> m_enabledLayers;

  std::reference_wrapper<Library const> m_vulkanLib;
  std::unique_ptr<InstanceCore<1, 0>> m_coreInstanceSymbols{};
  ApiVersion m_apiVer;
};

} // namespace vkw
#endif // VKRENDERER_INSTANCE_HPP
