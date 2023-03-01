#ifndef VKRENDERER_COMMANDPOOL_HPP
#define VKRENDERER_COMMANDPOOL_HPP

#include <vkw/Device.hpp>

namespace vkw {

class CommandPool : public UniqueVulkanObject<VkCommandPool> {
public:
  CommandPool(Device const &device, VkCommandPoolCreateFlags flags,
              uint32_t queueFamily);

  bool canResetCommandBuffer() const {
    return m_createFlags & VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  }

  bool transientAllocations() const {
    return m_createFlags & VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
  }

  uint32_t queueFamilyIndex() const { return m_queueFamily; }

private:
  VkCommandPoolCreateFlags m_createFlags;
  uint32_t m_queueFamily;
};
} // namespace vkw
#endif // VKRENDERER_COMMANDPOOL_HPP
