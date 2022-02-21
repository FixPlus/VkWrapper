#ifndef VKRENDERER_COMMON_HPP
#define VKRENDERER_COMMON_HPP

#include <functional>
#include <vector>
namespace vkw {

/*
 *  class RefArray
 *
 *  Used to pass arbitrary amount of T objects by reference.
 *  Implicitly casts to pointer to UnderlyingT object array used by vulkan
 * functions. e.g. RefArray<Semaphore, VkSemaphore>
 */
template <typename T, typename UnderlyingT> class RefArray {
public:
  using TRef = std::reference_wrapper<T>;
  using IterT = typename std::vector<TRef>::const_iterator;

  RefArray(std::initializer_list<TRef> array) : m_container(array) {
    m_raw.reserve(m_container.size());
    for (auto &elem : m_container)
      m_raw.template emplace_back(elem.get());
  }
  // should be implicit to allow passing single object by reference
  RefArray(T &single) : RefArray({single}) {}

  // Range-based for operators

  IterT begin() const { return m_container.cbegin(); }

  IterT end() const { return m_container.cend(); }

  // Implicit cast to array of UnderlyingT

  operator UnderlyingT const *() const {
    if (empty())
      return nullptr;
    return m_raw.data();
  }
  operator UnderlyingT *() {
    if (empty())
      return nullptr;
    return m_raw.data();
  }

  size_t size() const { return m_container.size(); }

  bool empty() const { return m_container.empty(); }

private:
  std::vector<TRef> const m_container;
  std::vector<UnderlyingT> m_raw;
};

#define VKR_DECLARE_ARRAY_TYPES(T)                                             \
  using T##RefArray = RefArray<T, Vk##T>;                                      \
  using T##ConstRefArray = RefArray<T const, Vk##T>;

#define VKR_DECLARE_ARRAY_TYPES_NON_CONV(T, UT)                                \
  using T##RefArray = RefArray<T, UT>;                                         \
  using T##ConstRefArray = RefArray<T const, UT>;
} // namespace vkr
#endif // VKRENDERER_COMMON_HPP
