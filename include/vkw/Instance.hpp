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

struct InstanceCreateInfo {
  void requestApiVersion(ApiVersion version) noexcept { apiVersion = version; }
  void requestExtension(ext ext) noexcept(ExceptionsDisabled) {
    requestedExtensions.push_back(ext);
  }

  void requestLayer(layer layer) noexcept(ExceptionsDisabled) {
    requestedLayers.push_back(layer);
  }

  std::vector<ext> requestedExtensions;
  std::vector<layer> requestedLayers;
  ApiVersion apiVersion = ApiVersion{1, 0, 0};
  std::string_view applicationName = "APITest";
  std::string_view engineName = "APITest";
  ApiVersion applicationVersion = ApiVersion{1, 0, 0};
  ApiVersion engineVersion = ApiVersion{1, 0, 0};
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

  auto &apiVersion() const noexcept { return m_apiVer; }

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
