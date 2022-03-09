#include "vkw/FrameBuffer.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"
#include "vkw/Image.hpp"
#include "vkw/RenderPass.hpp"

namespace vkw {

FrameBuffer::FrameBuffer(Device &device, RenderPass &renderPass,
                         VkExtent2D extents,
                         Image2DArrayViewConstRefArray const &views,
                         uint32_t layers)
    : m_device(device), m_parent(renderPass), m_views(views) {
  m_createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.width = extents.width;
  m_createInfo.height = extents.height;
  m_createInfo.renderPass = renderPass;
  m_createInfo.layers = layers;
  m_createInfo.flags = 0;

  // Attachment validation

  auto &renderPassAttachments = renderPass.info().attachmentDescriptions();

  // size match check

  if (views.size() != renderPassAttachments.size())
    throw Error("Attachment count mismatch: FrameBuffer(" +
                std::to_string(views.size()) + ")<->RenderPass(" +
                std::to_string(renderPassAttachments.size()) + ")");

  auto viewIter = views.begin();
  uint32_t counter = 0;

  // format match check
  if (std::any_of(renderPassAttachments.begin(), renderPassAttachments.end(),
                  [&viewIter, &counter](AttachmentDescription const &desc) {
                    return counter++,
                           desc.format() != (viewIter++)->get().format();
                  })) {
    throw Error("Attachment format mismatch on (" + std::to_string(counter) +
                ") index: FrameBuffer(" +
                std::to_string((--viewIter)->get().format()) +
                ")<->RenderPass(" +
                std::to_string(
                    (renderPassAttachments.begin() + counter)->get().format()) +
                ")");
  }

  counter = 0;

  // extents check
  if (std::any_of(views.begin(), views.end(),
                  [extents, &counter](Image2DArrayViewCRef const &view) {
                    auto viewExtent = view.get().image()->rawExtents();
                    counter++;
                    return extents.width > viewExtent.width ||
                           extents.height > viewExtent.height;
                  })) {
    auto viewExtent = (views.begin() + counter)->get().image()->rawExtents();
    throw Error("Attachment #" + std::to_string(counter) + " extents(" +
                std::to_string(viewExtent.width) + "x" +
                std::to_string(viewExtent.height) +
                ") are less than FrameBuffer extent(" +
                std::to_string(extents.width) + "x" +
                std::to_string(extents.height) + ")");
  }

  counter = 0;

  // layer check
  if (std::any_of(views.begin(), views.end(),
                  [layers, &counter](Image2DArrayViewCRef const &view) {
                    return counter++, view.get().layers() < layers;
                  })) {
    throw(Error("Attachment #" + std::to_string(counter) +
                " has fewer layers(" +
                std::to_string((views.begin() + counter)->get().layers()) +
                ") than Framebuffer(" + std::to_string(layers) + ")"));
  }

  m_createInfo.attachmentCount = m_views.size();
  m_createInfo.pAttachments = m_views;

  VK_CHECK_RESULT(m_device.core<1, 0>().vkCreateFramebuffer(
      m_device, &m_createInfo, nullptr, &m_framebuffer))
}

FrameBuffer::FrameBuffer(Device &device, RenderPass &renderPass,
                         VkExtent2D extents,
                         Image2DViewConstRefArray const &views)
    : m_device(device), m_parent(renderPass), m_views(views) {
  m_createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.width = extents.width;
  m_createInfo.height = extents.height;
  m_createInfo.renderPass = renderPass;
  m_createInfo.layers = 1;
  m_createInfo.flags = 0;

  // Attachment validation

  auto &renderPassAttachments = renderPass.info().attachmentDescriptions();

  // size match check

  if (views.size() != renderPassAttachments.size())
    throw Error("Attachment count mismatch: FrameBuffer(" +
                std::to_string(views.size()) + ")<->RenderPass(" +
                std::to_string(renderPassAttachments.size()) + ")");

  auto viewIter = views.begin();
  uint32_t counter = 0;

  // format match check
  std::for_each(
      renderPassAttachments.begin(), renderPassAttachments.end(),
      [&viewIter, &counter](AttachmentDescription const &desc) {
        if (desc.format() == viewIter->get().format()) {
          throw Error("Attachment format mismatch on (" +
                      std::to_string(counter) + ") index: FrameBuffer(" +
                      std::to_string(viewIter->get().format()) +
                      ")<->RenderPass(" + std::to_string(desc.format()) + ")");
        }
        ++counter;
        ++viewIter;
      });

  counter = 0;

  // extents check
  if (std::any_of(views.begin(), views.end(),
                  [extents, &counter](Image2DViewCRef const &view) {
                    auto viewExtent = view.get().image()->rawExtents();
                    counter++;
                    return extents.width > viewExtent.width ||
                           extents.height > viewExtent.height;
                  })) {
    auto viewExtent = (views.begin() + counter)->get().image()->rawExtents();
    throw Error("Attachment #" + std::to_string(counter) + " extents(" +
                std::to_string(viewExtent.width) + "x" +
                std::to_string(viewExtent.height) +
                ") are less than FrameBuffer extent(" +
                std::to_string(extents.width) + "x" +
                std::to_string(extents.height) + ")");
  }

  counter = 0;

  m_createInfo.attachmentCount = m_views.size();
  m_createInfo.pAttachments = m_views;

  VK_CHECK_RESULT(m_device.core<1, 0>().vkCreateFramebuffer(
      m_device, &m_createInfo, nullptr, &m_framebuffer))
}

FrameBuffer::~FrameBuffer() {
  if (m_framebuffer == VK_NULL_HANDLE)
    return;

  m_device.core<1, 0>().vkDestroyFramebuffer(m_device, m_framebuffer, nullptr);
}
} // namespace vkw