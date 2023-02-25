#include "vkw/FrameBuffer.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"
#include "vkw/Image.hpp"
#include "vkw/RenderPass.hpp"
#include <numeric>

namespace vkw {

FrameBuffer::FrameBuffer(Device &device, RenderPass &renderPass,
                         VkExtent2D extents,
                         std::span<ImageViewVT<V2DA> const *> views,
                         uint32_t layers)
    : m_device(device), m_parent(renderPass) {
  std::transform(
      views.begin(), views.end(), std::back_inserter(m_views),
      [](ImageViewVT<V2DA> const *view) { return StrongReference<ImageViewBase const>(*view); });
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
                           desc.format() != (*(viewIter++))->format();
                  })) {
    throw Error("Attachment format mismatch on (" + std::to_string(counter) +
                ") index: FrameBuffer(" +
                std::to_string((*(--viewIter))->format()) + ")<->RenderPass(" +
                std::to_string(
                    (renderPassAttachments.begin() + counter)->get().format()) +
                ")");
  }

  counter = 0;

  // extents check
  if (std::any_of(views.begin(), views.end(),
                  [extents, &counter](ImageViewVT<V2DA> const *view) {
                    auto viewExtent = view->image()->rawExtents();
                    counter++;
                    return extents.width > viewExtent.width ||
                           extents.height > viewExtent.height;
                  })) {
    auto viewExtent = (*(views.begin() + counter))->image()->rawExtents();
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
                  [layers, &counter](ImageViewVT<V2DA> const *view) {
                    return counter++, view->layers() < layers;
                  })) {
    throw(Error("Attachment #" + std::to_string(counter) +
                " has fewer layers(" +
                std::to_string((*(views.begin() + counter))->layers()) +
                ") than Framebuffer(" + std::to_string(layers) + ")"));
  }

  m_createInfo.attachmentCount = views.size();
  boost::container::small_vector<VkImageView, 4> rawViews;
  std::transform(
      views.begin(), views.end(), std::back_inserter(rawViews),
      [](ImageViewVT<V2DA> const *view) -> VkImageView { return *view; });

  m_createInfo.pAttachments = rawViews.data();

  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateFramebuffer(
      m_device.get(), &m_createInfo, nullptr, &m_framebuffer))
}

FrameBuffer::FrameBuffer(Device &device, RenderPass &renderPass,
                         VkExtent2D extents,
                         std::span<ImageViewVT<V2D> const *> views)
    : m_device(device), m_parent(renderPass) {
  std::transform(
      views.begin(), views.end(), std::back_inserter(m_views),
      [](ImageViewVT<V2D> const *view) { return StrongReference<ImageViewBase const>(*view); });
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
        if (desc.format() != (*viewIter)->format()) {
          throw Error("Attachment format mismatch on (" +
                      std::to_string(counter) + ") index: FrameBuffer(" +
                      std::to_string((*viewIter)->format()) +
                      ")<->RenderPass(" + std::to_string(desc.format()) + ")");
        }
        ++counter;
        ++viewIter;
      });

  counter = 0;

  // extents check
  if (std::any_of(views.begin(), views.end(),
                  [extents, &counter](ImageViewVT<V2D> const *view) {
                    auto viewExtent = view->image()->rawExtents();
                    counter++;
                    return extents.width > viewExtent.width ||
                           extents.height > viewExtent.height;
                  })) {
    auto viewExtent = (*(views.begin() + counter))->image()->rawExtents();
    throw Error("Attachment #" + std::to_string(counter) + " extents(" +
                std::to_string(viewExtent.width) + "x" +
                std::to_string(viewExtent.height) +
                ") are less than FrameBuffer extent(" +
                std::to_string(extents.width) + "x" +
                std::to_string(extents.height) + ")");
  }

  counter = 0;

  m_createInfo.attachmentCount = views.size();
  boost::container::small_vector<VkImageView, 4> rawViews;
  std::transform(
      views.begin(), views.end(), std::back_inserter(rawViews),
      [](ImageViewVT<V2D> const *view) -> VkImageView { return *view; });

  m_createInfo.pAttachments = rawViews.data();

  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateFramebuffer(
      m_device.get(), &m_createInfo, nullptr, &m_framebuffer))
}

FrameBuffer::~FrameBuffer() {
  if (m_framebuffer == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroyFramebuffer(m_device.get(),
                                                   m_framebuffer, nullptr);
}

} // namespace vkw