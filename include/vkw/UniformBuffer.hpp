#ifndef VKWRAPPER_UNIFORMBUFFER_HPP
#define VKWRAPPER_UNIFORMBUFFER_HPP
#include "Buffer.hpp"

namespace vkw {

template <typename T> class UniformBuffer : virtual public Buffer<T> {
public:
  UniformBuffer(Device &device, VmaAllocationCreateInfo const &createInfo,
                VkBufferUsageFlags usage = 0)
      : Buffer<T>(device, 1, usage | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                  createInfo) {}

private:
};

template <typename T> class StorageBuffer : virtual public Buffer<T> {
public:
  StorageBuffer(Device &device, VmaAllocationCreateInfo const &createInfo,
                VkBufferUsageFlags usage = 0)
      : Buffer<T>(device, 1, usage | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                  createInfo) {}

private:
};

} // namespace vkw
#endif // VKWRAPPER_UNIFORMBUFFER_HPP
