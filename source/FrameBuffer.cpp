#include "FrameBuffer.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Utils.hpp"

namespace vkw {

FrameBuffer::FrameBuffer(Device &device, RenderPass &renderPass,
                         VkExtent2D extents, ImageViewConstRefArray views,
                         uint32_t layers)
    : m_device(device), m_parent(renderPass), m_views(std::move(views)) {

  m_createInfo.renderPass = renderPass;
  m_createInfo.flags = 0; // TODO
  m_createInfo.pNext = nullptr;
  m_createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  m_createInfo.height = extents.height;
  m_createInfo.width = extents.width;
  m_createInfo.layers = layers;
  m_createInfo.attachmentCount = m_views.size();
  m_createInfo.pAttachments = m_views;

  VK_CHECK_RESULT(
      vkCreateFramebuffer(m_device, &m_createInfo, nullptr, &m_framebuffer))
}

FrameBuffer::~FrameBuffer() {
  if (m_framebuffer == VK_NULL_HANDLE)
    return;

  vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
}
} // namespace vkr