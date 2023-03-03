#ifndef VKRENDERER_FRAMEBUFFER_HPP
#define VKRENDERER_FRAMEBUFFER_HPP

#include <vkw/Image.hpp>
#include <vkw/RenderPass.hpp>

#include <span>

namespace vkw {

class ImageViewBase;

class FrameBufferInfo {
public:
  FrameBufferInfo(RenderPass const &renderPass, VkExtent2D extents,
                  std::span<ImageViewVT<V2DA> const *> views,
                  uint32_t layers = 1) noexcept(ExceptionsDisabled);

  FrameBufferInfo(
      RenderPass const &renderPass, VkExtent2D extents,
      std::span<ImageViewVT<V2D> const *> views) noexcept(ExceptionsDisabled);

  template <typename T>
  using ViewsContainer = boost::container::small_vector<T, 5>;

  VkExtent2D extents() const noexcept {
    return {m_createInfo.width, m_createInfo.height};
  }

  VkRect2D getFullRenderArea() const noexcept { return {{0, 0}, extents()}; }

  uint32_t layers() const noexcept { return m_createInfo.layers; }

  auto &attachments() noexcept { return m_views; }

  auto const &attachments() const noexcept { return m_views; }

  auto &info() const noexcept { return m_createInfo; }

  auto &pass() const noexcept { return m_parent.get(); }

private:
  StrongReference<RenderPass const> m_parent;
  VkFramebufferCreateInfo m_createInfo{};
  ViewsContainer<VkImageView> m_rawViews;
  ViewsContainer<StrongReference<ImageViewBase const>> m_views;
};

class FrameBuffer : public FrameBufferInfo,
                    public UniqueVulkanObject<VkFramebuffer> {
public:
  FrameBuffer(Device const &device, RenderPass const &renderPass,
              VkExtent2D extents, std::span<ImageViewVT<V2DA> const *> views,
              uint32_t layers = 1) noexcept(ExceptionsDisabled)
      : FrameBufferInfo(renderPass, extents, views, layers),
        UniqueVulkanObject<VkFramebuffer>(device, info()) {}

  FrameBuffer(
      Device const &device, RenderPass const &renderPass, VkExtent2D extents,
      std::span<ImageViewVT<V2D> const *> views) noexcept(ExceptionsDisabled)
      : FrameBufferInfo(renderPass, extents, views),
        UniqueVulkanObject<VkFramebuffer>(device, info()) {}
};
} // namespace vkw
#endif // VKRENDERER_FRAMEBUFFER_HPP
