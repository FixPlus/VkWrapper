#include "vkw/SwapChain.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"
#include "vkw/Fence.hpp"
#include "vkw/Image.hpp"
#include "vkw/Semaphore.hpp"

namespace vkw {

SwapChain::SwapChain(Device &device, VkSwapchainCreateInfoKHR createInfo)
    : m_device(device), m_createInfo(createInfo) {
  m_swapchain_ext = static_cast<VkKhrSwapchain const *>(
      device.extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME));
  if (!m_swapchain_ext)
    throw ExtensionMissing(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  VK_CHECK_RESULT(m_swapchain_ext->vkCreateSwapchainKHR(device, &createInfo,
                                                        nullptr, &m_swapchain))
  VK_CHECK_RESULT(m_swapchain_ext->vkGetSwapchainImagesKHR(
      device, m_swapchain, &m_imageCount, nullptr))
}

SwapChain::~SwapChain() {
  if (m_swapchain != VK_NULL_HANDLE)
    m_swapchain_ext->vkDestroySwapchainKHR(m_device.get(), m_swapchain,
                                           nullptr);
}

uint32_t SwapChain::currentImage() const {
  if (!m_currentImage) {
    throw Error("No image has been acquired yet");
  }

  return m_currentImage.value();
}

bool SwapChain::acquireNextImage(const Semaphore &signalSemaphore,
                                 const Fence &signalFence, uint64_t timeout) {
  return acquireNextImageImpl(signalSemaphore, signalFence, timeout);
}

bool SwapChain::acquireNextImage(const Semaphore &signalSemaphore,
                                 uint64_t timeout) {
  return acquireNextImageImpl(signalSemaphore, VK_NULL_HANDLE, timeout);
}

bool SwapChain::acquireNextImage(const Fence &signalFence, uint64_t timeout) {
  return acquireNextImageImpl(VK_NULL_HANDLE, signalFence, timeout);
}

bool SwapChain::acquireNextImageImpl(VkSemaphore semaphore, VkFence fence,
                                     uint64_t timeout) {
  uint32_t imageIndex;
  auto result = m_swapchain_ext->vkAcquireNextImageKHR(
      m_device.get(), m_swapchain, timeout, semaphore, fence, &imageIndex);

  switch (result) {
  case VK_SUBOPTIMAL_KHR:
  case VK_SUCCESS:
    m_currentImage = imageIndex;
    return true;
  case VK_NOT_READY:
  case VK_TIMEOUT:
    return false;
  default:
    VK_CHECK_RESULT(result) return false;
  }
}

std::vector<SwapChainImage> SwapChain::retrieveImages() {

  std::vector<VkImage> images(m_imageCount);
  VK_CHECK_RESULT(m_swapchain_ext->vkGetSwapchainImagesKHR(
      m_device.get(), m_swapchain, &m_imageCount, images.data()))

  std::vector<SwapChainImage> ret{};

  for (auto const &image : images) {
    ret.push_back(SwapChainImage{
        image, m_createInfo.imageFormat, m_createInfo.imageExtent.width,
        m_createInfo.imageExtent.height, m_createInfo.imageArrayLayers,
        m_createInfo.imageUsage});
  }

  return ret;
}
} // namespace vkw
