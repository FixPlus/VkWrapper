#ifndef VKRENDERER_IMAGE_HPP
#define VKRENDERER_IMAGE_HPP

#include "Allocation.hpp"
#include "Common.hpp"
#include <memory>
#include <vector>

namespace vkw {

class ImageView {
public:
  ImageView(ImageView const &another) = delete;
  ImageView(ImageView &&another)
      : m_createInfo(another.m_createInfo), m_imageView(another.m_imageView),
        m_parent(another.m_parent) {
    another.m_imageView = VK_NULL_HANDLE;
  }

  ImageView const &operator=(ImageView const &another) = delete;
  ImageView &operator=(ImageView &&another) noexcept {
    m_imageView = another.m_imageView;
    m_createInfo = another.m_createInfo;
    m_parent = another.m_parent;
    another.m_imageView = VK_NULL_HANDLE;
    return *this;
  }

  bool operator==(ImageView const &another) const;

  ImageInterface const *image() const { return m_parent; }

  operator VkImageView() const { return m_imageView; };

  VkFormat format() const { return m_createInfo.format; };

  virtual ~ImageView() = default;

protected:
  explicit ImageView(ImageInterface const *image = nullptr,
                     VkFormat format = VK_FORMAT_MAX_ENUM,
                     uint32_t baseMipLevel = 0, uint32_t levelCount = 1,
                     VkComponentMapping componentMapping = {},
                     VkImageViewCreateFlags flags = 0);
  VkImageViewCreateInfo m_createInfo{};
  VkImageView m_imageView{};

private:
  ImageInterface const *m_parent{};
};

class ImageViewCreator : virtual public ImageView {
public:
  ~ImageViewCreator() override;

protected:
  explicit ImageViewCreator(Device const &device);

private:
  DeviceCRef m_device;
};

class ColorImageView : virtual public ImageView {
public:
  using ImageAspect = ColorImageInterface;

protected:
  ColorImageView() {
    m_createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_COLOR_BIT;
  }
};

class DepthImageView : virtual public ImageView {
public:
  using ImageAspect = DepthStencilImageInterface;

protected:
  DepthImageView() {
    m_createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
  }
};

class StencilImageView : virtual public ImageView {
public:
  using ImageAspect = DepthStencilImageInterface;

protected:
  StencilImageView() {
    m_createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }
};

class DepthStencilImageView : virtual public ImageView {
public:
  using ImageAspect = DepthStencilImageInterface;

protected:
  DepthStencilImageView() {
    m_createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    m_createInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_DEPTH_BIT;
  }
};

class Image1DView : virtual public ImageView {
public:
  uint32_t baseLayer() const {
    return m_createInfo.subresourceRange.baseArrayLayer;
  }

protected:
  Image1DView(uint32_t baseLayer) {
    m_createInfo.viewType = VK_IMAGE_VIEW_TYPE_1D;
    m_createInfo.subresourceRange.baseArrayLayer = baseLayer;
    m_createInfo.subresourceRange.layerCount = 1;
  }
};

class Image1DArrayView : virtual public ImageView {
public:
  uint32_t layers() const { return m_createInfo.subresourceRange.layerCount; }
  uint32_t baseLayer() const {
    return m_createInfo.subresourceRange.baseArrayLayer;
  }

protected:
  Image1DArrayView(uint32_t baseLayer, uint32_t layerCount) {
    m_createInfo.viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
    m_createInfo.subresourceRange.baseArrayLayer = baseLayer;
    m_createInfo.subresourceRange.layerCount = layerCount;
  }

private:
};

class Image2DView : virtual public ImageView {
public:
  uint32_t baseLayer() const {
    return m_createInfo.subresourceRange.baseArrayLayer;
  }

protected:
  Image2DView(uint32_t baseLayer) {
    m_createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    m_createInfo.subresourceRange.baseArrayLayer = baseLayer;
    m_createInfo.subresourceRange.layerCount = 1;
  };
};

class Image2DArrayView : virtual public ImageView {
public:
  uint32_t layers() const { return m_createInfo.subresourceRange.layerCount; }
  uint32_t baseLayer() const {
    return m_createInfo.subresourceRange.baseArrayLayer;
  }

protected:
  Image2DArrayView(uint32_t baseLayer, uint32_t layerCount) {
    m_createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    m_createInfo.subresourceRange.baseArrayLayer = baseLayer;
    m_createInfo.subresourceRange.layerCount = layerCount;
  }
};

class Image3DView : virtual public ImageView {
protected:
  Image3DView() {
    m_createInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
    m_createInfo.subresourceRange.layerCount = 1;
    m_createInfo.subresourceRange.baseArrayLayer = 0;
  };
};

class ImageCubeView : virtual public ImageView {
public:
  uint32_t baseLayer() const {
    return m_createInfo.subresourceRange.baseArrayLayer / 6;
  }

protected:
  ImageCubeView(uint32_t baseLayer) {
    m_createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    m_createInfo.subresourceRange.baseArrayLayer = baseLayer * 6;
    m_createInfo.subresourceRange.layerCount = 6;
  };
};

class ImageCubeArrayView : virtual public ImageView {
public:
  uint32_t layers() const { return m_createInfo.subresourceRange.layerCount; }
  uint32_t baseLayer() const {
    return m_createInfo.subresourceRange.baseArrayLayer;
  }

protected:
  ImageCubeArrayView(uint32_t baseLayer, uint32_t layerCount) {
    m_createInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
    m_createInfo.subresourceRange.baseArrayLayer = baseLayer * 6;
    m_createInfo.subresourceRange.layerCount = layerCount * 6;
  }
};

class ImageInterface {
public:
  explicit ImageInterface(VkImage image = VK_NULL_HANDLE,
                          VkImageUsageFlags usage = 0)
      : m_image(image) {
    m_createInfo.usage = usage;
    m_createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    m_createInfo.pNext = nullptr;
    m_createInfo.arrayLayers =
        1; // default parameter, maybe overridden by child classes
  };

