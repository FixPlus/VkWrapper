#include "vkw/CommandPool.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {
CommandPool::CommandPool(Device &device, VkCommandPoolCreateFlags flags,
                         uint32_t queueFamily)
    : m_device(device), m_queueFamily(queueFamily), m_createFlags(flags) {

  VkCommandPoolCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.queueFamilyIndex = queueFamily;
  createInfo.flags = flags;

  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateCommandPool(
      m_device.get(), &createInfo, nullptr, &m_commandPool))
}

CommandPool::~CommandPool() {
  if (m_commandPool == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroyCommandPool(m_device.get(),
                                                   m_commandPool, nullptr);
}
} // namespace vkw