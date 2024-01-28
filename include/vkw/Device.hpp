#ifndef VKRENDERER_DEVICE_HPP
#define VKRENDERER_DEVICE_HPP

#include <vkw/PhysicalDevice.hpp>
#include <vkw/UniqueVulkanObject.hpp>

#include <vma/vk_mem_alloc.h>

#include <cstdint>
#include <map>
#include <set>
#include <unordered_map>

namespace vkw {

class Instance;
class BufferBase;
class Queue;

enum class ext;

class DeviceInfo {
public:
  explicit DeviceInfo(Instance const &parent,
                      PhysicalDevice phDevice) noexcept(ExceptionsDisabled);

  auto &info() const noexcept { return m_createInfo; }

  PhysicalDevice const &physicalDevice() const noexcept { return m_ph_device; }

  auto &apiVersion() const noexcept { return m_apiVer; }

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
  Device(Instance const &parent,
         PhysicalDevice phDevice) noexcept(ExceptionsDisabled);

  Queue const &getQueue(unsigned queueFamilyIndex, unsigned queueIndex) const
      noexcept(ExceptionsDisabled) {
    return *m_queues.at(queueFamilyIndex).at(queueIndex);
  }

  Queue const &anyGraphicsQueue() const noexcept(ExceptionsDisabled);

  Queue const &anyComputeQueue() const noexcept(ExceptionsDisabled);

  Queue const &anyTransferQueue() const noexcept(ExceptionsDisabled);

  Queue const &getSpecificQueue(QueueFamily::Type type) const
      noexcept(ExceptionsDisabled);

  VmaAllocator getAllocator() const noexcept { return m_allocator.get(); }

  template <uint32_t major, uint32_t minor>
  DeviceCore<major, minor> core() const noexcept(ExceptionsDisabled) {
    if (apiVersion() < ApiVersion{major, minor, 0})
      postError(Error{
          "Cannot use core " + std::to_string(major) + "." +
          std::to_string(minor) +
          " vulkan symbols. Version loaded: " + std::string(apiVersion())});
    return *static_cast<DeviceCore<major, minor> const *>(
        m_coreDeviceSymbols.get());
  }

  // Returns budget of each device heap.
  std::vector<std::pair<VkDeviceSize, VkDeviceSize>> getDeviceMemoryUsage();
  // Should be called every frame.
  void frame();

  void waitIdle() noexcept(ExceptionsDisabled);

  ~Device() override;

private:
  VmaAllocator m_allocatorCreateImpl() noexcept(ExceptionsDisabled);
  struct AllocatorDeleter {
    void operator()(VmaAllocator a);
  };
  std::unique_ptr<std::remove_pointer_t<VmaAllocator>, AllocatorDeleter>
      m_allocator;
  size_t m_currentFrame = 0u;
  using InFamilyQueueContainerT =
      boost::container::small_vector<std::unique_ptr<Queue>, 3>;
  using FamilyContainerT =
      boost::container::small_vector<InFamilyQueueContainerT, 3>;

  FamilyContainerT m_queues;

  std::unique_ptr<DeviceCore<1, 0>> m_coreDeviceSymbols;
};
} // namespace vkw
#endif // VKRENDERER_DEVICE_HPP
