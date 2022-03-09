#ifndef VKRENDERER_DEVICE_HPP
#define VKRENDERER_DEVICE_HPP

#include "Exception.hpp"
#include "Library.hpp"
#include "PhysicalDevice.hpp"
#include "SymbolTable.hpp"
#include "vma/vk_mem_alloc.h"
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Instance;
class BufferBase;
class Queue;

class Device {
public:
  Instance const &getParent() const { return m_parent; };

  Device(Device const &another) = delete;
  Device const &operator=(Device const &another) = delete;
  Device(Device &&another);
  Device &operator=(Device &&another) = delete;

  Device(Instance &parent, PhysicalDevice phDevice);
  virtual ~Device();

  bool supportsPresenting() const {
    return m_ph_device.extensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  std::unique_ptr<BufferBase>
  createBuffer(VmaAllocationCreateInfo const &allocCreateInfo,
               VkBufferCreateInfo const &createInfo);

  std::shared_ptr<Queue> getQueue(uint32_t queueFamilyIndex,
                                  uint32_t queueIndex);
  std::shared_ptr<Queue> getGraphicsQueue() {
    return getQueue(queueFamilyIndices.graphics, 0);
  };
  std::shared_ptr<Queue> getTransferQueue() {
    return getQueue(queueFamilyIndices.transfer, 0);
  };
  std::shared_ptr<Queue> getComputeQueue() {
    return getQueue(queueFamilyIndices.compute, 0);
  };

  operator VkDevice() const { return m_device; }

  VmaAllocator getAllocator() const { return m_allocator; }

  PhysicalDevice const &physicalDevice() const { return m_ph_device; }

  template <uint32_t major, uint32_t minor>
  DeviceCore<major, minor> core() const {
    if (m_apiVer < ApiVersion{major, minor, 0})
      throw Error{"Cannot use core " + std::to_string(major) + "." +
                  std::to_string(minor) +
                  " vulkan symbols. Version loaded: " + std::string(m_apiVer)};
    return *static_cast<DeviceCore<major, minor> const *>(
        m_coreDeviceSymbols.get());
  }

  DeviceExtensionBase const *extension(std::string const &extName) const {
    if (!m_extensions.contains(extName))
      return nullptr;
    return m_extensions.at(extName).get();
  }

  void waitIdle();

private:
  Instance &m_parent;
  VkDevice m_device;

  VmaAllocator m_allocator;

  PhysicalDevice m_ph_device;

  struct {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
  } queueFamilyIndices;

  std::vector<std::pair<uint32_t, uint32_t>> m_available_queues;

  std::map<std::pair<uint32_t, uint32_t>, std::shared_ptr<Queue>> m_queues;

  std::unique_ptr<DeviceCore<1, 0>> m_coreDeviceSymbols;
  std::unordered_map<std::string, std::unique_ptr<DeviceExtensionBase>>
      m_extensions;
  ApiVersion m_apiVer;
};
} // namespace vkw
#endif // VKRENDERER_DEVICE_HPP
