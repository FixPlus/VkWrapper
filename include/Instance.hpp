#ifndef VKRENDERER_INSTANCE_HPP
#define VKRENDERER_INSTANCE_HPP

#include "Device.hpp"
#include "Exception.hpp"
#include "Library.hpp"
#include "Surface.hpp"
#include "SymbolTable.hpp"
#include <functional>
#include <iostream>
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

  std::vector<DeviceInfo> enumerateAvailableDevices() const;

  Device *getDevice(uint32_t id) {
    return m_devices.count(id) ? m_devices.at(id).get() : nullptr;
  }

  // TODO: don't use raw pointer here
  Device *createDevice(uint32_t id);

  void destroyDevice(uint32_t id) {
    if (getDevice(id) == nullptr)
      throw Error("No device with id provided to destroyDevice()");

    m_devices.erase(id);
  }

  bool isExtensionEnabled(std::string const &extension) {
    return m_extensions.contains(extension);
  };

  operator VkInstance() const { return m_instance; }

  virtual ~Instance();

  // ****** DEBUG *******

  void printExtensions() const {
    for (auto &ext : m_extensions) {
      std::cout << ext.first << std::endl;
    }
  }

  InstanceCore<1, 0> const &core_1_0() const { return *m_coreInstanceSymbols; }

  InstanceCore<1, 1> const &core_1_1() const {
    if (m_apiVer < ApiVersion{1, 1, 0})
      throw Error{"Cannot use core 1.1 vulkan symbols. Version loaded: " +
                  std::string(m_apiVer)};

    auto *ptr =
        static_cast<InstanceCore<1, 1> const *>(m_coreInstanceSymbols.get());
    return *ptr;
  }

  InstanceExtensionBase *getExtension(std::string const &name) const {
    if (!m_extensions.contains(name))
      return nullptr;
    return m_extensions.at(name).get();
  }

private:
  VkInstance m_instance{};

  std::unordered_map<std::string, std::unique_ptr<InstanceExtensionBase>>
      m_extensions;
  // TODO: instance owing device handles is questionable
  std::unordered_map<uint32_t, std::unique_ptr<Device>> m_devices;
  std::reference_wrapper<Library const> m_vulkanLib;
  std::unique_ptr<InstanceCore<1, 0>> m_coreInstanceSymbols{};
  ApiVersion m_apiVer;
  bool m_validation;
};

} // namespace vkw
#endif // VKRENDERER_INSTANCE_HPP
