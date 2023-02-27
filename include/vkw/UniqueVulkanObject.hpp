#ifndef VKWRAPPER_UNIQUEVULKANOBJECT_HPP
#define VKWRAPPER_UNIQUEVULKANOBJECT_HPP

#include <vkw/ReferenceGuard.hpp>
#include <vkw/VulkanTypeTraits.hpp>

namespace vkw {

template <typename T, typename DeleterT>
class UniqueVulkanObjectCommon
    : private std::unique_ptr<std::remove_pointer_t<T>, DeleterT>,
      public ReferenceGuard {
public:
  UniqueVulkanObjectCommon(T handle, DeleterT const &d)
      : std::unique_ptr<std::remove_pointer_t<T>, DeleterT>(handle, d){};

  operator T() const {
    return std::unique_ptr<std::remove_pointer_t<T>, DeleterT>::get();
  }

protected:
  auto handle() const {
    return std::unique_ptr<std::remove_pointer_t<T>, DeleterT>::get();
  }
};

template <typename T> class UniqueVulkanObjectCommonDeleter {
public:
  using TypeTraits = VulkanTypeTraits<T>;

  UniqueVulkanObjectCommonDeleter(
      typename TypeTraits::CreatorType const &creator)
      : m_creator(creator){};

  void operator()(T handle) {
    std::invoke(TypeTraits::getDestructor(m_creator.get()), m_creator.get(),
                handle, nullptr);
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
    : public UniqueVulkanObjectCommon<T, UniqueVulkanObjectCommonDeleter<T>> {
public:
  using TypeTraits = VulkanTypeTraits<T>;

private:
  static T m_createImpl(typename TypeTraits::CreatorType const &creator,
                        typename TypeTraits::CreateInfoType const &createInfo) {
    // TODO: add checks here
    T ret;
    std::invoke(TypeTraits::getConstructor(creator), creator, &createInfo,
                nullptr, &ret);
    return ret;
  }

public:
  UniqueVulkanObject(typename TypeTraits::CreatorType const &creator,
                     typename TypeTraits::CreateInfoType const &createInfo)
      : UniqueVulkanObjectCommon<T, UniqueVulkanObjectCommonDeleter<T>>(
            m_createImpl(creator, createInfo),
            UniqueVulkanObjectCommonDeleter<T>(creator)),
        m_creator(creator) {}

  auto &parent() const { return m_creator.get(); }

private:
  StrongReference<typename TypeTraits::CreatorType const> m_creator;
};

/**
 *
 * Specialization for @struct VkDevice
 *
 */

class DeviceDeleter {
public:
  DeviceDeleter(vkw::Instance const &instance) : m_instance(instance) {}

  void operator()(VkDevice handle) {
    std::invoke(VulkanTypeTraits<VkDevice>::getDestructor(m_instance), handle,
                nullptr);
  }

private:
  std::reference_wrapper<vkw::Instance const> m_instance;
};

template <>
class UniqueVulkanObject<VkDevice>
    : public UniqueVulkanObjectCommon<VkDevice, DeviceDeleter> {
public:
  using TypeTraits = VulkanTypeTraits<VkDevice>;

private:
  static VkDevice m_createImpl(vkw::Instance const &instance,
                               VkPhysicalDevice phDevice,
                               VkDeviceCreateInfo const &createInfo) {
    // TODO: add checks here
    VkDevice ret;
    std::invoke(TypeTraits::getConstructor(instance), phDevice, &createInfo,
                nullptr, &ret);
    return ret;
  }

public:
  UniqueVulkanObject(vkw::Instance const &instance, VkPhysicalDevice phDevice,
                     VkDeviceCreateInfo const &createInfo)
      : UniqueVulkanObjectCommon<VkDevice, DeviceDeleter>(
            m_createImpl(instance, phDevice, createInfo),
            DeviceDeleter(instance)),
        m_instance(instance) {}

  auto &parent() const { return m_instance.get(); }

private:
  StrongReference<vkw::Instance const> m_instance;
};

} // namespace vkw
#endif // VKWRAPPER_UNIQUEVULKANOBJECT_HPP
