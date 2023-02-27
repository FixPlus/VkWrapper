#ifndef VKRENDERER_DEVICE_HPP
#define VKRENDERER_DEVICE_HPP

#include "vkw/Exception.hpp"
#include "vkw/Library.hpp"
#include "vkw/PhysicalDevice.hpp"
#include "vkw/SymbolTable.hpp"
#include <vkw/Instance.hpp>
#include <vkw/UniqueVulkanObject.hpp>

#include "vma/vk_mem_alloc.h"

#include <boost/container/small_vector.hpp>
#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>

namespace vkw {

class Instance;
class BufferBase;
class Queue;

enum class ext;

class DeviceInfo {
public:
  explicit DeviceInfo(PhysicalDevice phDevice);

  auto &info() const { return m_createInfo; }

  PhysicalDevice const &physicalDevice() const { return m_ph_device; }

  auto &apiVersion() const { return m_apiVer; }

private:
  VkDeviceCreateInfo m_createInfo{};
  boost::container::small_vector<VkDeviceQueueCreateInfo, 3> m_queueCreateInfo;
  boost::container::small_vector<const char *, 8> m_enabledExtensionsRaw;

  PhysicalDevice m_ph_device;
  std::set<ext> m_enabledExtensions;
  ApiVersion m_apiVer;
};

class Device : public DeviceInfo, public UniqueVulkanObject<VkDevice> {
public:
  Device(Instance const &parent, PhysicalDevice phDevice);

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

  VmaAllocator getAllocator() const { return m_allocator.get(); }

  template <uint32_t major, uint32_t minor>
  DeviceCore<major, minor> core() const {
    if (apiVersion() < ApiVersion{major, minor, 0})
      throw Error{"Cannot use core " + std::to_string(major) + "." +
                  std::to_string(minor) + " vulkan symbols. Version loaded: " +
                  std::string(apiVersion())};
    return *static_cast<DeviceCore<major, minor> const *>(
        m_coreDeviceSymbols.get());
  }

  void waitIdle();

private:
  VmaAllocator m_allocatorCreateImpl();
  struct AllocatorDeleter {
    void operator()(VmaAllocator a);
  };
  std::unique_ptr<std::remove_pointer_t<VmaAllocator>, AllocatorDeleter>
      m_allocator;

  using InFamilyQueueContainerT =
      boost::container::small_vector<std::unique_ptr<Queue>, 3>;
  using FamilyContainerT =
      boost::container::small_vector<InFamilyQueueContainerT, 3>;

  FamilyContainerT m_queues;

  std::unique_ptr<DeviceCore<1, 0>> m_coreDeviceSymbols;
};
} // namespace vkw
#endif // VKRENDERER_DEVICE_HPP
