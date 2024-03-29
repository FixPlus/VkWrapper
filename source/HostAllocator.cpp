#include <cstdlib>
#include <cstring>
#include <vkw/HostAllocator.hpp>

namespace vkw {

HostAllocator::HostAllocator(bool enabled) noexcept(ExceptionsDisabled) {
  if (!enabled)
    return;
  m_allocator = std::make_unique<VkAllocationCallbacks>();

  m_allocator->pUserData = this;
  m_allocator->pfnAllocation = &m_allocate;
  m_allocator->pfnReallocation = &m_reallocate;
  m_allocator->pfnFree = &m_free;
  m_allocator->pfnInternalAllocation = &m_internalAllocNotify;
  m_allocator->pfnInternalFree = &m_internalFreeNotify;
}

void *HostAllocator::allocate(size_t size, size_t alignment,
                              VkSystemAllocationScope scope) noexcept {
#if _WIN32
  return _aligned_malloc(size, alignment);
#else
  return std::aligned_alloc(alignment, size);
#endif
}

void *HostAllocator::reallocate(void *original, size_t size, size_t alignment,
                                VkSystemAllocationScope scope) noexcept {
#if _WIN32
  return _aligned_realloc(original, size, alignment);
#else
  auto *newData = std::realloc(original, size);
  if ((uint64_t)newData % alignment == 0)
    return newData;

  auto *alignedData = std::aligned_alloc(alignment, size);
  memcpy(alignedData, newData, size);
  return alignedData;
#endif
}

void HostAllocator::free(void *memory) noexcept {
#if _WIN32
  return _aligned_free(memory);
#else
  return std::free(memory);
#endif
}

void *HostAllocator::m_allocate(void *m_this, size_t size, size_t alignment,
                                VkSystemAllocationScope scope) noexcept {
  return reinterpret_cast<HostAllocator *>(m_this)->allocate(size, alignment,
                                                             scope);
}

void *HostAllocator::m_reallocate(void *m_this, void *original, size_t size,
                                  size_t alignment,
                                  VkSystemAllocationScope scope) noexcept {
  return reinterpret_cast<HostAllocator *>(m_this)->reallocate(
      original, size, alignment, scope);
}

void HostAllocator::m_free(void *m_this, void *memory) noexcept {
  return reinterpret_cast<HostAllocator *>(m_this)->free(memory);
}

void HostAllocator::m_internalAllocNotify(
    void *m_this, size_t size, VkInternalAllocationType allocationType,
    VkSystemAllocationScope allocationScope) noexcept {
  reinterpret_cast<HostAllocator *>(m_this)->internalAllocNotify(
      size, allocationType, allocationScope);
}

void HostAllocator::m_internalFreeNotify(
    void *m_this, size_t size, VkInternalAllocationType allocationType,
    VkSystemAllocationScope allocationScope) noexcept {
  reinterpret_cast<HostAllocator *>(m_this)->internalFreeNotify(
      size, allocationType, allocationScope);
}
} // namespace vkw