#include "Fence.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

Fence::Fence(Device &device, bool createSignaled) : m_device(device) {
  VkFenceCreateInfo createInfo{};
  VkFenceCreateFlags flags{};
  if (createSignaled)
    flags |= VK_FENCE_CREATE_SIGNALED_BIT;

  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = flags;
  VK_CHECK_RESULT(vkCreateFence(m_device, &createInfo, nullptr, &m_fence))
}

bool Fence::wait_impl(Device &device, VkFence *pFences, uint32_t fenceCount,
                      bool waitAll, uint64_t timeout) {
  auto result = vkWaitForFences(device, fenceCount, pFences,
                                static_cast<VkBool32>(waitAll), timeout);
  if (result == VK_TIMEOUT)
    return false;
  if (result == VK_SUCCESS)
    return true;
  VK_CHECK_RESULT(result)

  return false;
}

bool Fence::signaled() const {
  auto result = vkGetFenceStatus(m_device, m_fence);
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
  vkDestroyFence(m_device, m_fence, nullptr);
}

void Fence::reset() { VK_CHECK_RESULT(vkResetFences(m_device, 1, &m_fence)) }
} // namespace vkr
