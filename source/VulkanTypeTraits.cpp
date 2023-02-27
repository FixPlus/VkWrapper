#include "vkw/VulkanTypeTraits.hpp"
#include <vkw/Device.hpp>
#include <vkw/Instance.hpp>

namespace vkw {

#define VKW_GENERATE_TYPE_FUNC_IMPL
#include "VulkanTypeTraits.inc"
#undef VKW_GENERATE_TYPE_FUNC_IMPL

PFN_vkCreateDevice
VulkanTypeTraits<VkDevice>::getConstructor(vkw::Instance const &creator) {
  return creator.core<1, 0>().vkCreateDevice;
}
PFN_vkDestroyDevice
VulkanTypeTraits<VkDevice>::getDestructor(vkw::Instance const &creator) {
  return reinterpret_cast<PFN_vkDestroyDevice>(
      creator.parent().vkGetInstanceProcAddr(creator, "vkDestroyDevice"));
}
} // namespace vkw