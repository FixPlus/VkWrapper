#ifndef VKRENDERER_INSTANCE_HPP
#define VKRENDERER_INSTANCE_HPP

#include <vkw/Library.hpp>

#include <boost/container/small_vector.hpp>
#include <functional>
#include <set>
#include <unordered_map>
#include <vector>

namespace vkw {

class DynamicLoader;

enum class ext;
enum class layer;

class PhysicalDevice;

class InstanceCreateInfo {
public:
  void requestApiVersion(ApiVersion version) noexcept {
    m_apiVersion = version;
  }
  void requestExtension(ext ext) noexcept(ExceptionsDisabled) {
    m_reqExtensions.push_back(ext);
  }

  void requestLayer(layer layer) noexcept(ExceptionsDisabled) {
    m_reqLayers.push_back(layer);
  }

  auto requestedExtensionsBegin() const noexcept {
    return m_reqExtensions.begin();
  }

  auto requestedExtensionsEnd() const noexcept { return m_reqExtensions.end(); }

  auto requestedLayersBegin() const noexcept { return m_reqLayers.begin(); }

  auto requestedLayersEnd() const noexcept { return m_reqLayers.end(); }

  ApiVersion requestedApiVersion() const noexcept { return m_apiVersion; }

private:
  std::vector<ext> m_reqExtensions;
  std::vector<layer> m_reqLayers;
  ApiVersion m_apiVersion = VK_API_VERSION_1_0;
  std::string_view m_appName;
};
class Instance : public ReferenceGuard {
public:
  Instance(Library const &library,
           InstanceCreateInfo const &createInfo) noexcept(ExceptionsDisabled);
  Instance(Instance const &another) = delete;
  Instance const &operator=(Instance const &another) = delete;
  Instance(Instance &&another) noexcept;
  Instance &operator=(Instance &&another) noexcept;

  boost::container::small_vector<std::unique_ptr<PhysicalDevice>, 3>
  enumerateAvailableDevices() const noexcept(ExceptionsDisabled);

  bool isExtensionEnabled(ext extension) const noexcept {
    return m_enabledExtensions.contains(extension);
  }

  bool isLayerEnabled(layer layer) const noexcept {
    return m_enabledLayers.contains(layer);
  }

  operator VkInstance() const noexcept { return m_instance; }

  virtual ~Instance();

  template <uint32_t major, uint32_t minor>
  InstanceCore<major, minor> const &core() const noexcept(ExceptionsDisabled) {
    if (m_apiVer < ApiVersion{major, minor, 0})
      postError(
          Error{"Cannot use core " + std::to_string(major) + "." +
                std::to_string(minor) +
                " vulkan symbols. Version loaded: " + std::string(m_apiVer)});

    auto *ptr = static_cast<InstanceCore<major, minor> const *>(
        m_coreInstanceSymbols.get());
    return *ptr;
  }

  auto &hostAllocator() const noexcept {
    return m_vulkanLib.get().hostAllocator();
  }

  Library const &parent() const noexcept { return m_vulkanLib; }

private:
  VkInstance m_instance{};

  std::set<ext> m_enabledExtensions;
  std::set<layer> m_enabledLayers;

  StrongReference<Library const> m_vulkanLib;
  std::unique_ptr<InstanceCore<1, 0>> m_coreInstanceSymbols{};
  ApiVersion m_apiVer;
};

} // namespace vkw
#endif // VKRENDERER_INSTANCE_HPP
