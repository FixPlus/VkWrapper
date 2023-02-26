#include "vkw/Buffer.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

void Allocation::map() {
  if (!m_mapped)
    VK_CHECK_RESULT(vmaMapMemory(m_allocator, m_allocation, &m_mapped));
}

BufferBase::BufferBase(VmaAllocator allocator,
                       VkBufferCreateInfo const &createInfo,
                       VmaAllocationCreateInfo const &allocCreateInfo)
    : Allocation(allocator),
      m_createInfo(createInfo){

          VK_CHECK_RESULT(vmaCreateBuffer(m_allocator, &createInfo,
                                          &allocCreateInfo, &m_buffer,
                                          &m_allocation, &m_allocInfo))

      }

      BufferBase::~BufferBase() {
  if (m_buffer != VK_NULL_HANDLE) {
    unmap();
    vmaDestroyBuffer(m_allocator, m_buffer, m_allocation);
  }
}

std::unique_ptr<BufferBase>
createBufferBaseOnDevice(Device &device,
                         VkBufferCreateInfo const &bufferCreateInfo,
                         VmaAllocationCreateInfo const &allocCreateInfo) {
  return device.createBuffer(allocCreateInfo, bufferCreateInfo);
}
} // namespace vkw