#ifndef VKRENDERER_ALLOCATION_HPP
#define VKRENDERER_ALLOCATION_HPP

#include "vma/vk_mem_alloc.h"

namespace vkw {

class Device;

class Allocation {
public:
  virtual ~Allocation() = default;

  bool mappable() const {
    const VkPhysicalDeviceMemoryProperties *pMemProps;
    vmaGetMemoryProperties(m_allocator, &pMemProps);
    auto bits = pMemProps->memoryTypes[m_allocInfo.memoryType].propertyFlags;

    return bits & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ||
           bits & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }

  bool coherent() const {
    const VkPhysicalDeviceMemoryProperties *pMemProps;
    vmaGetMemoryProperties(m_allocator, &pMemProps);
    auto bits = pMemProps->memoryTypes[m_allocInfo.memoryType].propertyFlags;

    return bits & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
  }

  void *map();

  void unmap() { vmaUnmapMemory(m_allocator, m_allocation); }
  void flush(VkDeviceSize offset, VkDeviceSize size) {
    vmaFlushAllocation(m_allocator, m_allocation, offset, size);
  }

  void invalidate(VkDeviceSize offset, VkDeviceSize size) {
    vmaInvalidateAllocation(m_allocator, m_allocation, offset, size);
  }

protected:
  explicit Allocation(VmaAllocator parent) : m_allocator(parent){};

  VmaAllocator m_allocator;
  VmaAllocation m_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo m_allocInfo{};
};
} // namespace vkw
#endif // VKRENDERER_ALLOCATION_HPP
