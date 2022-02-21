#ifndef VKRENDERER_FENCE_HPP
#define VKRENDERER_FENCE_HPP

#include "Common.hpp"
#include <vulkan/vulkan.h>

namespace vkw {

class Device;

class Fence {
public:
  Fence(Device &device, bool createSignaled = false);
  Fence(Fence &&another)
      : m_device(another.m_device), m_fence(another.m_fence) {
    another.m_fence = VK_NULL_HANDLE;
  }

  virtual ~Fence();

  void reset();

  // returns true if fence condition satisfied before timeout

  bool wait(uint64_t timeout = UINT64_MAX) {
    return wait_impl(m_device, &m_fence, 1, true, timeout);
  }

  bool signaled() const;

  template <typename Iter>
  static bool wait_any(Iter begin, Iter end, uint64_t timeout = UINT64_MAX) {
    std::vector<VkFence> fences{};
    for (auto it = begin; it != end; ++it) {
      fences.push_back(*it.m_fence);
    }
    return wait_impl(begin->m_device, fences.data(), fences.size(), false,
                     timeout);
  }

  template <typename Iter>
  static bool wait_all(Iter begin, Iter end, uint64_t timeout = UINT64_MAX) {
    std::vector<VkFence> fences{};
    for (auto it = begin; it != end; ++it) {
      fences.push_back(*it.m_fence);
    }
    return wait_impl(begin->m_device, fences.data(), fences.size(), true,
                     timeout);
  }

  operator VkFence() const { return m_fence; }

private:
  Device &m_device;
  VkFence m_fence = VK_NULL_HANDLE;

  static bool wait_impl(Device &device, VkFence *pFences, uint32_t fenceCount,
                        bool waitAll, uint64_t timeout);
};

VKR_DECLARE_ARRAY_TYPES(Fence)
} // namespace vkr
#endif // VKRENDERER_FENCE_HPP