  ImageInterface(ImageInterface const &another) = delete;
  ImageInterface(ImageInterface &&another)
      : m_image(another.m_image), m_createInfo(another.m_createInfo),
        m_viewsCache(std::move(another.m_viewsCache)) {}

  ImageInterface const &operator=(ImageInterface const &another) = delete;
  ImageInterface &operator=(ImageInterface &&another) = delete;

  virtual ~ImageInterface() = default;

  VkImageUsageFlags usage() const { return m_createInfo.usage; }

  VkFormat format() const { return m_createInfo.format; }

  VkExtent3D rawExtents() const { return m_createInfo.extent; }

  VkImageType type() const { return m_createInfo.imageType; }

  uint32_t mipLevels() const { return m_createInfo.mipLevels;}

  VkImageSubresourceRange completeSubresourceRange() const;

  operator VkImage() const { return m_image; }

protected:
  ImageView *m_cacheView(std::unique_ptr<ImageView> view);
  VkImage m_image;
  VkImageCreateInfo m_createInfo{};

private:
  std::vector<std::unique_ptr<ImageView>> m_viewsCache;
};

/** Incomplete Image Interfaces. */
class ColorImageInterface : virtual public ImageInterface {
public:
  uint32_t redBits() const;
  uint32_t greenBits() const;
  uint32_t blueBits() const;
  uint32_t alphaBits() const;

  using ViewType = ColorImageView;

protected:
  explicit ColorImageInterface(VkFormat colorFormat) {
    m_createInfo.format = colorFormat;
  };
};

class DepthStencilImageInterface : virtual public ImageInterface {
public:
  uint32_t depthBits() const;
  uint32_t stencilBits() const;

  using ViewType = DepthStencilImageView;

protected:
  explicit DepthStencilImageInterface(VkFormat depthStencilFormat) {
    m_createInfo.format = depthStencilFormat;
  }
};

/** Complete Image View classes. **/

template <ImageViewAspectInterface T>
class T_Image1DView : public T, public Image1DView, public ImageViewCreator {
private:
  using CompatibleImageType = T_Image1DInterface<typename T::ImageAspect>;
  using CompatibleArrayedImageType =
      T_Image1DArrayInterface<typename T::ImageAspect>;
  friend CompatibleImageType;
  friend CompatibleArrayedImageType;

