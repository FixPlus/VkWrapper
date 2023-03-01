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
                  uint32_t layers = 1);

  FrameBufferInfo(RenderPass const &renderPass, VkExtent2D extents,
                  std::span<ImageViewVT<V2D> const *> views);

  template <typename T>
  using ViewsContainer = boost::container::small_vector<T, 5>;

  VkExtent2D extents() const {
    return {m_createInfo.width, m_createInfo.height};
  }

  VkRect2D getFullRenderArea() const { return {{0, 0}, extents()}; }

  uint32_t layers() const { return m_createInfo.layers; }

  auto &attachments() { return m_views; }

  auto const &attachments() const { return m_views; }

  auto &info() const { return m_createInfo; }

  auto &pass() const { return m_parent.get(); }

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
              uint32_t layers = 1)
      : FrameBufferInfo(renderPass, extents, views, layers),
        UniqueVulkanObject<VkFramebuffer>(device, info()) {}

  FrameBuffer(Device const &device, RenderPass const &renderPass,
              VkExtent2D extents, std::span<ImageViewVT<V2D> const *> views)
      : FrameBufferInfo(renderPass, extents, views),
        UniqueVulkanObject<VkFramebuffer>(device, info()) {}
};
} // namespace vkw
#endif // VKRENDERER_FRAMEBUFFER_HPP
