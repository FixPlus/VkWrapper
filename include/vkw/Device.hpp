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
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Instance;
class BufferBase;
class Queue;

enum class ext;

class Device {
public:
  Instance const &getParent() const { return m_parent; };

  Device(Device const &another) = delete;
  Device const &operator=(Device const &another) = delete;
  Device(Device &&another);
  Device &operator=(Device &&another) noexcept;

  Device(Instance &parent, PhysicalDevice phDevice);
  virtual ~Device();

  std::unique_ptr<BufferBase>
  createBuffer(VmaAllocationCreateInfo const &allocCreateInfo,
               VkBufferCreateInfo const &createInfo);

  Queue const &getQueue(unsigned queueFamilyIndex, unsigned queueIndex) const {
    return *m_queues.at(queueFamilyIndex).at(queueIndex);
  }

  Queue const &anyGraphicsQueue() const;

  Queue const &anyComputeQueue() const;

  Queue const &anyTransferQueue() const;

  Queue const &getSpecificQueue(QueueFamily::Type type) const;

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

  void waitIdle();

private:
  InstanceRef m_parent;
  VkDevice m_device;

  VmaAllocator m_allocator;

  PhysicalDevice m_ph_device;

  std::vector<std::vector<std::unique_ptr<Queue>>> m_queues;

  std::unique_ptr<DeviceCore<1, 0>> m_coreDeviceSymbols;
  std::set<ext> m_enabledExtensions;

  ApiVersion m_apiVer;
};
} // namespace vkw
#endif // VKRENDERER_DEVICE_HPP