  T_Image1DView(Device const &device, CompatibleImageType const *image,
                VkFormat format, VkComponentMapping componentMapping,
                uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image1DView(0){};

  T_Image1DView(Device const &device, CompatibleArrayedImageType const *image,
                VkFormat format, VkComponentMapping componentMapping,
                uint32_t baseLayer = 0, uint32_t baseMipLevel = 0,
                uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image1DView(baseLayer){

                                  };
};

template <ImageViewAspectInterface T>
class T_Image1DArrayView : public T,
                           public Image1DArrayView,
                           public ImageViewCreator {
private:
  using CompatibleArrayedImageType =
      T_Image1DArrayInterface<typename T::ImageAspect>;
  friend CompatibleArrayedImageType;

  T_Image1DArrayView(Device const &device,
                     CompatibleArrayedImageType const *image, VkFormat format,
                     VkComponentMapping componentMapping,
                     uint32_t baseLayer = 0, uint32_t layerCount = 1,
                     uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                     VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image1DArrayView(baseLayer, layerCount){

                                  };
};

template <ImageViewAspectInterface T>
class T_Image2DView : public T, public Image2DView, public ImageViewCreator {
private:
  using CompatibleImageType1 = T_Image2DInterface<typename T::ImageAspect>;
  using CompatibleImageType2 = T_Image3DInterface<typename T::ImageAspect>;
  using CompatibleArrayedImageType =
      T_Image2DArrayInterface<typename T::ImageAspect>;
  friend CompatibleImageType1;
  friend CompatibleImageType2;
  friend CompatibleArrayedImageType;

  T_Image2DView(Device const &device, CompatibleImageType1 const *image,
                VkFormat format, VkComponentMapping componentMapping,
                uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image2DView(0) {}

  T_Image2DView(Device const &device, CompatibleArrayedImageType const *image,
                VkFormat format, VkComponentMapping componentMapping,
                uint32_t baseLayer = 0, uint32_t baseMipLevel = 0,
                uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image2DView(baseLayer){

                                  };

  T_Image2DView(Device const &device, CompatibleImageType2 const *image,
                VkFormat format, VkComponentMapping componentMapping,
                uint32_t baseLayer = 0, uint32_t baseMipLevel = 0,
                uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image2DView(baseLayer){

                                  };
};

template <ImageViewAspectInterface T>
class T_Image2DArrayView : public T,
                           public Image2DArrayView,
                           public ImageViewCreator {
private:
  using CompatibleImageType = T_Image3DInterface<typename T::ImageAspect>;
  using CompatibleImageType2 = T_Image2DInterface<typename T::ImageAspect>;
  using CompatibleArrayedImageType =
      T_Image2DArrayInterface<typename T::ImageAspect>;
  friend CompatibleImageType;
  friend CompatibleImageType2;
  friend CompatibleArrayedImageType;

  T_Image2DArrayView(Device const &device, VkFormat format,
                     CompatibleArrayedImageType const *image,
                     VkComponentMapping componentMapping,
                     uint32_t baseLayer = 0, uint32_t layerCount = 1,
                     uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                     VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image2DArrayView(baseLayer, layerCount){

                                  };

  T_Image2DArrayView(Device const &device, CompatibleImageType const *image,
                     VkFormat format, VkComponentMapping componentMapping,
                     uint32_t baseLayer = 0, uint32_t layerCount = 1,
                     uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                     VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image2DArrayView(baseLayer, layerCount){

                                  };

  T_Image2DArrayView(Device const &device, CompatibleImageType2 const *image,
                     VkFormat format, VkComponentMapping componentMapping,
                     uint32_t baseLayer = 0, uint32_t layerCount = 1,
                     uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                     VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image2DArrayView(baseLayer, layerCount){

                                  };
};

template <ImageViewAspectInterface T>
class T_Image3DView : public T, public Image3DView, public ImageViewCreator {
private:
  using CompatibleImageType = T_Image3DInterface<typename T::ImageAspect>;
  friend CompatibleImageType;

  T_Image3DView(Device const &device, CompatibleImageType const *image,
                VkFormat format, VkComponentMapping componentMapping,
                uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), Image3DView(){

                                  };
};

template <ImageViewAspectInterface T>
class T_ImageCubeView : public T,
                        public ImageCubeView,
                        public ImageViewCreator {
private:
  using CompatibleImageType = T_ImageCubeInterface<typename T::ImageAspect>;
  using CompatibleArrayedImageType =
      T_ImageCubeArrayInterface<typename T::ImageAspect>;
  friend CompatibleImageType;
  friend CompatibleArrayedImageType;

  T_ImageCubeView(Device const &device, CompatibleImageType const *image,
                  VkFormat format, VkComponentMapping componentMapping,
                  uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                  VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), ImageCubeView(0){

                                  };

  T_ImageCubeView(Device const &device, CompatibleArrayedImageType const *image,
                  VkFormat format, uint32_t baseLayer,
                  VkComponentMapping componentMapping,
                  uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                  VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), ImageCubeView(baseLayer){

                                  };
};

template <ImageViewAspectInterface T>
class T_ImageCubeArrayView : public T,
                             public ImageCubeArrayView,
                             public ImageViewCreator {
private:
  using CompatibleArrayedImageType =
      T_ImageCubeArrayInterface<typename T::ImageAspect>;
  friend CompatibleArrayedImageType;

  T_ImageCubeArrayView(Device const &device,
                       CompatibleArrayedImageType const *image, VkFormat format,
                       uint32_t baseLayer, uint32_t layerCount,
                       VkComponentMapping componentMapping,
                       uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                       VkImageViewCreateFlags flags = 0)
      : ImageView(image, format, baseMipLevel, mipLevelCount, componentMapping,
                  flags),
        ImageViewCreator(device), ImageCubeArrayView(baseLayer, layerCount){

                                  };
};

class ImageArrayInterface : virtual public ImageInterface {
public:
  uint32_t arrayLayers() const { return m_createInfo.arrayLayers; }

protected:
  explicit ImageArrayInterface(uint32_t arrayLayers) {
    m_createInfo.arrayLayers = arrayLayers;
  }
};

class Image1DInterface : virtual public ImageInterface {
public:
  uint32_t width() const { return m_createInfo.extent.width; }

protected:
  explicit Image1DInterface(uint32_t width) {
    m_createInfo.imageType = VK_IMAGE_TYPE_1D;
    m_createInfo.extent = {width, 1, 1};
  }
};

class Image1DArrayInterface : public Image1DInterface,
                              public ImageArrayInterface {
protected:
  Image1DArrayInterface(uint32_t width, uint32_t layers)
      : Image1DInterface(width), ImageArrayInterface(layers) {}
};

class Image2DInterface : virtual public ImageInterface {
public:
  uint32_t width() const { return m_createInfo.extent.width; }
  uint32_t height() const { return m_createInfo.extent.height; }
  VkExtent2D extents() const { return {width(), height()}; }

protected:
  Image2DInterface(uint32_t width, uint32_t height) {
    m_createInfo.imageType = VK_IMAGE_TYPE_2D;
    m_createInfo.extent = {width, height, 1};
  }
};

class Image2DArrayInterface : public Image2DInterface,
                              public ImageArrayInterface {
protected:
  Image2DArrayInterface(uint32_t width, uint32_t height, uint32_t layers)
      : Image2DInterface(width, height), ImageArrayInterface(layers) {}
};

class ImageCubeInterface : public Image2DArrayInterface {
protected:
  explicit ImageCubeInterface(uint32_t cubeSize)
      : Image2DArrayInterface(cubeSize, cubeSize, 6) {
    m_createInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
  }
};

class ImageCubeArrayInterface : public ImageCubeInterface {
public:
  uint32_t cubeArrayLayers() const { return m_createInfo.arrayLayers / 6; }

protected:
  ImageCubeArrayInterface(uint32_t cubeSize, uint32_t arrayLayers)
      : ImageCubeInterface(cubeSize) {
    m_createInfo.arrayLayers *= arrayLayers;
  }
};

class Image3DInterface : virtual public ImageInterface {
public:
  uint32_t width() const { return m_createInfo.extent.width; }
  uint32_t height() const { return m_createInfo.extent.height; }
  uint32_t depth() const { return m_createInfo.extent.depth; }
  VkExtent3D extents() const { return {width(), height(), depth()}; }

protected:
  Image3DInterface(uint32_t width, uint32_t height, uint32_t depth) {
    m_createInfo.imageType = VK_IMAGE_TYPE_3D;
    m_createInfo.extent = {width, height, depth};
  }
};

class ImageRestInterface : virtual public ImageInterface {
protected:
  ImageRestInterface(VkSampleCountFlagBits samples, uint32_t mipLevels,
                     VkImageUsageFlags usage, VkImageCreateFlags flags,
                     VkImageLayout initialLayout, VkImageTiling tiling,
                     VkSharingMode sharingMode, uint32_t queueFamilyIndexCount,
                     uint32_t const *pQueueFamilyIndices) {
    m_createInfo.usage = usage;
    m_createInfo.flags = flags;
    m_createInfo.initialLayout = initialLayout;
    m_createInfo.samples = samples;
    m_createInfo.tiling = tiling;
    m_createInfo.mipLevels = mipLevels;
    m_createInfo.sharingMode = sharingMode;
    m_createInfo.queueFamilyIndexCount = queueFamilyIndexCount;
    m_createInfo.pQueueFamilyIndices = pQueueFamilyIndices;
  }
};

/** ***************** Complete image Interfaces. *****************************/

template <ImageViewAspectInterface T> class T_Image1DView;

template <ImageAspectInterface T>
class T_Image1DInterface : public T,
                           public Image1DInterface,
                           public ImageRestInterface {
public:
  /** Available views for basic 1D image */
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image1DView<U>
  const &getView(Device const &device, VkFormat format,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image1DView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{
            new T_Image1DView<U>{device, this, format, componentMapping,
                                 baseMipLevel, mipLevelCount, flags}}));
  }

protected:
  T_Image1DInterface(VkFormat colorFormat, uint32_t width)
      : T(colorFormat), Image1DInterface(width) {}
};

template <ImageAspectInterface T>
class T_Image2DInterface : public T, public Image2DInterface {
public:
  /** Available views for basic 2D image */
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image2DView<U>
  const &getView(Device const &device, VkFormat format,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {

    return *dynamic_cast<T_Image2DView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{
            new T_Image2DView<U>{device, this, format, componentMapping,
                                 baseMipLevel, mipLevelCount, flags}}));
  }

  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image2DArrayView<U>
  const &getView(Device const &device, VkComponentMapping componentMapping,
                 VkFormat format, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image2DArrayView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{
            new T_Image2DArrayView<U>{device, this, format, componentMapping, 0,
                                      1, baseMipLevel, mipLevelCount, flags}}));
  }

protected:
  T_Image2DInterface(VkFormat colorFormat, uint32_t width, uint32_t height)
      : T(colorFormat), Image2DInterface(width, height) {}
};

template <ImageAspectInterface T>
class T_Image3DInterface : public T, public Image3DInterface {
public:
  /** Available views for basic 3D image */
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image3DView<U>
  const &getView(Device const &device, VkFormat format,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image3DView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{
            new T_Image3DView<U>{device, this, format, componentMapping,
                                 baseMipLevel, mipLevelCount, flags}}));
  }
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image2DView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image2DView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{new T_Image2DView<U>{
            device, this, format, baseLayer, componentMapping, baseMipLevel,
            mipLevelCount, flags}}));
  }

  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image2DArrayView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 uint32_t layerCount, VkComponentMapping componentMapping,
                 uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                 VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image2DArrayView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{new T_Image2DArrayView<U>{
            device, this, format, baseLayer, layerCount, componentMapping,
            baseMipLevel, mipLevelCount, flags}}));
  }

