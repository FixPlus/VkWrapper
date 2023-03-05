#ifndef VKRENDERER_ALLOCATION_HPP
#define VKRENDERER_ALLOCATION_HPP

#include "vma/vk_mem_alloc.h"
#include <vkw/Exception.hpp>

#include <boost/container/small_vector.hpp>
#include <span>

namespace vkw {

class Device;

class Allocation {
public:
  bool mappable() const noexcept;

  bool coherent() const noexcept;

  auto allocationSize() const noexcept { return m_allocInfo.size; }

  template <typename T> std::span<T> mapped() const noexcept {
    auto *ptr = reinterpret_cast<T *>(m_allocInfo.pMappedData);
    auto count = m_allocInfo.pMappedData ? m_allocInfo.size / sizeof(T) : 0;
    return {ptr, ptr + count};
  }

  void map() noexcept(ExceptionsDisabled);

  void unmap();

  void flush(VkDeviceSize offset = 0,
             VkDeviceSize size = VK_WHOLE_SIZE) noexcept(ExceptionsDisabled);

  void
  invalidate(VkDeviceSize offset = 0,
             VkDeviceSize size = VK_WHOLE_SIZE) noexcept(ExceptionsDisabled);

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
  explicit Allocation(VmaAllocator parent) noexcept : m_allocator(parent){};

  VmaAllocator m_allocator;
  VmaAllocation m_allocation = VK_NULL_HANDLE;
  VmaAllocationInfo m_allocInfo{};
};

class SharingInfo {
public:
  SharingInfo() = default;
  auto sharingMode() const { return m_sharingMode; }

  std::span<unsigned const> queueFamilies() const {
    return {m_queueFamilies.data(), m_queueFamilies.size()};
  }

  SharingInfo &addQueueFamily(unsigned index) {
    m_queueFamilies.push_back(index);
    m_sharingMode = VK_SHARING_MODE_CONCURRENT;
    return *this;
  }

private:
  VkSharingMode m_sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  boost::container::small_vector<unsigned, 3> m_queueFamilies;
};

} // namespace vkw
#endif // VKRENDERER_ALLOCATION_HPP
