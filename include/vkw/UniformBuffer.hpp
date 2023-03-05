#ifndef VKWRAPPER_UNIFORMBUFFER_HPP
#define VKWRAPPER_UNIFORMBUFFER_HPP

#include <vkw/Buffer.hpp>

namespace vkw {

template <typename T> class UniformBuffer : public Buffer<T> {
public:
  UniformBuffer(
      Device &device, VmaAllocationCreateInfo const &createInfo,
      VkBufferUsageFlags usage = 0,
      SharingInfo const &sharingInfo = {}) noexcept(ExceptionsDisabled)
      : Buffer<T>(device, 1, usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  createInfo, sharingInfo) {}

private:
};

template <typename T> class StorageBuffer : public Buffer<T> {
public:
  StorageBuffer(
      Device &device, VmaAllocationCreateInfo const &createInfo,
      VkBufferUsageFlags usage = 0,
      SharingInfo const &sharingInfo = {}) noexcept(ExceptionsDisabled)
      : Buffer<T>(device, 1, usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                  createInfo, sharingInfo) {}

private:
};

} // namespace vkw
#endif // VKWRAPPER_UNIFORMBUFFER_HPP