protected:
  T_Image3DInterface(VkFormat colorFormat, uint32_t width, uint32_t height,
                     uint32_t depth)
      : T(colorFormat), Image3DInterface(width, height, depth) {}
};

template <ImageAspectInterface T>
class T_ImageCubeInterface : public T, public ImageCubeInterface {
public:
  /** Available views for basic cube image */
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_ImageCubeView<U>
  const &getView(Device const &device, VkFormat format,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_ImageCubeView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{
            new T_ImageCubeView<U>{device, this, format, componentMapping,
                                   baseMipLevel, mipLevelCount, flags}}));
  }

protected:
  T_ImageCubeInterface(VkFormat colorFormat, uint32_t cubeSize)
      : T(colorFormat), ImageCubeInterface(cubeSize) {}
};

template <ImageAspectInterface T>
class T_Image1DArrayInterface : public T, public Image1DArrayInterface {
public:
  /** Available views for basic 1D image array */
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image1DView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image1DView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{new T_Image1DView<U>{
            device, this, format, baseLayer, componentMapping, baseMipLevel,
            mipLevelCount, flags}}));
  }

  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image1DArrayView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 uint32_t layerCount, VkComponentMapping componentMapping,
                 uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                 VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image1DArrayView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{new T_Image1DArrayView<U>{
            device, this, format, baseLayer, layerCount, componentMapping,
            baseMipLevel, mipLevelCount, flags}}));
  }

