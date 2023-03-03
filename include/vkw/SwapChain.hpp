#ifndef VKRENDERER_SWAPCHAIN_HPP
#define VKRENDERER_SWAPCHAIN_HPP

#include <vkw/Extensions.hpp>
#include <vkw/Image.hpp>

#include <optional>

namespace vkw {

class Semaphore;
class Fence;

class SwapChain : public ReferenceGuard {
public:
  SwapChain(Device &device, VkSwapchainCreateInfoKHR createInfo) noexcept(ExceptionsDisabled);

  SwapChain(SwapChain &&another) noexcept
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

  enum AcquireStatus {
    SUCCESSFUL,
    SUBOPTIMAL,
    NOT_READY,
    TIMEOUT,
    OUT_OF_DATE
  };

  AcquireStatus acquireNextImage(Semaphore const &signalSemaphore,
                                 Fence const &signalFence,
                                 uint64_t timeout = UINT64_MAX) noexcept(ExceptionsDisabled);
  AcquireStatus acquireNextImage(Semaphore const &signalSemaphore,
                                 uint64_t timeout = UINT64_MAX) noexcept(ExceptionsDisabled);
  AcquireStatus acquireNextImage(Fence const &signalFence,
                                 uint64_t timeout = UINT64_MAX) noexcept(ExceptionsDisabled);

  auto &images() const noexcept{ return m_images; }
  uint32_t currentImage() const noexcept(ExceptionsDisabled);
  uint32_t imageCount() const noexcept{ return m_imageCount; }

  virtual ~SwapChain();

  Extension<ext::KHR_swapchain> const &extension() const noexcept{
    return m_swapchain_ext;
  }

  operator VkSwapchainKHR() const noexcept{ return m_swapchain; }

private:
  uint32_t m_imageCount{};
  AcquireStatus acquireNextImageImpl(VkSemaphore semaphore, VkFence fence,
                                     uint64_t timeout) noexcept(ExceptionsDisabled);
  std::optional<uint32_t> m_currentImage{};
  StrongReference<Device> m_device;
  Extension<ext::KHR_swapchain> m_swapchain_ext;
  boost::container::small_vector<SwapChainImage, 3> m_images;
  VkSwapchainCreateInfoKHR m_createInfo;
  VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
};

} // namespace vkw
#endif // VKRENDERER_SWAPCHAIN_HPP
