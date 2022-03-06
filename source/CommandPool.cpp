#include "CommandPool.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {
CommandPool::CommandPool(Device &device, VkCommandPoolCreateFlags flags,
                         uint32_t queueFamily)
    : m_device(device), m_queueFamily(queueFamily), m_createFlags(flags) {

  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.queueFamilyIndex = queueFamily;
  createInfo.flags = flags;

  VK_CHECK_RESULT(m_device.core_1_0().vkCreateCommandPool(
      m_device, &createInfo, nullptr, &m_commandPool))
}

CommandPool::~CommandPool() {
  if (m_commandPool == VK_NULL_HANDLE)
    return;

  m_device.core_1_0().vkDestroyCommandPool(m_device, m_commandPool, nullptr);
}
} // namespace vkw