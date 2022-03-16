#ifndef VKRENDERER_BUFFER_HPP
#define VKRENDERER_BUFFER_HPP
#include "Allocation.hpp"
#include <memory>

namespace vkw {

class BufferBase : public Allocation {
public:
  BufferBase(VmaAllocator allocator, VkBufferCreateInfo const &createInfo,
             VmaAllocationCreateInfo const &allocCreateInfo);

  BufferBase(BufferBase const &another) = delete;
  BufferBase(BufferBase &&another) noexcept
      : Allocation(another.m_allocator), m_buffer(another.m_buffer),
        m_createInfo(another.m_createInfo) {
    m_allocInfo = another.m_allocInfo;
    m_allocation = another.m_allocation;
    another.m_buffer = VK_NULL_HANDLE;
  }

  BufferBase const &operator=(BufferBase const &another) = delete;
  BufferBase &operator=(BufferBase &&another) noexcept {
    m_allocator = another.m_allocator;
    m_allocInfo = another.m_allocInfo;
    m_buffer = another.m_buffer;
    m_createInfo = another.m_createInfo;
    m_allocation = another.m_allocation;
    another.m_buffer = VK_NULL_HANDLE;
    return *this;
  }

  ~BufferBase() override;

  operator VkBuffer() const { return m_buffer; }

  VkBufferUsageFlags usage() const { return m_createInfo.usage; }

  bool canBeCopySrc() const {
    return m_createInfo.usage & VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  }

  bool canBeCopyDst() const {
    return m_createInfo.usage & VK_BUFFER_USAGE_TRANSFER_DST_BIT;
  }

private:
  VkBufferCreateInfo m_createInfo;
  VkBuffer m_buffer = VK_NULL_HANDLE;
};

class Device;

std::unique_ptr<BufferBase>
createBufferBaseOnDevice(Device &device,
                         VkBufferCreateInfo const &bufferCreateInfo,
                         VmaAllocationCreateInfo const &allocCreateInfo);

template <typename T> class Buffer {
public:
  Buffer(Device &device, uint64_t count, VkBufferUsageFlags usage,
         VmaAllocationCreateInfo const &allocCreateInfo)
      : m_count(count), m_device(device) {
    VkBufferCreateInfo createInfo{};
    createInfo.size = count * sizeof(T);
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.usage = usage;
    createInfo.pNext = nullptr;
    m_buffer = createBufferBaseOnDevice(device, createInfo, allocCreateInfo);
  }
  Buffer(Buffer const &another) = delete;
  Buffer(Buffer &&another) noexcept
      : m_device(another.m_device), m_count(another.m_count),
        m_buffer(std::move(another.m_buffer)) {}
  Buffer const &operator=(Buffer const &another) = delete;
  Buffer &operator=(Buffer &&another) noexcept {
    m_device = std::move(another.m_device);
    m_buffer = std::move(another.m_buffer);
    m_count = another.m_count;
    return *this;
  }

  bool mappable() const { return m_buffer->mappable(); }

  bool coherent() const { return m_buffer->coherent(); }

  T *map() { return reinterpret_cast<T *>(m_buffer->map()); }

  void unmap() { m_buffer->unmap(); }

  void flush(VkDeviceSize size = VK_WHOLE_SIZE,
             VkDeviceSize offset = VK_WHOLE_SIZE) {
    m_buffer->flush(offset, size);
  }

  void invalidate(VkDeviceSize size = VK_WHOLE_SIZE,
                  VkDeviceSize offset = VK_WHOLE_SIZE) {
    m_buffer->invalidate(offset, size);
  }

  uint64_t size() const { return m_count; }

  operator BufferBase &() { return *m_buffer; }

  operator BufferBase const &() const { return *m_buffer; }

  virtual ~Buffer() = default;

protected:
  std::reference_wrapper<Device> m_device;

private:
  uint64_t m_count;
  std::unique_ptr<BufferBase> m_buffer;
};

} // namespace vkw
#endif // VKRENDERER_BUFFER_HPP