protected:
  T_Image1DArrayInterface(VkFormat colorFormat, uint32_t width,
                          uint32_t layerCount)
      : T(colorFormat), Image1DArrayInterface(width, layerCount) {}
};

template <ImageAspectInterface T>
class T_Image2DArrayInterface : public T, public Image2DArrayInterface {
public:
  /** Available views for basic 2D image array */
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image2DView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image2DView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{new T_Image2DView<U>{
            device, this, format, componentMapping, baseLayer, baseMipLevel,
            mipLevelCount, flags}}));
  }
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_Image2DArrayView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 uint32_t layerCount, VkComponentMapping componentMapping,
                 uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                 VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_Image2DArrayView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{new T_Image2DArrayView<U>{
            device, format, this, componentMapping, baseLayer, layerCount,
            baseMipLevel, mipLevelCount, flags}}));
  }

protected:
  T_Image2DArrayInterface(VkFormat colorFormat, uint32_t width, uint32_t height,
                          uint32_t layerCount)
      : T(colorFormat), Image2DArrayInterface(width, height, layerCount) {}
};

template <ImageAspectInterface T>
class T_ImageCubeArrayInterface : public T, public ImageCubeArrayInterface {
public:
  /** Available views for basic cube image array */
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_ImageCubeView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 VkComponentMapping componentMapping, uint32_t baseMipLevel = 0,
                 uint32_t mipLevelCount = 1, VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_ImageCubeView<U> *>(
        m_cacheView(std::unique_ptr<ImageView>{new T_ImageCubeView<U>{
            device, this, format, baseLayer, componentMapping, baseMipLevel,
            mipLevelCount, flags}}));
  }
  template <ImageViewAspectInterface U>
  requires std::same_as<typename U::ImageAspect, T> T_ImageCubeArrayView<U>
  const &getView(Device const &device, VkFormat format, uint32_t baseLayer,
                 uint32_t layerCount, VkComponentMapping componentMapping,
                 uint32_t baseMipLevel = 0, uint32_t mipLevelCount = 1,
                 VkImageViewCreateFlags flags = 0) {
    return *dynamic_cast<T_ImageCubeArrayView<U> *>(
        m_cacheView(std::make_unique<ImageView>(T_ImageCubeArrayView<U>{
            device, this, format, baseLayer, layerCount, componentMapping,
            baseMipLevel, mipLevelCount, flags})));
  }

