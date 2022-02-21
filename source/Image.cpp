#include "Image.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

bool operator==(VkImageSubresourceRange const &lhs,
                VkImageSubresourceRange const &rhs) {
  return lhs.levelCount == rhs.levelCount && lhs.layerCount == rhs.layerCount &&
         lhs.baseMipLevel == rhs.baseMipLevel &&
         lhs.baseArrayLayer == rhs.baseMipLevel &&
         lhs.aspectMask == rhs.aspectMask;
}

bool operator==(VkComponentMapping const &lhs, VkComponentMapping const &rhs) {
  return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b && lhs.a == rhs.a;
}

bool operator==(VkImageViewCreateInfo const &lhs,
                VkImageViewCreateInfo const &rhs) {
  return lhs.image == rhs.image &&
         lhs.subresourceRange == rhs.subresourceRange &&
         lhs.flags == rhs.flags && lhs.format == rhs.format &&
         lhs.viewType == rhs.viewType && lhs.components == rhs.components;
}

ImageBase::ImageBase(Device &device, VmaAllocator allocator,
                     VkImageCreateInfo createInfo,
                     VmaAllocationCreateInfo allocCreateInfo)
    : Allocation(allocator),
      ImageInterface(device, VK_NULL_HANDLE, createInfo){VK_CHECK_RESULT(
          vmaCreateImage(m_allocator, &m_createInfo, &allocCreateInfo, &m_image,
                         &m_allocation, &m_allocInfo))}

      ImageBase::~ImageBase() {
  if (m_image == VK_NULL_HANDLE)
    return;
  vmaDestroyImage(m_allocator, m_image, m_allocation);
}

ImageInterface::~ImageInterface() {
  for (auto const &view : m_viewsCache) {
    vkDestroyImageView(m_device, view, nullptr);
  }
}

ImageView ImageInterface::getView(VkImageViewCreateInfo viewInfo) {

  viewInfo.image = m_image;

  auto cached = std::find_if(m_viewsCache.begin(), m_viewsCache.end(),
                             [viewInfo](ImageView const &imageView) {
                               return imageView.m_createInfo == viewInfo;
                             });

  if (cached != m_viewsCache.end())
    return *cached;

  VkImageView imageView;

  VK_CHECK_RESULT(vkCreateImageView(m_device, &viewInfo, nullptr, &imageView))

  m_viewsCache.push_back(ImageView{imageView, viewInfo, this});

  return m_viewsCache.back();
}
} // namespace vkr