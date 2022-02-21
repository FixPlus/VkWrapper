#ifndef VKRENDERER_SWAPCHAIN_HPP
#define VKRENDERER_SWAPCHAIN_HPP

#include "Common.hpp"
#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Surface;
class Device;
class Semaphore;
class Fence;
class ImageInterface;

class SwapChain {
public:
  SwapChain(Device &device, VkSwapchainCreateInfoKHR createInfo);
  SwapChain(SwapChain &&another)
      : m_device(another.m_device), m_createInfo(another.m_createInfo),
        m_swapchain(another.m_swapchain) {
    another.m_swapchain = VK_NULL_HANDLE;
  }

  bool acquireNextImage(Semaphore const &signalSemaphore,
                        Fence const &signalFence,
                        uint64_t timeout = UINT64_MAX);
  bool acquireNextImage(Semaphore const &signalSemaphore,
                        uint64_t timeout = UINT64_MAX);
  bool acquireNextImage(Fence const &signalFence,
                        uint64_t timeout = UINT64_MAX);

  std::vector<ImageInterface> retrieveImages();

  uint32_t currentImage() const;
  uint32_t imageCount() const { return m_imageCount; }

  virtual ~SwapChain();

  operator VkSwapchainKHR() const { return m_swapchain; }

private:
  uint32_t m_imageCount{};
  bool acquireNextImageImpl(VkSemaphore semaphore, VkFence fence,
                            uint64_t timeout);
  std::optional<uint32_t> m_currentImage{};
  Device &m_device;
  VkSwapchainCreateInfoKHR m_createInfo;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
};

VKR_DECLARE_ARRAY_TYPES_NON_CONV(SwapChain, VkSwapchainKHR)
} // namespace vkr
#endif // VKRENDERER_SWAPCHAIN_HPP
