#ifndef VKRENDERER_COMMANDPOOL_HPP
#define VKRENDERER_COMMANDPOOL_HPP

#include <vulkan/vulkan.h>

namespace vkw {

class Device;
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
  Device &m_device;
  VkCommandPool m_commandPool;
  VkCommandPoolCreateFlags m_createFlags;
  uint32_t m_queueFamily;
};
} // namespace vkr
#endif // VKRENDERER_COMMANDPOOL_HPP