protected:
  T_ImageCubeArrayInterface(VkFormat colorFormat, uint32_t cubeSize,
                            uint32_t layerCount)
      : T(colorFormat), ImageCubeArrayInterface(cubeSize, layerCount) {}
};

class AllocatedImage : public Allocation, virtual public ImageInterface {
public:
  AllocatedImage(VmaAllocator allocator,
                 VmaAllocationCreateInfo allocCreateInfo);
  AllocatedImage(AllocatedImage &&another) noexcept
      : Allocation(another.m_allocator), ImageInterface(std::move(another)) {
    another.m_image = VK_NULL_HANDLE;
  }

  AllocatedImage(AllocatedImage const &another) = delete;
  AllocatedImage const &operator=(AllocatedImage const &another) = delete;
  AllocatedImage &operator=(AllocatedImage &&another) = delete;

  ~AllocatedImage() override;

protected:
};

/** ********************** Complete Image Types. ****************************/

/**
 *                            Swap chain Image
 *     This class represents the image retrieved from swap chain.
 *
 */

class SwapChainImage : public ColorImage2DArrayInterface {
public:
private:
  friend class SwapChain;
  SwapChainImage(VkImage swapChainImage, VkFormat surfaceFormat, uint32_t width,
                 uint32_t height, uint32_t layers, VkImageUsageFlags usage)
      : ImageInterface(swapChainImage),
        ColorImage2DArrayInterface(surfaceFormat, width, height, layers) {
    m_createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    m_createInfo.mipLevels = 1;
    m_createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    m_createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    m_createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    m_createInfo.usage = usage;
  }
};

/**
 *                            Staging Image
 *     This class represents the image with linear tiling layout.
 *     In Vulkan, images with such tiling are strongly limited and are
 *     capable only of transfer in/out operations. However this is the only
 *     type of images that can be mapped to host memory and be filled directly.
 *
 */
