
#include "vkw/Device.hpp"
#include "vkw/Instance.hpp"

namespace vkw {

template <typename T> struct NativeHandlerOf {};

template <> struct NativeHandlerOf<Instance> { using Handle = VkInstance; };

template <> struct NativeHandlerOf<Device> { using Handle = VkDevice; };

template <typename T>
using PFN_getProcAddr =
    PFN_vkVoidFunction (*)(typename NativeHandlerOf<T>::Handle, const char *);

} // namespace vkw

namespace vkw::internal {

bool isExtensionEnabled(Instance const &instance, const char *extName) {
  return instance.isExtensionEnabled(instance.parent().ExtensionId(extName));
}
bool isExtensionEnabled(Device const &device, const char *extName) {
  auto extId = device.getParent().parent().ExtensionId(extName);
  auto &enabledExtensions = device.physicalDevice().enabledExtensions();
  return std::any_of(enabledExtensions.begin(), enabledExtensions.end(),
                     [extId](ext entry) { return entry == extId; });
}

bool isLayerEnabled(Instance const &instance, layer layer) {
  return instance.isLayerEnabled(layer);
}

template <typename T> PFN_getProcAddr<T> getProcAddrOf(T const &instance);

template <> PFN_getProcAddr<Device> getProcAddrOf(Device const &device) {
  return device.getParent().core<1, 0>().vkGetDeviceProcAddr;
}

template <> PFN_getProcAddr<Instance> getProcAddrOf(Instance const &instance) {
  return instance.parent().vkGetInstanceProcAddr;
}

VkInstance handleOf(Instance const &instance) { return instance; }
VkDevice handleOf(Device const &device) { return device; }

} // namespace vkw::internal