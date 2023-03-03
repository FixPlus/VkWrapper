#ifndef VKWRAPPER_HOSTALLOCATOR_HPP
#define VKWRAPPER_HOSTALLOCATOR_HPP

#include <vkw/ReferenceGuard.hpp>

#include <memory>
#include <vulkan/vulkan.h>

namespace vkw {

class HostAllocator : public ReferenceGuard {
public:
  explicit HostAllocator(bool enabled) noexcept(ExceptionsDisabled);

  VkAllocationCallbacks const *allocator() noexcept {
    return m_allocator.get();
  }

  virtual ~HostAllocator() = default;

protected:
  virtual void *allocate(size_t size, size_t alignment,
                         VkSystemAllocationScope scope) noexcept;

  virtual void *reallocate(void *original, size_t size, size_t alignment,
                           VkSystemAllocationScope scope) noexcept;

  virtual void free(void *memory) noexcept;

  virtual void
  internalAllocNotify(size_t size, VkInternalAllocationType allocationType,
                      VkSystemAllocationScope allocationScope) noexcept {}

  virtual void
  internalFreeNotify(size_t size, VkInternalAllocationType allocationType,
                     VkSystemAllocationScope allocationScope) noexcept {}

private:
  static void *m_allocate(void *m_this, size_t size, size_t alignment,
                          VkSystemAllocationScope scope) noexcept;

  static void *m_reallocate(void *m_this, void *original, size_t size,
                            size_t alignment,
                            VkSystemAllocationScope scope) noexcept;

  static void m_free(void *m_this, void *memory) noexcept;

  static void
  m_internalAllocNotify(void *m_this, size_t size,
                        VkInternalAllocationType allocationType,
                        VkSystemAllocationScope allocationScope) noexcept;

  static void
  m_internalFreeNotify(void *m_this, size_t size,
                       VkInternalAllocationType allocationType,
                       VkSystemAllocationScope allocationScope) noexcept;

  std::unique_ptr<VkAllocationCallbacks> m_allocator;
};
} // namespace vkw
#endif // VKWRAPPER_HOSTALLOCATOR_HPP
