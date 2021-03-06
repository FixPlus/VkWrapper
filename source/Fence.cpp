#include "vkw/Fence.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

Fence::Fence(Device &device, bool createSignaled) : m_device(device) {
  VkFenceCreateInfo createInfo{};
  VkFenceCreateFlags flags{};
  if (createSignaled)
    flags |= VK_FENCE_CREATE_SIGNALED_BIT;

  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = flags;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateFence(
      m_device.get(), &createInfo, nullptr, &m_fence))
}

bool Fence::wait_impl(Device &device, VkFence *pFences, uint32_t fenceCount,
                      bool waitAll, uint64_t timeout) {
  auto result = device.core<1, 0>().vkWaitForFences(
      device, fenceCount, pFences, static_cast<VkBool32>(waitAll), timeout);
  if (result == VK_TIMEOUT)
    return false;
  if (result == VK_SUCCESS)
    return true;
  VK_CHECK_RESULT(result)

  return false;
}

bool Fence::signaled() const {
  auto result =
      m_device.get().core<1, 0>().vkGetFenceStatus(m_device.get(), m_fence);
  if (result == VK_SUCCESS)
    return true;
  if (result == VK_NOT_READY)
    return false;
  VK_CHECK_RESULT(result)

  return false;
}

Fence::~Fence() {
  if (m_fence == VK_NULL_HANDLE)
    return;
  m_device.get().core<1, 0>().vkDestroyFence(m_device.get(), m_fence, nullptr);
}

void Fence::reset() {
  VK_CHECK_RESULT(
      m_device.get().core<1, 0>().vkResetFences(m_device.get(), 1, &m_fence))
}
} // namespace vkw