class StagingImage : public ColorImage2DInterface,
                     public ImageRestInterface,
                     private AllocatedImage {
public:
  StagingImage(VmaAllocator allocator, VkFormat colorFormat, uint32_t width,
               uint32_t height)
      : ColorImage2DInterface(colorFormat, width, height),
        ImageRestInterface(VK_SAMPLE_COUNT_1_BIT, 1,
                           VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                               VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                           0, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_TILING_LINEAR,
                           VK_SHARING_MODE_EXCLUSIVE, 0, nullptr),
        AllocatedImage(allocator,
                       {.usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
                        .requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT}) {
  }

  unsigned char *map() {
    return static_cast<unsigned char *>(AllocatedImage::map());
  }

  void unmap() { AllocatedImage::unmap(); }

  void flush() { AllocatedImage::flush(0, VK_WHOLE_SIZE); }

  void invalidate() { AllocatedImage::invalidate(0, VK_WHOLE_SIZE); }
};

class ColorImage2D : public ColorImage2DInterface,
                     public ImageRestInterface,
                     public AllocatedImage {
public:
  ColorImage2D(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo,
               VkFormat format, uint32_t width, uint32_t height,
               uint32_t mipLevels, VkImageUsageFlags usage,
               VkImageCreateFlags flags = 0)
      : ColorImage2DInterface(format, width, height),
        ImageRestInterface(VK_SAMPLE_COUNT_1_BIT, mipLevels, usage, flags,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_TILING_OPTIMAL,
                           VK_SHARING_MODE_EXCLUSIVE, 0, nullptr),
        AllocatedImage(allocator, allocCreateInfo) {}
};

template<uint32_t layers>
class ColorImage2DArray : public ColorImage2DArrayInterface,
                     public ImageRestInterface,
                     public AllocatedImage {
public:
  ColorImage2DArray(VmaAllocator allocator, VmaAllocationCreateInfo allocCreateInfo,
               VkFormat format, uint32_t width, uint32_t height,
               uint32_t mipLevels, VkImageUsageFlags usage,
               VkImageCreateFlags flags = 0)
      : ColorImage2DArrayInterface(format, width, height, layers),
        ImageRestInterface(VK_SAMPLE_COUNT_1_BIT, mipLevels, usage, flags,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_TILING_OPTIMAL,
                           VK_SHARING_MODE_EXCLUSIVE, 0, nullptr),
        AllocatedImage(allocator, allocCreateInfo) {}
};

class DepthStencilImage2D : public DepthStencilImage2DInterface,
                            public ImageRestInterface,
                            public AllocatedImage {
public:
  DepthStencilImage2D(VmaAllocator allocator,
                      VmaAllocationCreateInfo allocCreateInfo, VkFormat format,
                      uint32_t width, uint32_t height, uint32_t mipLevels,
                      VkImageUsageFlags usage, VkImageCreateFlags flags = 0)
      : DepthStencilImage2DInterface(format, width, height),
        ImageRestInterface(VK_SAMPLE_COUNT_1_BIT, mipLevels, usage, flags,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_TILING_OPTIMAL,
                           VK_SHARING_MODE_EXCLUSIVE, 0, nullptr),
        AllocatedImage(allocator, allocCreateInfo) {}
};

class DepthStencilImage2DArray : public DepthStencilImage2DArrayInterface,
                                 public ImageRestInterface,
                                 public AllocatedImage {
public:
  DepthStencilImage2DArray(VmaAllocator allocator,
                           VmaAllocationCreateInfo allocCreateInfo,
                           VkFormat format, uint32_t width, uint32_t height,
                           uint32_t layerCount, uint32_t mipLevels,
                           VkImageUsageFlags usage,
                           VkImageCreateFlags flags = 0)
      : DepthStencilImage2DArrayInterface(format, width, height, layerCount),
        ImageRestInterface(VK_SAMPLE_COUNT_1_BIT, mipLevels, usage, flags,
                           VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_TILING_OPTIMAL,
                           VK_SHARING_MODE_EXCLUSIVE, 0, nullptr),
        AllocatedImage(allocator, allocCreateInfo) {}
};
} // namespace vkw
#endif // VKRENDERER_IMAGE_HPP
