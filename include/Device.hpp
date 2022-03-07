#ifndef VKRENDERER_DEVICE_HPP
#define VKRENDERER_DEVICE_HPP

#include "Exception.hpp"
#include "Library.hpp"
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

struct DeviceInfo {
  uint32_t id;
  std::string name;
  VkPhysicalDevice physicalDevice;
};

class Instance;
class BufferBase;
class Queue;

class Device {
public:
  DeviceInfo const &getInfo() const { return m_info; };

  Instance const &getParent() const { return m_parent; };

  Device(Device const &another) = delete;
  Device const &operator=(Device const &another) = delete;
  Device(Device &&another);
  Device &operator=(Device &&another) = delete;

  Device(Instance &parent, uint32_t id);
  virtual ~Device();

  bool extensionSupported(std::string const &extension) const {
    return (std::find(m_supportedExtensions.begin(),
                      m_supportedExtensions.end(),
                      extension) != m_supportedExtensions.end());
  }

  bool supportsPresenting() const {
    return extensionSupported(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
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

  VkPhysicalDevice getPhysicalDevice() const { return ph_device; }
  VmaAllocator getAllocator() const { return m_allocator; }

  std::vector<VkQueueFamilyProperties> const &getQueueFamilyProperties() const {
    return m_queueFamilyProperties;
  }

  DeviceCore<1, 0> const &core_1_0() const { return *m_coreDeviceSymbols; }

  DeviceCore<1, 1> const &core_1_1() const {
    if (m_apiVer < ApiVersion{1, 1, 0})
      throw Error{"Cannot use core 1.1 vulkan symbols. Version loaded: " +
                  std::string(m_apiVer)};
    return *static_cast<DeviceCore<1, 1> const *>(m_coreDeviceSymbols.get());
  }

  DeviceExtensionBase const *getExtension(std::string const &extName) const {
    if (!m_extensions.contains(extName))
      return nullptr;
    return m_extensions.at(extName).get();
  }

  void waitIdle();

private:
  Instance &m_parent;
  VkDevice m_device;
  DeviceInfo m_info;

  VmaAllocator m_allocator;

  VkPhysicalDevice ph_device;
  /** @brief Properties of the physical device including limits that the
   * application can check against */
  VkPhysicalDeviceProperties m_properties{};
  /** @brief Features of the physical device that an application can use to
   * check if a feature is supported */
  VkPhysicalDeviceFeatures m_features{};
  /** @brief Features that have been enabled for use on the physical device */
  VkPhysicalDeviceFeatures m_enabledFeatures{};
  /** @brief Memory types and heaps of the physical device */
  VkPhysicalDeviceMemoryProperties m_memoryProperties{};
  /** @brief Queue family properties of the physical device */
  std::vector<VkQueueFamilyProperties> m_queueFamilyProperties{};
  /** @brief List of extensions supported by the device */
  std::vector<std::string> m_supportedExtensions{};

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
