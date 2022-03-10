#ifndef VKRENDERER_INSTANCE_HPP
#define VKRENDERER_INSTANCE_HPP

#include "Device.hpp"
#include "Exception.hpp"
#include "Library.hpp"
#include "SymbolTable.hpp"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class DynamicLoader;

class Instance {
public:
  Instance(Library const &library, std::vector<std::string> reqExtensions = {},
           bool enableValidation = true);
  Instance(Instance const &another) = delete;
  Instance const &operator=(Instance const &another) = delete;
  Instance(Instance &&another) noexcept;
  Instance &operator=(Instance &&another) noexcept;

  std::vector<PhysicalDevice> enumerateAvailableDevices() const;

  bool isExtensionEnabled(std::string const &extension) {
    return m_extensions.contains(extension);
  };

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

  InstanceExtensionBase *extension(std::string const &name) const {
    if (!m_extensions.contains(name))
      return nullptr;
    return m_extensions.at(name).get();
  }

  /** Iterators for extensions.*/

  auto extensions_begin() { return m_extensions.begin(); }

  auto extensions_end() { return m_extensions.end(); }

  auto extensions_begin() const { return m_extensions.begin(); }

  auto extensions_end() const { return m_extensions.end(); }

  using extension_iterator =
      std::unordered_map<std::string,
                         std::unique_ptr<InstanceExtensionBase>>::iterator;
  using extension_const_iterator = std::unordered_map<
      std::string, std::unique_ptr<InstanceExtensionBase>>::const_iterator;

private:
  VkInstance m_instance{};

  std::unordered_map<std::string, std::unique_ptr<InstanceExtensionBase>>
      m_extensions;
  std::reference_wrapper<Library const> m_vulkanLib;
  std::unique_ptr<InstanceCore<1, 0>> m_coreInstanceSymbols{};
  ApiVersion m_apiVer;
  bool m_validation;
};

} // namespace vkw
#endif // VKRENDERER_INSTANCE_HPP
