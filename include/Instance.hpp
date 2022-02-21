#ifndef VKRENDERER_INSTANCE_HPP
#define VKRENDERER_INSTANCE_HPP

#include "Device.hpp"
#include "Exception.hpp"
#include "Surface.hpp"
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <vulkan/vulkan.h>

namespace vkw {

using SurfaceCreator = std::function<VkSurfaceKHR(VkInstance)>;

class Instance {
public:
  Instance(std::vector<std::string> reqExtensions = {},
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
    return std::find(m_extensions.begin(), m_extensions.end(), extension) !=
           m_extensions.end();
  };

  std::unique_ptr<Surface> createSurface(SurfaceCreator surfaceCreator);

  operator VkInstance() const { return m_instance; }

  void checkExtensions(std::string const &ext) {
    if (!isExtensionEnabled(ext)) {
      throw ExtensionMissing(ext);
    }
  }

  template <typename... Args>
  void checkExtensions(std::string const &ext, Args... args) {
    if (!isExtensionEnabled(ext)) {
      throw ExtensionMissing(ext);
    }
    checkExtensions(args...);
  }

  virtual ~Instance();

  // ****** DEBUG *******

  void printExtensions() const {
    std::for_each(
        m_extensions.begin(), m_extensions.end(),
        [](std::string const &ext) { std::cout << ext << std::endl; });
  }

private:
  VkInstance m_instance{};

  std::vector<std::string> m_extensions{};
  // TODO: instance owing device handles is questionable
  std::unordered_map<uint32_t, std::unique_ptr<Device>> m_devices;
  bool m_validation;
};

} // namespace vkr
#endif // VKRENDERER_INSTANCE_HPP
