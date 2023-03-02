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

void PresentInfo::m_fill_info() {
  m_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  m_info.pNext = nullptr;
  m_info.waitSemaphoreCount = m_wait_semaphores.size();
  m_info.pWaitSemaphores = m_wait_semaphores.data();
  m_info.swapchainCount = m_swapChains.size();
  m_info.pSwapchains = m_swapChains.data();
  m_info.pImageIndices = m_images.data();
  m_info.pResults = nullptr;
}

void SubmitInfo::m_fill_info() {
  assert(m_wait_stage.size() == m_wait_semaphores.size() &&
         "Count of dst stage masks must be equal to count of wait semaphores");
  m_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  m_info.pNext = nullptr;
  m_info.commandBufferCount = m_cmd_buffers.size();
  m_info.pCommandBuffers = m_cmd_buffers.data();
  m_info.signalSemaphoreCount = m_signal_semaphores.size();
  m_info.pSignalSemaphores = m_signal_semaphores.data();
  m_info.waitSemaphoreCount = m_wait_semaphores.size();
  m_info.pWaitSemaphores = m_wait_semaphores.data();
  m_info.pWaitDstStageMask = m_wait_stage.data();
}
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