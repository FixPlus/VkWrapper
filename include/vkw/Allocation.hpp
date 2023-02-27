#ifndef VKRENDERER_ALLOCATION_HPP
#define VKRENDERER_ALLOCATION_HPP

#include "vma/vk_mem_alloc.h"
#include <span>

namespace vkw {

class Device;

class Allocation {
public:
  bool mappable() const;

  bool coherent() const;

  auto allocationSize() const { return m_allocInfo.size; }

  template <typename T> std::span<T> mapped() const {
    auto *ptr = reinterpret_cast<T *>(m_allocInfo.pMappedData);
    auto count = m_allocInfo.pMappedData ? m_allocInfo.size / sizeof(T) : 0;
    return {ptr, ptr + count};
  }

  void map();

  void unmap();

  void flush(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

  void invalidate(VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);

  Allocation(Allocation &&another) noexcept
      : m_allocator(another.m_allocator), m_allocation(another.m_allocation),
        m_allocInfo(another.m_allocInfo) {
    another.m_allocInfo.pMappedData = nullptr;
  }
  Allocation(Allocation const &another) = delete;

  Allocation &operator=(Allocation &&another) noexcept {
    std::swap(m_allocator, another.m_allocator);
    std::swap(m_allocation, another.m_allocation);
    std::swap(m_allocInfo, another.m_allocInfo);
    return *this;
  }
  Allocation &operator=(Allocation const &another) = delete;

  virtual ~Allocation() = default;

protected:
  explicit Allocation(VmaAllocator parent) : m_allocator(parent){};

  VmaAllocator m_allocator;
  VmaAllocation m_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo m_allocInfo{};
};
} // namespace vkw
#endif // VKRENDERER_ALLOCATION_HPP
