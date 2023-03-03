#ifndef VKRENDERER_BUFFER_HPP
#define VKRENDERER_BUFFER_HPP

#include <vkw/Allocation.hpp>
#include <vkw/Device.hpp>

namespace vkw {

class BufferBase : public Allocation, public ReferenceGuard {
public:
  BufferBase(VmaAllocator allocator, VkBufferCreateInfo const &createInfo,
             VmaAllocationCreateInfo const
                 &allocCreateInfo) noexcept(ExceptionsDisabled);

  BufferBase(BufferBase const &another) = delete;
  BufferBase(BufferBase &&another) noexcept
      : Allocation(std::move(another)), m_buffer(another.m_buffer),
        m_createInfo(another.m_createInfo) {
    another.m_buffer = VK_NULL_HANDLE;
  }

  BufferBase const &operator=(BufferBase const &another) = delete;
  BufferBase &operator=(BufferBase &&another) noexcept {
    Allocation::operator=(std::move(another));
    m_createInfo = another.m_createInfo;
    std::swap(m_buffer, another.m_buffer);
    return *this;
  }

  auto bufferSize() const noexcept { return m_createInfo.size; }

  ~BufferBase() override;

  operator VkBuffer() const noexcept { return m_buffer; }

  VkBufferUsageFlags usage() const noexcept { return m_createInfo.usage; }

  bool canBeCopySrc() const noexcept {
    return m_createInfo.usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  bool canBeCopyDst() const noexcept {
    return m_createInfo.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

  bool isVertexBuffer() const noexcept {
    return m_createInfo.usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  }

  bool isIndexBuffer() const noexcept {
    return m_createInfo.usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  }

  bool isUniformBuffer() const noexcept {
    return m_createInfo.usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  }

  bool isStorageBuffer() const noexcept {
    return m_createInfo.usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
  }

protected:
  VkBufferCreateInfo m_createInfo;

private:
  VkBuffer m_buffer = VK_NULL_HANDLE;
};

class Device;

template <typename T> class Buffer : public BufferBase {
public:
  Buffer(Device const &device, uint64_t count, VkBufferUsageFlags usage,
         VmaAllocationCreateInfo const
             &allocCreateInfo) noexcept(ExceptionsDisabled)
      : BufferBase(device.getAllocator(), m_fillInfo(count, usage),
                   allocCreateInfo),
        m_count(count), m_device(device) {}

  std::span<T> mapped() const noexcept { return Allocation::mapped<T>(); }

  uint64_t size() const noexcept { return m_count; }

protected:
  StrongReference<Device const> m_device;

private:
  VkBufferCreateInfo m_fillInfo(uint64_t count,
                                VkBufferUsageFlags usage) noexcept {
    VkBufferCreateInfo createInfo{};
    createInfo.size = count * sizeof(T);
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.usage = usage;
    createInfo.pNext = nullptr;
    return createInfo;
  }
  uint64_t m_count;
};

} // namespace vkw
#endif // VKRENDERER_BUFFER_HPP
