#include "Utils.hpp"
#include <vkw/Allocation.hpp>

namespace vkw {

bool Allocation::mappable() const noexcept {
  const VkPhysicalDeviceMemoryProperties *pMemProps;
  vmaGetMemoryProperties(m_allocator, &pMemProps);
  auto bits = pMemProps->memoryTypes[m_allocInfo.memoryType].propertyFlags;

  return bits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ||
         bits & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
}

bool Allocation::coherent() const noexcept {
  const VkPhysicalDeviceMemoryProperties *pMemProps;
  vmaGetMemoryProperties(m_allocator, &pMemProps);
  auto bits = pMemProps->memoryTypes[m_allocInfo.memoryType].propertyFlags;

  return bits & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
}

void Allocation::unmap() {
  if (!m_allocInfo.pMappedData)
    return;
  vmaUnmapMemory(m_allocator, m_allocation);
  m_allocInfo.pMappedData = nullptr;
}

void Allocation::map() noexcept(ExceptionsDisabled) {
  if (!m_allocInfo.pMappedData)
    VK_CHECK_RESULT(
        vmaMapMemory(m_allocator, m_allocation, &m_allocInfo.pMappedData));
}

void Allocation::flush(VkDeviceSize offset,
                       VkDeviceSize size) noexcept(ExceptionsDisabled) {
  VK_CHECK_RESULT(vmaFlushAllocation(m_allocator, m_allocation, offset, size));
}

void Allocation::invalidate(VkDeviceSize offset,
                            VkDeviceSize size) noexcept(ExceptionsDisabled) {
  VK_CHECK_RESULT(
      vmaInvalidateAllocation(m_allocator, m_allocation, offset, size));
}
} // namespace vkw