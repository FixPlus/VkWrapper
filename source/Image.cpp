#include "vkw/Image.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"
#include <cassert>

namespace vkw {

ImageViewBase::ImageViewBase(ImageInterface const *image, VkFormat format,
                             uint32_t baseMipLevel, uint32_t levelCount,
                             VkComponentMapping componentMapping,
                             VkImageViewCreateFlags flags)
    : m_parent(*image) {
  m_createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.subresourceRange.baseMipLevel = baseMipLevel;
  m_createInfo.subresourceRange.levelCount = levelCount;
  m_createInfo.components = componentMapping;
  m_createInfo.flags = flags;
  m_createInfo.image = image->operator VkImage_T *();
  m_createInfo.format = format;
}

bool operator==(VkImageSubresourceRange const &lhs,
                VkImageSubresourceRange const &rhs) {
  return lhs.levelCount == rhs.levelCount && lhs.layerCount == rhs.layerCount &&
         lhs.baseMipLevel == rhs.baseMipLevel &&
         lhs.baseArrayLayer == rhs.baseArrayLayer &&
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

bool ImageViewBase::operator==(ImageViewBase const &another) const {
  return m_createInfo == another.m_createInfo;
}

AllocatedImage::AllocatedImage(VmaAllocator allocator,
                               VmaAllocationCreateInfo allocCreateInfo)
    : Allocation(allocator), ImageInterface(){VK_CHECK_RESULT(vmaCreateImage(
                                 m_allocator, &m_createInfo, &allocCreateInfo,
                                 &m_image, &m_allocation, &m_allocInfo))}

      AllocatedImage::~AllocatedImage() {
  if (m_image == VK_NULL_HANDLE)
    return;
  vmaDestroyImage(m_allocator, m_image, m_allocation);
}

bool ImageInterface::isDepthFormat(VkFormat format) {
  switch (format) {
  case VK_FORMAT_D16_UNORM:
  case VK_FORMAT_D16_UNORM_S8_UINT:
  case VK_FORMAT_X8_D24_UNORM_PACK32:
  case VK_FORMAT_D24_UNORM_S8_UINT:
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
  case VK_FORMAT_D32_SFLOAT:
  case VK_FORMAT_S8_UINT:
    return true;
  default:
    return false;
  }
}

bool ImageInterface::isColorFormat(VkFormat format) {
  return !isDepthFormat(format);
}
VkImageSubresourceRange ImageInterface::completeSubresourceRange() const {
  VkImageSubresourceRange ret{};
  ret.baseMipLevel = 0;
  ret.levelCount = m_createInfo.mipLevels;
  ret.aspectMask = isDepthFormat(format()) ? VK_IMAGE_ASPECT_DEPTH_BIT
                                           : VK_IMAGE_ASPECT_COLOR_BIT;
  ret.baseArrayLayer = 0;
  ret.layerCount = m_createInfo.arrayLayers;
  return ret;
}

ImageViewCreator::~ImageViewCreator() {
  if (m_imageView == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroyImageView(m_device.get(), m_imageView,
                                                 nullptr);
}

ImageViewCreator::ImageViewCreator(Device const &device) : m_device(device) {
  VK_CHECK_RESULT(device.core<1, 0>().vkCreateImageView(
      m_device.get(), &m_createInfo, nullptr, &m_imageView))
}

unsigned m_FormatRedBits(VkFormat format) {
  switch (format) {
  case VK_FORMAT_R8G8B8A8_UINT:
  case VK_FORMAT_R8G8B8A8_SNORM:
  case VK_FORMAT_R8G8B8A8_SSCALED:
  case VK_FORMAT_R8G8B8A8_UNORM:
  case VK_FORMAT_R8G8B8A8_SINT:
  case VK_FORMAT_R8G8B8A8_USCALED:
  case VK_FORMAT_R8G8B8A8_SRGB:
  case VK_FORMAT_R8G8B8_SINT:
  case VK_FORMAT_R8G8B8_SNORM:
  case VK_FORMAT_R8G8B8_SRGB:
  case VK_FORMAT_R8G8B8_UINT:
  case VK_FORMAT_R8G8B8_USCALED:
  case VK_FORMAT_R8G8B8_UNORM:
  case VK_FORMAT_R8G8B8_SSCALED:
  case VK_FORMAT_R8G8_SINT:
  case VK_FORMAT_R8G8_SNORM:
  case VK_FORMAT_R8G8_SRGB:
  case VK_FORMAT_R8G8_UINT:
  case VK_FORMAT_R8G8_USCALED:
  case VK_FORMAT_R8G8_UNORM:
  case VK_FORMAT_R8G8_SSCALED:
  case VK_FORMAT_R8_SINT:
  case VK_FORMAT_R8_SNORM:
  case VK_FORMAT_R8_SRGB:
  case VK_FORMAT_R8_UINT:
  case VK_FORMAT_R8_USCALED:
  case VK_FORMAT_R8_UNORM:
  case VK_FORMAT_R8_SSCALED:
    return 8;
  case VK_FORMAT_R16G16B16A16_UINT:
  case VK_FORMAT_R16G16B16A16_SNORM:
  case VK_FORMAT_R16G16B16A16_SSCALED:
  case VK_FORMAT_R16G16B16A16_UNORM:
  case VK_FORMAT_R16G16B16A16_SINT:
  case VK_FORMAT_R16G16B16A16_USCALED:
  case VK_FORMAT_R16G16B16_SINT:
  case VK_FORMAT_R16G16B16_SNORM:
  case VK_FORMAT_R16G16B16_UINT:
  case VK_FORMAT_R16G16B16_USCALED:
  case VK_FORMAT_R16G16B16_UNORM:
  case VK_FORMAT_R16G16B16_SSCALED:
  case VK_FORMAT_R16G16_SINT:
  case VK_FORMAT_R16G16_SNORM:
  case VK_FORMAT_R16G16_UINT:
  case VK_FORMAT_R16G16_USCALED:
  case VK_FORMAT_R16G16_UNORM:
  case VK_FORMAT_R16G16_SSCALED:
  case VK_FORMAT_R16_SINT:
  case VK_FORMAT_R16_SNORM:
  case VK_FORMAT_R16_UINT:
  case VK_FORMAT_R16_USCALED:
  case VK_FORMAT_R16_UNORM:
  case VK_FORMAT_R16_SSCALED:
    return 16;
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    return 11;
  default:
    assert("Format Not Handled yet");
  }
  return 0;
}
unsigned m_FormatGreenBits(VkFormat format) {
  switch (format) {
  case VK_FORMAT_R8G8B8A8_UINT:
  case VK_FORMAT_R8G8B8A8_SNORM:
  case VK_FORMAT_R8G8B8A8_SSCALED:
  case VK_FORMAT_R8G8B8A8_UNORM:
  case VK_FORMAT_R8G8B8A8_SINT:
  case VK_FORMAT_R8G8B8A8_USCALED:
  case VK_FORMAT_R8G8B8A8_SRGB:
  case VK_FORMAT_R8G8B8_SINT:
  case VK_FORMAT_R8G8B8_SNORM:
  case VK_FORMAT_R8G8B8_SRGB:
  case VK_FORMAT_R8G8B8_UINT:
  case VK_FORMAT_R8G8B8_USCALED:
  case VK_FORMAT_R8G8B8_UNORM:
  case VK_FORMAT_R8G8B8_SSCALED:
  case VK_FORMAT_R8G8_SINT:
  case VK_FORMAT_R8G8_SNORM:
  case VK_FORMAT_R8G8_SRGB:
  case VK_FORMAT_R8G8_UINT:
  case VK_FORMAT_R8G8_USCALED:
  case VK_FORMAT_R8G8_UNORM:
  case VK_FORMAT_R8G8_SSCALED:
    return 8;
  case VK_FORMAT_R16G16B16A16_UINT:
  case VK_FORMAT_R16G16B16A16_SNORM:
  case VK_FORMAT_R16G16B16A16_SSCALED:
  case VK_FORMAT_R16G16B16A16_UNORM:
  case VK_FORMAT_R16G16B16A16_SINT:
  case VK_FORMAT_R16G16B16A16_USCALED:
  case VK_FORMAT_R16G16B16_SINT:
  case VK_FORMAT_R16G16B16_SNORM:
  case VK_FORMAT_R16G16B16_UINT:
  case VK_FORMAT_R16G16B16_USCALED:
  case VK_FORMAT_R16G16B16_UNORM:
  case VK_FORMAT_R16G16B16_SSCALED:
  case VK_FORMAT_R16G16_SINT:
  case VK_FORMAT_R16G16_SNORM:
  case VK_FORMAT_R16G16_UINT:
  case VK_FORMAT_R16G16_USCALED:
  case VK_FORMAT_R16G16_UNORM:
  case VK_FORMAT_R16G16_SSCALED:
    return 16;
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    return 11;
  case VK_FORMAT_R8_SINT:
  case VK_FORMAT_R8_SNORM:
  case VK_FORMAT_R8_SRGB:
  case VK_FORMAT_R8_UINT:
  case VK_FORMAT_R8_USCALED:
  case VK_FORMAT_R8_UNORM:
  case VK_FORMAT_R8_SSCALED:
  case VK_FORMAT_R16_SINT:
  case VK_FORMAT_R16_SNORM:
  case VK_FORMAT_R16_UINT:
  case VK_FORMAT_R16_USCALED:
  case VK_FORMAT_R16_UNORM:
  case VK_FORMAT_R16_SSCALED:
    return 0;

  default:
    assert("Format Not Handled yet");
  }
  return 0;
}
unsigned m_FormatBlueBits(VkFormat format) {
  switch (format) {
  case VK_FORMAT_R8G8B8A8_UINT:
  case VK_FORMAT_R8G8B8A8_SNORM:
  case VK_FORMAT_R8G8B8A8_SSCALED:
  case VK_FORMAT_R8G8B8A8_UNORM:
  case VK_FORMAT_R8G8B8A8_SINT:
  case VK_FORMAT_R8G8B8A8_USCALED:
  case VK_FORMAT_R8G8B8A8_SRGB:
  case VK_FORMAT_R8G8B8_SINT:
  case VK_FORMAT_R8G8B8_SNORM:
  case VK_FORMAT_R8G8B8_SRGB:
  case VK_FORMAT_R8G8B8_UINT:
  case VK_FORMAT_R8G8B8_USCALED:
  case VK_FORMAT_R8G8B8_UNORM:
  case VK_FORMAT_R8G8B8_SSCALED:
    return 8;

  case VK_FORMAT_R8_SSCALED:
  case VK_FORMAT_R16G16B16A16_UINT:
  case VK_FORMAT_R16G16B16A16_SNORM:
  case VK_FORMAT_R16G16B16A16_SSCALED:
  case VK_FORMAT_R16G16B16A16_UNORM:
  case VK_FORMAT_R16G16B16A16_SINT:
  case VK_FORMAT_R16G16B16A16_USCALED:
  case VK_FORMAT_R16G16B16_SINT:
  case VK_FORMAT_R16G16B16_SNORM:
  case VK_FORMAT_R16G16B16_UINT:
  case VK_FORMAT_R16G16B16_USCALED:
  case VK_FORMAT_R16G16B16_UNORM:
  case VK_FORMAT_R16G16B16_SSCALED:
    return 16;
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    return 10;
  case VK_FORMAT_R8G8_SINT:
  case VK_FORMAT_R8G8_SNORM:
  case VK_FORMAT_R8G8_SRGB:
  case VK_FORMAT_R8G8_UINT:
  case VK_FORMAT_R8G8_USCALED:
  case VK_FORMAT_R8G8_UNORM:
  case VK_FORMAT_R8G8_SSCALED:
  case VK_FORMAT_R8_SINT:
  case VK_FORMAT_R8_SNORM:
  case VK_FORMAT_R8_SRGB:
  case VK_FORMAT_R8_UINT:
  case VK_FORMAT_R8_USCALED:
  case VK_FORMAT_R8_UNORM:
  case VK_FORMAT_R16G16_SINT:
  case VK_FORMAT_R16G16_SNORM:
  case VK_FORMAT_R16G16_UINT:
  case VK_FORMAT_R16G16_USCALED:
  case VK_FORMAT_R16G16_UNORM:
  case VK_FORMAT_R16G16_SSCALED:
  case VK_FORMAT_R16_SINT:
  case VK_FORMAT_R16_SNORM:
  case VK_FORMAT_R16_UINT:
  case VK_FORMAT_R16_USCALED:
  case VK_FORMAT_R16_UNORM:
  case VK_FORMAT_R16_SSCALED:
    return 0;
  default:
    assert("Format Not Handled yet");
  }
  return 0;
}
unsigned m_FormatAlphaBits(VkFormat format) {
  switch (format) {
  case VK_FORMAT_R8G8B8A8_UINT:
  case VK_FORMAT_R8G8B8A8_SNORM:
  case VK_FORMAT_R8G8B8A8_SSCALED:
  case VK_FORMAT_R8G8B8A8_UNORM:
  case VK_FORMAT_R8G8B8A8_SINT:
  case VK_FORMAT_R8G8B8A8_USCALED:
  case VK_FORMAT_R8G8B8A8_SRGB:
    return 8;
  case VK_FORMAT_R16G16B16A16_UINT:
  case VK_FORMAT_R16G16B16A16_SNORM:
  case VK_FORMAT_R16G16B16A16_SSCALED:
  case VK_FORMAT_R16G16B16A16_UNORM:
  case VK_FORMAT_R16G16B16A16_SINT:
  case VK_FORMAT_R16G16B16A16_USCALED:
    return 16;
  case VK_FORMAT_R8G8B8_SINT:
  case VK_FORMAT_R8G8B8_SNORM:
  case VK_FORMAT_R8G8B8_SRGB:
  case VK_FORMAT_R8G8B8_UINT:
  case VK_FORMAT_R8G8B8_USCALED:
  case VK_FORMAT_R8G8B8_UNORM:
  case VK_FORMAT_R8G8B8_SSCALED:
  case VK_FORMAT_R8G8_SINT:
  case VK_FORMAT_R8G8_SNORM:
  case VK_FORMAT_R8G8_SRGB:
  case VK_FORMAT_R8G8_UINT:
  case VK_FORMAT_R8G8_USCALED:
  case VK_FORMAT_R8G8_UNORM:
  case VK_FORMAT_R8G8_SSCALED:
  case VK_FORMAT_R8_SINT:
  case VK_FORMAT_R8_SNORM:
  case VK_FORMAT_R8_SRGB:
  case VK_FORMAT_R8_UINT:
  case VK_FORMAT_R8_USCALED:
  case VK_FORMAT_R8_UNORM:
  case VK_FORMAT_R8_SSCALED:
  case VK_FORMAT_R16G16B16_SINT:
  case VK_FORMAT_R16G16B16_SNORM:
  case VK_FORMAT_R16G16B16_UINT:
  case VK_FORMAT_R16G16B16_USCALED:
  case VK_FORMAT_R16G16B16_UNORM:
  case VK_FORMAT_R16G16B16_SSCALED:
  case VK_FORMAT_R16G16_SINT:
  case VK_FORMAT_R16G16_SNORM:
  case VK_FORMAT_R16G16_UINT:
  case VK_FORMAT_R16G16_USCALED:
  case VK_FORMAT_R16G16_UNORM:
  case VK_FORMAT_R16G16_SSCALED:
  case VK_FORMAT_R16_SINT:
  case VK_FORMAT_R16_SNORM:
  case VK_FORMAT_R16_UINT:
  case VK_FORMAT_R16_USCALED:
  case VK_FORMAT_R16_UNORM:
  case VK_FORMAT_R16_SSCALED:
  case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
    return 0;
  default:
    assert("Format Not Handled yet");
  }
  return 0;
}

unsigned m_FormatDepthBits(VkFormat format) {
  switch (format) {
  case VK_FORMAT_D16_UNORM:
  case VK_FORMAT_D16_UNORM_S8_UINT:
    return 16;
  case VK_FORMAT_X8_D24_UNORM_PACK32:
  case VK_FORMAT_D24_UNORM_S8_UINT:
    return 24;
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
  case VK_FORMAT_D32_SFLOAT:
    return 32;
  case VK_FORMAT_S8_UINT:
    return 0;
  default:
    assert("Format Not Handled yet");
  }
  return 0;
}
unsigned m_FormatStencilBits(VkFormat format) {
  switch (format) {

  case VK_FORMAT_D16_UNORM_S8_UINT:
  case VK_FORMAT_S8_UINT:
  case VK_FORMAT_D24_UNORM_S8_UINT:
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
    return 8;
  case VK_FORMAT_X8_D24_UNORM_PACK32:
  case VK_FORMAT_D16_UNORM:
  case VK_FORMAT_D32_SFLOAT:
    return 0;
  default:
    assert("Format Not Handled yet");
  }
  return 0;
}
} // namespace vkw