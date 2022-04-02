#include "vkw/Semaphore.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

Semaphore::Semaphore(Device &device) : m_device(device) {
  VkSemaphoreCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  createInfo.pNext = nullptr;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateSemaphore(
      m_device.get(), &createInfo, nullptr, &m_semaphore))
}

Semaphore::~Semaphore() {
  if (m_semaphore == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroySemaphore(m_device.get(), m_semaphore,
                                                 nullptr);
}
} // namespace vkw