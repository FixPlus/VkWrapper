#include "vkw/CommandPool.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

namespace {

VkCommandPoolCreateInfo fillCreateInfo(VkCommandPoolCreateFlags flags,
                                       uint32_t queueFamily) {
  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.queueFamilyIndex = queueFamily;
  createInfo.flags = flags;

  return createInfo;
}
} // namespace
CommandPool::CommandPool(Device const &device, VkCommandPoolCreateFlags flags,
                         uint32_t queueFamily)
    : UniqueVulkanObject<VkCommandPool>(device,
                                        fillCreateInfo(flags, queueFamily)),
      m_queueFamily(queueFamily), m_createFlags(flags) {}

} // namespace vkw