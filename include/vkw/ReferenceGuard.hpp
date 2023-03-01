#ifndef VKWRAPPER_REFERENCEGUARD_HPP
#define VKWRAPPER_REFERENCEGUARD_HPP

#include <atomic>
#include <functional>
#include <vkw/Exception.hpp>

namespace vkw {

/**
 *
 * @class ReferenceGuard
 * Implements intrusive reference counter.
 * It is intended to use whenever instances of your class are
 * referenced somewhere else and those references are stored for
 * prolonged period of program runtime. Reference Guard will check
 * if such references still exists when referenced instance is
 * about to be moved or deleted. If reference counter is non-zero
 * all those pending references will eventually dangle if move or
 * destruction occur. To prevent this, Reference Guard will issue
 * irrecoverable error. Exception handling cannot be used here,
 * because throwing is not allowed in move ctors/assignments or
 * destructors.
 *
 * Apart from shared pointer this class do not enforce or implements
 * any ownership mechanisms and it's sole purpose is to check for
 * dangling references.
 *
 * @macro VKW_ENABLE_REFERENCE_GUARD
 * is used to enable reference counter. When disabled, both
 * ReferenceGuard and StrongReference<> are empty
 *
 */
#ifdef VKW_ENABLE_REFERENCE_GUARD
class ReferenceGuard {
public:
  ReferenceGuard() = default;

  void add_reference() const { m_ref_count++; }

  void remove_reference() const { m_ref_count--; }

  ReferenceGuard(ReferenceGuard const &another) {
    // Here we don't need to check anything
  }

  ReferenceGuard(ReferenceGuard &&another) noexcept {
    another.m_check_ref_count();
  }

  ReferenceGuard &operator=(ReferenceGuard &&another) noexcept {
    another.m_check_ref_count();
    m_check_ref_count();
    return *this;
  }

  ReferenceGuard &operator=(ReferenceGuard const &another) {
    m_check_ref_count();
    return *this;
  }

  virtual ~ReferenceGuard() { m_check_ref_count(); }

private:
  void m_check_ref_count() {
    if (m_ref_count != 0) {
      auto error = ReferenceGuardError(m_ref_count.load());
      irrecoverableError(error);
    }
  }
  mutable std::atomic<int> m_ref_count = 0;
};
#else
class ReferenceGuard {};
#endif
/**
 *
 *
 * @class StrongReference
 * Provides a mechanism of storing long-life references
 * to object with reference checker enabled.
 *
 * @parameter T
 * specifies a class for which reference should be constructed.
 * T must be derived from ReferenceGuard class to be able
 * to have StrongReference<T> instances.
 *
 * @parameter TBase
 * Must be some base class for T, which is strictly derived from
 * single ReferenceGuard base class. It is used to invoke methods
 * of ReferenceGuard. It is defaulted to be T itself. However,
 * there are cases where T can have multiple ReferenceGuarded bases,
 * in such case invoking it's methods is impossible due to
 * ambiguity and TBase must be provided to resolve it.
 *
 * @parameter U
 * U must be derived from T and U& must be implicitly
 * convertible to T&.
 *
 */
#ifdef VKW_ENABLE_REFERENCE_GUARD
template <typename T, typename TBase = T>
requires std::derived_from<TBase, ReferenceGuard> and
    std::is_base_of_v<TBase, T>
class StrongReference : public std::reference_wrapper<T> {
public:
  template <class U>
  requires std::derived_from<U, T> StrongReference(U &object)
      : std::reference_wrapper<T>{object} {
    std::invoke(&TBase::add_reference, static_cast<TBase &>(object));
  }

  StrongReference(StrongReference &&another) noexcept
      : std::reference_wrapper<T>(another) {
    std::invoke(&TBase::add_reference,
                static_cast<TBase &>(std::reference_wrapper<T>::get()));
  }

  StrongReference(StrongReference const &another)
      : std::reference_wrapper<T>(another) {
    std::invoke(&TBase::add_reference,
                static_cast<TBase &>(std::reference_wrapper<T>::get()));
  }

  StrongReference &operator=(StrongReference &&another) noexcept {
    if (this == &another)
      return *this;
    std::swap(static_cast<std::reference_wrapper<T> &>(*this),
              static_cast<std::reference_wrapper<T> &>(another));
    return *this;
  }

  StrongReference &operator=(StrongReference const &another) {
    if (this == &another)
      return *this;
    StrongReference tmp{another};
    *this = std::move(tmp);
    return *this;
  }

  ~StrongReference() {
    std::invoke(&TBase::remove_reference,
                static_cast<TBase &>(std::reference_wrapper<T>::get()));
  }
};
#else

template <typename T, typename TBase = T>
requires std::derived_from<TBase, ReferenceGuard> and
    std::is_base_of_v<TBase, T>
class StrongReference : public std::reference_wrapper<T> {
public:
  template <class U>
  requires std::derived_from<U, T> StrongReference(U &object)
      : std::reference_wrapper<T>{object} {}
};

#endif
} // namespace vkw
#endif // VKWRAPPER_REFERENCEGUARD_HPP
