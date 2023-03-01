#ifndef VKRENDERER_COMMON_HPP
#define VKRENDERER_COMMON_HPP

#include <ranges>
#include <vkw/ReferenceGuard.hpp>

namespace vkw {
template <typename T, typename U>
concept reference_wrapper_like = requires(T ref) {
  { ref.get() } -> std::convertible_to<U &>;
};

template <typename T, typename U>
concept derived_or_reference_wrapped =
    std::same_as < typename std::remove_cv<T>::type,
typename std::remove_cv<U>::type > || std::derived_from<T, U> ||
    reference_wrapper_like<T, U>;

template <typename T, typename U>
concept forward_range_of = std::ranges::forward_range<T> &&
    derived_or_reference_wrapped<std::ranges::range_value_t<T>, U>;

namespace ranges {

  template <typename T, typename U>
  requires derived_or_reference_wrapped<T, U>
  struct raw_ref {
    constexpr static U &get(T &a) { return a; }
  };

  template <typename U> struct raw_ref<std::reference_wrapper<U>, U> {
    constexpr static U &get(std::reference_wrapper<U> a) { return a.get(); }
  };

  template <typename U> struct raw_ref<std::reference_wrapper<const U>, U> {
    constexpr static U const &get(std::reference_wrapper<const U> a) {
      return a.get();
    }
  };

  // Disclaimer: I don't use std::ranges::subrange in order this code compile on
  // older compilers

  template <typename Base, forward_range_of<Base> R> class subrange {
  public:
    using iterator = decltype(std::ranges::begin(std::declval<R const &>()));
    using value_type = std::ranges::range_value_t<R>;

    subrange(R const &r)
        : m_begin(std::ranges::begin(r)), m_end(std::ranges::end(r)) {}

    auto begin() { return m_begin; }

    auto end() { return m_end; }

    static constexpr Base const &get(value_type const &v) {
      return raw_ref<value_type const, Base const>::get(v);
    }

  private:
    iterator m_begin, m_end;
  };

  template <typename U>
  subrange(U const &r) -> subrange<std::ranges::range_value_t<U>, U>;

  template <typename Base, typename R>
  subrange<Base, R> make_subrange(R const &r) {
    return subrange<Base, R>(r);
  }
} // namespace ranges

} // namespace vkw
#endif // VKRENDERER_COMMON_HPP
