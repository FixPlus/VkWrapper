#include "vkw/Fence.hpp"
#include "Utils.hpp"

namespace vkw {

namespace {

VkFenceCreateInfo fillCreateInfo(bool createSignaled) {
  VkFenceCreateInfo createInfo{};
  VkFenceCreateFlags flags{};
  if (createSignaled)
    flags |= VK_FENCE_CREATE_SIGNALED_BIT;

  createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = flags;
  return createInfo;
}
} // namespace

Fence::Fence(Device const &device, bool createSignaled)
    : UniqueVulkanObject<VkFence>(device, fillCreateInfo(createSignaled)) {}

bool Fence::wait_impl(Device const &device, VkFence const *pFences,
                      uint32_t fenceCount, bool waitAll, uint64_t timeout) {
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
  auto result = parent().core<1, 0>().vkGetFenceStatus(parent(), handle());
  if (result == VK_SUCCESS)
    return true;
  if (result == VK_NOT_READY)
    return false;
  VK_CHECK_RESULT(result)

  return false;
}

void Fence::reset() {
  auto h = handle();
  VK_CHECK_RESULT(parent().core<1, 0>().vkResetFences(parent(), 1, &h))
}
} // namespace vkw
