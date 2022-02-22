#ifndef VKRENDERER_FRAMEBUFFER_HPP
#define VKRENDERER_FRAMEBUFFER_HPP

#include "Common.hpp"
#include <vulkan/vulkan.h>

namespace vkw {

class FrameBuffer {
public:
  FrameBuffer(Device &device, RenderPass &renderPass, VkExtent2D extents,
              ImageViewConstRefArray views, uint32_t layers = 1);
  FrameBuffer(FrameBuffer &&another) noexcept
      : m_device(another.m_device), m_parent(another.m_parent),
        m_framebuffer(another.m_framebuffer),
        m_views(std::move(another.m_views)) {
    another.m_framebuffer = VK_NULL_HANDLE;
  }

  virtual ~FrameBuffer();

  VkExtent2D extents() const {
    return {m_createInfo.width, m_createInfo.height};
  }

  VkRect2D getFullRenderArea() const { return {{0, 0}, extents()}; }

  uint32_t layers() const { return m_createInfo.layers; }

  ImageViewConstRefArray &attachments() { return m_views; }

  ImageViewConstRefArray const &attachments() const { return m_views; }

  operator VkFramebuffer() const { return m_framebuffer; }

private:
  Device &m_device;
  RenderPass &m_parent;
  VkFramebufferCreateInfo m_createInfo{};
  ImageViewConstRefArray m_views;
  VkFramebuffer m_framebuffer = VK_NULL_HANDLE;
};
} // namespace vkw
#endif // VKRENDERER_FRAMEBUFFER_HPP
