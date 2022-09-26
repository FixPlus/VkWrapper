#include "vkw/Queue.hpp"
#include "Utils.hpp"
#include "vkw/CommandBuffer.hpp"
#include "vkw/Device.hpp"
#include "vkw/Extensions.hpp"
#include "vkw/Fence.hpp"
#include "vkw/Instance.hpp"
#include "vkw/Semaphore.hpp"
#include "vkw/Surface.hpp"
#include "vkw/SwapChain.hpp"
#include <cassert>

namespace vkw {

Queue::Queue(Device &parent, uint32_t queueFamilyIndex, uint32_t queueIndex)
    : m_parent(parent), m_familyIndex(queueFamilyIndex),
      m_queueIndex(queueIndex) {
  m_parent.get().core<1, 0>().vkGetDeviceQueue(parent, queueFamilyIndex,
                                               queueIndex, &m_queue);
}

bool Queue::supportsPresenting(Surface const &surface) const {
  VkBool32 ret;
  VK_CHECK_RESULT(surface.ext().vkGetPhysicalDeviceSurfaceSupportKHR(
      m_parent.get().physicalDevice(), m_familyIndex, surface, &ret))
  return ret;
}

void Queue::waitIdle() const {
  VK_CHECK_RESULT(m_parent.get().core<1, 0>().vkQueueWaitIdle(m_queue))
}

static bool queuePresent(PFN_vkQueuePresentKHR p_vkQueuePresentKHR,
                         VkQueue queue, VkPresentInfoKHR const *pPresentInfo) {
  auto result = p_vkQueuePresentKHR(queue, pPresentInfo);

  if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
    return true;
  }

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return false;

  VK_CHECK_RESULT(result)

  return false;
}

bool Queue::present(PresentInfo const &presentInfo) const {
  VkPresentInfoKHR info = presentInfo;
  return queuePresent(presentInfo.swapChainExtension().vkQueuePresentKHR,
                      m_queue, &info);
}
void Queue::m_submit(const VkSubmitInfo *info, size_t infoCount,
                     Fence const *fence) const {

    VK_CHECK_RESULT(m_parent.get().core<1, 0>().vkQueueSubmit(
        m_queue, infoCount, info,
        fence ? fence->operator VkFence_T *() : VK_NULL_HANDLE))}

QueueFamily const &Queue::family() const {
  return m_parent.get().physicalDevice().queueFamilies().at(m_familyIndex);
}
} // namespace vkw