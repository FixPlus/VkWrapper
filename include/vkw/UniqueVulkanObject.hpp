#ifndef VKWRAPPER_UNIQUEVULKANOBJECT_HPP
#define VKWRAPPER_UNIQUEVULKANOBJECT_HPP

#include <vkw/ReferenceGuard.hpp>
#include <vkw/VulkanTypeTraits.hpp>

#include <memory>

namespace vkw {

template <typename T, typename Parent, typename DeleterT>
class UniqueVulkanObjectCommon
    : private std::unique_ptr<std::remove_pointer_t<T>, DeleterT>,
      public ReferenceGuard {
public:
  UniqueVulkanObjectCommon(T handle, Parent const &parent,
                           DeleterT const &d) noexcept(ExceptionsDisabled)
      : std::unique_ptr<std::remove_pointer_t<T>, DeleterT>(handle, d),
        m_parent(parent){};

  operator T() const noexcept {
    return std::unique_ptr<std::remove_pointer_t<T>, DeleterT>::get();
  }

  auto &parent() const noexcept { return m_parent.get(); }

  auto &hostAllocator() const noexcept { return parent().hostAllocator(); }

protected:
  auto handle() const noexcept {
    return std::unique_ptr<std::remove_pointer_t<T>, DeleterT>::get();
  }

private:
  StrongReference<Parent const> m_parent;
};

template <typename T> class UniqueVulkanObjectCommonDeleter {
public:
  using TypeTraits = VulkanTypeTraits<T>;

  UniqueVulkanObjectCommonDeleter(typename TypeTraits::CreatorType const
                                      &creator) noexcept(ExceptionsDisabled)
      : m_creator(creator){};

  void operator()(T handle) noexcept {
    std::invoke(TypeTraits::getDestructor(m_creator.get()), m_creator.get(),
                handle, m_creator.get().hostAllocator().allocator());
  }

private:
  StrongReference<typename TypeTraits::CreatorType const> m_creator;
};
/**
 * @class UniqueVulkanObject
 *
 * @brief Base class template for all vulkan created objects. Some objects
 * have their VulkanTypeTraits<> class specialization auto-generated and
 * that's why can use main template. But there are exceptions like VkDevice
 * when it is needed to explicitly specialize this base template.
 *
 * @tparam T is raw vulkan handle(VkInstance, VkDevice)
 *
 */
template <typename T>
class UniqueVulkanObject
    : public UniqueVulkanObjectCommon<T,
                                      typename VulkanTypeTraits<T>::CreatorType,
                                      UniqueVulkanObjectCommonDeleter<T>> {
public:
  using TypeTraits = VulkanTypeTraits<T>;

private:
  static T m_createImpl(typename TypeTraits::CreatorType const &creator,
                        typename TypeTraits::CreateInfoType const
                            &createInfo) noexcept(ExceptionsDisabled) {
    // TODO: add checks here
    T ret;
    std::invoke(TypeTraits::getConstructor(creator), creator, &createInfo,
                creator.hostAllocator().allocator(), &ret);
    return ret;
  }

public:
  UniqueVulkanObject(typename TypeTraits::CreatorType const &creator,
                     typename TypeTraits::CreateInfoType const
                         &createInfo) noexcept(ExceptionsDisabled)
      : UniqueVulkanObjectCommon<T, typename TypeTraits::CreatorType,
                                 UniqueVulkanObjectCommonDeleter<T>>(
            m_createImpl(creator, createInfo), creator,
            UniqueVulkanObjectCommonDeleter<T>(creator)) {}
};

/**
 *
 * Specialization for @struct VkDevice
 *
 */

class DeviceDeleter {
public:
  DeviceDeleter(vkw::Instance const &instance) noexcept
      : m_instance(instance) {}

  void operator()(VkDevice handle) noexcept {
    std::invoke(VulkanTypeTraits<VkDevice>::getDestructor(m_instance), handle,
                m_instance.get().hostAllocator().allocator());
  }

private:
  std::reference_wrapper<vkw::Instance const> m_instance;
};

template <>
class UniqueVulkanObject<VkDevice>
    : public UniqueVulkanObjectCommon<VkDevice, vkw::Instance, DeviceDeleter> {
public:
  using TypeTraits = VulkanTypeTraits<VkDevice>;

private:
  static VkDevice m_createImpl(
      vkw::Instance const &instance, VkPhysicalDevice phDevice,
      VkDeviceCreateInfo const &createInfo) noexcept(ExceptionsDisabled) {
    // TODO: add checks here
    VkDevice ret;
    std::invoke(TypeTraits::getConstructor(instance), phDevice, &createInfo,
                instance.hostAllocator().allocator(), &ret);
    return ret;
  }

public:
  UniqueVulkanObject(
      vkw::Instance const &instance, VkPhysicalDevice phDevice,
      VkDeviceCreateInfo const &createInfo) noexcept(ExceptionsDisabled)
      : UniqueVulkanObjectCommon<VkDevice, vkw::Instance, DeviceDeleter>(
            m_createImpl(instance, phDevice, createInfo), instance,
            DeviceDeleter(instance)) {}
};

} // namespace vkw
#endif // VKWRAPPER_UNIQUEVULKANOBJECT_HPP
