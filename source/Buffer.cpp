#include "vkw/Buffer.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

BufferBase::BufferBase(
    VmaAllocator allocator, VkBufferCreateInfo const &createInfo,
    VmaAllocationCreateInfo const &allocCreateInfo) noexcept(ExceptionsDisabled)
    : Allocation(allocator), m_createInfo(createInfo) {

  VK_CHECK_RESULT(vmaCreateBuffer(m_allocator, &createInfo, &allocCreateInfo,
                                  &m_buffer, &m_allocation, &m_allocInfo));
}

BufferBase::~BufferBase() {
  if (m_buffer != VK_NULL_HANDLE) {
    unmap();
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
  }
}

} // namespace vkw