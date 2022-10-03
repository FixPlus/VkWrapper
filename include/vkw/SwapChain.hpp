#ifndef VKRENDERER_SWAPCHAIN_HPP
#define VKRENDERER_SWAPCHAIN_HPP

#include "Common.hpp"
#include "Extensions.hpp"
#include <boost/container/small_vector.hpp>
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class SwapChain {
public:
  SwapChain(Device &device, VkSwapchainCreateInfoKHR createInfo);
  SwapChain(SwapChain &&another)
      : m_device(another.m_device), m_createInfo(another.m_createInfo),
        m_swapchain(another.m_swapchain), m_imageCount(another.m_imageCount),
        m_currentImage(another.m_currentImage),
        m_swapchain_ext(another.m_swapchain_ext) {
    another.m_swapchain = VK_NULL_HANDLE;
  }

  SwapChain &operator=(SwapChain &&another) noexcept {
    m_device = another.m_device;
    m_createInfo = another.m_createInfo;
    m_imageCount = another.m_imageCount;
    m_currentImage = another.m_currentImage;
    m_swapchain_ext = another.m_swapchain_ext;
    std::swap(m_swapchain, another.m_swapchain);

    return *this;
  }

  bool acquireNextImage(Semaphore const &signalSemaphore,
                        Fence const &signalFence,
                        uint64_t timeout = UINT64_MAX);
  bool acquireNextImage(Semaphore const &signalSemaphore,
                        uint64_t timeout = UINT64_MAX);
  bool acquireNextImage(Fence const &signalFence,
                        uint64_t timeout = UINT64_MAX);

  boost::container::small_vector<SwapChainImage, 3> retrieveImages();

  uint32_t currentImage() const;
  uint32_t imageCount() const { return m_imageCount; }

  virtual ~SwapChain();

  Extension<ext::KHR_swapchain> const &extension() const { return m_swapchain_ext; }

  operator VkSwapchainKHR() const { return m_swapchain; }

private:
  uint32_t m_imageCount{};
  bool acquireNextImageImpl(VkSemaphore semaphore, VkFence fence,
                            uint64_t timeout);
  std::optional<uint32_t> m_currentImage{};
  DeviceRef m_device;
  Extension<ext::KHR_swapchain> m_swapchain_ext;
  VkSwapchainCreateInfoKHR m_createInfo;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
};

} // namespace vkw
#endif // VKRENDERER_SWAPCHAIN_HPP
