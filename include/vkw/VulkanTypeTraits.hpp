#ifndef VKWRAPPER_VULKANTYPETRAITS_HPP
#define VKWRAPPER_VULKANTYPETRAITS_HPP

#include <vulkan/vulkan.h>

namespace vkw {
class Device;
class Instance;

template <typename T> struct VulkanTypeTraits {};

#define VKW_GENERATE_TYPE_DEFINITIONS
#include "VulkanTypeTraits.inc"
#undef VKW_GENERATE_TYPE_DEFINITIONS

template <> struct VulkanTypeTraits<VkDevice> {
  using CreatorType = vkw::Device;
  using CreateInfoType = VkDeviceCreateInfo;
  static PFN_vkCreateDevice getConstructor(vkw::Instance const &creator);
  static PFN_vkDestroyDevice getDestructor(vkw::Instance const &creator);
};
} // namespace vkw
#endif // VKWRAPPER_VULKANTYPETRAITS_HPP
