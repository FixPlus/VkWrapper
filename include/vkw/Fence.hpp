#ifndef VKRENDERER_FENCE_HPP
#define VKRENDERER_FENCE_HPP

#include <concepts>
#include <vkw/Device.hpp>

namespace vkw {

class Fence;
template <typename T>
concept FenceIterator = std::forward_iterator<T> && requires(T a) {
  { *a } -> std::same_as<Fence &>;
};

class Fence : public UniqueVulkanObject<VkFence> {
public:
  Fence(Device const &device, bool createSignaled = false);

  void reset();

  // returns true if fence condition satisfied before timeout

  bool wait(uint64_t timeout = UINT64_MAX) {
    auto h = handle();
    return wait_impl(parent(), &h, 1, true, timeout);
  }

  bool signaled() const;

  template <FenceIterator Iter>
  static bool wait_any(Iter begin, Iter end, uint64_t timeout = UINT64_MAX) {
    boost::container::small_vector<VkFence, 4> fences{};
    for (auto it = begin; it != end; ++it) {
      fences.push_back((*it).m_fence);
    }
    return wait_impl(begin->m_device, fences.data(), fences.size(), false,
                     timeout);
  }

  template <FenceIterator Iter>
  static bool wait_all(Iter begin, Iter end, uint64_t timeout = UINT64_MAX) {
    boost::container::small_vector<VkFence, 4> fences{};
    for (auto it = begin; it != end; ++it) {
      fences.push_back((*it).m_fence);
    }
    return wait_impl(begin->m_device, fences.data(), fences.size(), true,
                     timeout);
  }

private:
  static bool wait_impl(Device const &device, VkFence const *pFences,
                        uint32_t fenceCount, bool waitAll, uint64_t timeout);
};

} // namespace vkw
#endif // VKRENDERER_FENCE_HPP
