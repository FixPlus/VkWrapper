#ifndef VKRENDERER_IMAGE_HPP
#define VKRENDERER_IMAGE_HPP

#include "Allocation.hpp"
#include "Common.hpp"
#include <vector>

namespace vkw {

class ImageView {
public:
  ImageInterface &parent() { return *m_parent; }

  ImageInterface const &parent() const { return *m_parent; }

  operator VkImageView() const { return m_imageView; };

  virtual ~ImageView() = default;

private:
  ImageView(VkImageView view, VkImageViewCreateInfo createInfo,
            ImageInterface *parent)
      : m_imageView(view), m_parent(parent), m_createInfo(createInfo) {}

  friend class ImageInterface;

  VkImageViewCreateInfo m_createInfo;
  VkImageView m_imageView;
  ImageInterface *m_parent;
};

class ImageInterface {
public:
  ImageInterface(Device &device, VkImage image = VK_NULL_HANDLE,
                 VkImageCreateInfo createInfo = {})
      : m_device(device), m_image(image), m_createInfo(createInfo){};

  ImageInterface(ImageInterface const &another) = delete;
  ImageInterface(ImageInterface &&another)
      : m_image(another.m_image), m_createInfo(another.m_createInfo),
        m_viewsCache(another.m_viewsCache), m_device(another.m_device) {
    another.m_viewsCache.clear();
  }

  ImageInterface const &operator=(ImageInterface const &another) = delete;
  ImageInterface &operator=(ImageInterface &&another) = delete;

  virtual ~ImageInterface();

  VkExtent3D extents() const { return m_createInfo.extent; }

  VkImageUsageFlags usage() const { return m_createInfo.usage; }

  VkFormat format() const { return m_createInfo.format; }

  VkImageType type() const { return m_createInfo.imageType; }

  operator VkImage() const { return m_image; }

  ImageView getView(VkImageViewCreateInfo viewInfo);

protected:
  VkImage m_image = VK_NULL_HANDLE;
  VkImageCreateInfo m_createInfo;
  Device const &m_device;

private:
  std::vector<ImageView> m_viewsCache;
};

class ImageBase : public Allocation, public ImageInterface {
public:
  ImageBase(Device &device, VmaAllocator allocator,
            VkImageCreateInfo createInfo,
            VmaAllocationCreateInfo allocCreateInfo);
  ImageBase(ImageBase &&another) noexcept
      : Allocation(another.m_allocator), ImageInterface(std::move(another)) {
    another.m_image = VK_NULL_HANDLE;
  }

  ImageBase(ImageBase const &another) = delete;
  ImageBase const &operator=(ImageBase const &another) = delete;
  ImageBase &operator=(ImageBase &&another) = delete;

  ~ImageBase() override;
};
} // namespace vkw
#endif // VKRENDERER_IMAGE_HPP
