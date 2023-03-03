#include "vkw/SwapChain.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"
#include "vkw/Fence.hpp"
#include "vkw/Image.hpp"
#include "vkw/Semaphore.hpp"

namespace vkw {

SwapChain::SwapChain(Device &device, VkSwapchainCreateInfoKHR createInfo) noexcept(ExceptionsDisabled)
    : m_device(device), m_createInfo(createInfo), m_swapchain_ext(device) {
  VK_CHECK_RESULT(m_swapchain_ext.vkCreateSwapchainKHR(
      device, &createInfo, device.hostAllocator().allocator(), &m_swapchain));
  VK_CHECK_RESULT(m_swapchain_ext.vkGetSwapchainImagesKHR(
      device, m_swapchain, &m_imageCount, nullptr));

  boost::container::small_vector<VkImage, 3> images(m_imageCount);
  VK_CHECK_RESULT(m_swapchain_ext.vkGetSwapchainImagesKHR(
      m_device.get(), m_swapchain, &m_imageCount, images.data()))

  for (auto const &image : images) {
    m_images.push_back(SwapChainImage{
        image, m_createInfo.imageFormat, m_createInfo.imageExtent.width,
        m_createInfo.imageExtent.height, m_createInfo.imageArrayLayers,
        m_createInfo.imageUsage});
  }
}

SwapChain::~SwapChain() {
  if (m_swapchain != VK_NULL_HANDLE)
    m_swapchain_ext.vkDestroySwapchainKHR(
        m_device.get(), m_swapchain,
        m_device.get().hostAllocator().allocator());
}

uint32_t SwapChain::currentImage() const noexcept(ExceptionsDisabled){
  if (!m_currentImage) {
    postError(Error("No image has been acquired yet"));
  }

  return m_currentImage.value();
}

SwapChain::AcquireStatus
SwapChain::acquireNextImage(const Semaphore &signalSemaphore,
                            const Fence &signalFence, uint64_t timeout) noexcept(ExceptionsDisabled){
  return acquireNextImageImpl(signalSemaphore, signalFence, timeout);
}

SwapChain::AcquireStatus
SwapChain::acquireNextImage(const Semaphore &signalSemaphore,
                            uint64_t timeout) noexcept(ExceptionsDisabled){
  return acquireNextImageImpl(signalSemaphore, VK_NULL_HANDLE, timeout);
}

SwapChain::AcquireStatus SwapChain::acquireNextImage(const Fence &signalFence,
                                                     uint64_t timeout) noexcept(ExceptionsDisabled){
  return acquireNextImageImpl(VK_NULL_HANDLE, signalFence, timeout);
}

SwapChain::AcquireStatus SwapChain::acquireNextImageImpl(VkSemaphore semaphore,
                                                         VkFence fence,
                                                         uint64_t timeout) noexcept(ExceptionsDisabled){
  uint32_t imageIndex;
  auto result = m_swapchain_ext.vkAcquireNextImageKHR(
      m_device.get(), m_swapchain, timeout, semaphore, fence, &imageIndex);

  switch (result) {
  case VK_SUBOPTIMAL_KHR:
    m_currentImage = imageIndex;
    return AcquireStatus::SUBOPTIMAL;
  case VK_SUCCESS:
    m_currentImage = imageIndex;
    return AcquireStatus::SUCCESSFUL;
  case VK_NOT_READY:
    return AcquireStatus::NOT_READY;
  case VK_TIMEOUT:
    return AcquireStatus::TIMEOUT;
  case VK_ERROR_OUT_OF_DATE_KHR:
    return AcquireStatus::OUT_OF_DATE;
  default:
    VK_CHECK_RESULT(result)
    return AcquireStatus::OUT_OF_DATE;
  }
}

} // namespace vkw
