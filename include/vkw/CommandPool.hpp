#ifndef VKRENDERER_COMMANDPOOL_HPP
#define VKRENDERER_COMMANDPOOL_HPP

#include "Common.hpp"
#include <vulkan/vulkan.h>

namespace vkw {

class CommandPool {
public:
  CommandPool(Device &device, VkCommandPoolCreateFlags flags,
              uint32_t queueFamily);
  CommandPool(CommandPool &&another)
      : m_device(another.m_device), m_commandPool(another.m_commandPool),
        m_queueFamily(another.m_queueFamily),
        m_createFlags(another.m_createFlags) {
    another.m_commandPool = VK_NULL_HANDLE;
  }

  CommandPool &operator=(CommandPool &&another) noexcept {
    m_device = another.m_device;
    m_createFlags = another.m_createFlags;
    m_queueFamily = another.m_queueFamily;
    std::swap(m_commandPool, another.m_commandPool);
    return *this;
  }

  bool canResetCommandBuffer() const {
    return m_createFlags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  }

  bool transientAllocations() const {
    return m_createFlags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  }

  uint32_t queueFamilyIndex() const { return m_queueFamily; }

  Device &getParent() { return m_device; }

  virtual ~CommandPool();

  operator VkCommandPool() const { return m_commandPool; }

private:
  DeviceRef m_device;
  VkCommandPool m_commandPool;
  VkCommandPoolCreateFlags m_createFlags;
  uint32_t m_queueFamily;
};
} // namespace vkw
#endif // VKRENDERER_COMMANDPOOL_HPP
