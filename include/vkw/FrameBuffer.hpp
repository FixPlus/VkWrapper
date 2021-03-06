#ifndef VKRENDERER_FRAMEBUFFER_HPP
#define VKRENDERER_FRAMEBUFFER_HPP

#include "Common.hpp"
#include <vulkan/vulkan.h>

namespace vkw {

class FrameBuffer {
public:
  FrameBuffer(Device &device, RenderPass &renderPass, VkExtent2D extents,
              Image2DArrayViewConstRefArray const &views, uint32_t layers = 1);

  FrameBuffer(Device &device, RenderPass &renderPass, VkExtent2D extents,
              Image2DViewConstRefArray const &views);

  FrameBuffer(FrameBuffer &&another) noexcept
      : m_device(another.m_device), m_parent(another.m_parent),
        m_framebuffer(another.m_framebuffer),
        m_views(std::move(another.m_views)),
        m_createInfo(another.m_createInfo) {
    another.m_framebuffer = VK_NULL_HANDLE;
  }

  FrameBuffer &operator=(FrameBuffer &&another) noexcept {
    m_device = another.m_device;
    m_parent = another.m_parent;
    m_createInfo = another.m_createInfo;
    m_views = std::move(another.m_views);
    std::swap(m_framebuffer, another.m_framebuffer);
    return *this;
  }

  virtual ~FrameBuffer();

  VkExtent2D extents() const {
    return {m_createInfo.width, m_createInfo.height};
  }

  VkRect2D getFullRenderArea() const { return {{0, 0}, extents()}; }

  uint32_t layers() const { return m_createInfo.layers; }

  std::vector<ImageViewCRef> &attachments() { return m_views; }

  std::vector<ImageViewCRef> const &attachments() const { return m_views; }

  operator VkFramebuffer() const { return m_framebuffer; }

private:
  DeviceRef m_device;
  RenderPassRef m_parent;
  VkFramebufferCreateInfo m_createInfo{};
  std::vector<ImageViewCRef> m_views;
  VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
};
} // namespace vkw
#endif // VKRENDERER_FRAMEBUFFER_HPP
