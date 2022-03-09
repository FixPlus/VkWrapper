#include "vkw/Queue.hpp"
#include "Utils.hpp"
#include "vkw/CommandBuffer.hpp"
#include "vkw/Device.hpp"
#include "vkw/Fence.hpp"
#include "vkw/Instance.hpp"
#include "vkw/Semaphore.hpp"
#include "vkw/Surface.hpp"
#include "vkw/SwapChain.hpp"
#include <cassert>

namespace vkw {

Queue::Queue(Device &parent, uint32_t queueFamilyIndex, uint32_t queueIndex)
    : m_parent(parent), m_familyIndex(queueFamilyIndex) {
  m_parent.core<1, 0>().vkGetDeviceQueue(parent, queueFamilyIndex, queueIndex,
                                         &m_queue);

  auto &queueFamilyProperties = m_parent.physicalDevice().queueProperties();

  queueFamilyProperties.at(queueFamilyIndex).queueFlags;
}

bool Queue::supportsPresenting(Surface const &surface) const {

  auto *surfaceExt = static_cast<VkKhrSurface const *>(
      m_parent.getParent().extension(VK_KHR_SURFACE_EXTENSION_NAME));

  if (!surfaceExt)
    return false;

  VkBool32 ret;
  VK_CHECK_RESULT(surfaceExt->vkGetPhysicalDeviceSurfaceSupportKHR(
      m_parent.physicalDevice(), m_familyIndex, surface, &ret))
  return ret;
}

void Queue::waitIdle() {
  VK_CHECK_RESULT(m_parent.core<1, 0>().vkQueueWaitIdle(m_queue))
}

void Queue::submit(const PrimaryCommandBufferConstRefArray &commandBuffer,
                   const SemaphoreConstRefArray &waitFor,
                   const std::vector<VkPipelineStageFlags> &waitTill,
                   const SemaphoreConstRefArray &signalTo, const Fence *fence) {

  assert(waitTill.size() == waitFor.size() &&
         "Count of dst stage masks must be equal to count of wait semaphores");

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.pNext = nullptr;
  submitInfo.commandBufferCount = commandBuffer.size();
  submitInfo.pCommandBuffers = commandBuffer;
  submitInfo.signalSemaphoreCount = signalTo.size();
  submitInfo.pSignalSemaphores = signalTo;
  submitInfo.waitSemaphoreCount = waitFor.size();
  submitInfo.pWaitSemaphores = waitFor;
  submitInfo.pWaitDstStageMask = waitTill.data();

  VK_CHECK_RESULT(m_parent.core<1, 0>().vkQueueSubmit(
      m_queue, 1, &submitInfo,
      fence ? fence->operator VkFence_T *() : VK_NULL_HANDLE))
}

static bool queuePresent(PFN_vkQueuePresentKHR p_vkQueuePresentKHR,
                         VkQueue queue, VkPresentInfoKHR *pPresentInfo) {
  auto result = p_vkQueuePresentKHR(queue, pPresentInfo);

  if (result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR) {
    return true;
  }

  if (result == VK_ERROR_OUT_OF_DATE_KHR)
    return false;

  VK_CHECK_RESULT(result)

  return false;
}

bool Queue::present(const SwapChainConstRefArray &swapChains,
                    const SemaphoreConstRefArray &waitFor) {
  VkPresentInfoKHR presentInfo{};

  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.waitSemaphoreCount = waitFor.size();
  presentInfo.pWaitSemaphores = waitFor;
  presentInfo.swapchainCount = swapChains.size();
  presentInfo.pSwapchains = swapChains;

  std::vector<uint32_t> images;

  images.reserve(swapChains.size());

  for (auto &swapChain : swapChains) {
    images.push_back(swapChain.get().currentImage());
  }
  presentInfo.pImageIndices = images.data();
  presentInfo.pResults = nullptr;

  return queuePresent(swapChains.begin()->get().ext()->vkQueuePresentKHR,
                      m_queue, &presentInfo);
}

bool Queue::present(const SwapChain &swapChain,
                    const SemaphoreConstRefArray &waitFor) {
  VkPresentInfoKHR presentInfo{};

  presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  presentInfo.pNext = nullptr;
  presentInfo.waitSemaphoreCount = waitFor.size();
  presentInfo.pWaitSemaphores = waitFor;
  presentInfo.swapchainCount = 1;
  VkSwapchainKHR rawSwapChain = swapChain.operator VkSwapchainKHR_T *();
  presentInfo.pSwapchains = &rawSwapChain;
  uint32_t image = swapChain.currentImage();
  presentInfo.pImageIndices = &image;
  presentInfo.pResults = nullptr;

  return queuePresent(swapChain.ext()->vkQueuePresentKHR, m_queue,
                      &presentInfo);
}
} // namespace vkw