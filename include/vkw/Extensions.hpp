#ifndef VKWRAPPER_EXTENSIONS_HPP
#define VKWRAPPER_EXTENSIONS_HPP

#include "Exception.hpp"
#include "SymbolTable.hpp"

namespace vkw {

class Instance;
class Device;
template <typename T> struct NativeHandlerOf {};

template <> struct NativeHandlerOf<Instance> { using Handle = VkInstance; };

template <> struct NativeHandlerOf<Device> { using Handle = VkDevice; };

enum class ext;

template <ext name> struct ExtensionName {};

namespace internal {
bool isExtensionEnabled(Instance const &instance, const char *extName);
bool isExtensionEnabled(Device const &device, const char *extName);

template <typename T>
using PFN_getProcAddr =
    PFN_vkVoidFunction (*)(typename NativeHandlerOf<T>::Handle, const char *);

template <typename T> PFN_getProcAddr<T> getProcAddrOf(T const &instance);

extern template PFN_getProcAddr<Device> getProcAddrOf(Device const &instance);

extern template PFN_getProcAddr<Instance>
getProcAddrOf(Instance const &instance);

VkInstance handleOf(Instance const &instance);
VkDevice handleOf(Device const &device);
} // namespace internal

template <ext id, typename T>
class ExtensionBase
    : public SymbolTableBase<typename NativeHandlerOf<T>::Handle> {
public:
  ExtensionBase(T const &handle)
      : SymbolTableBase<typename NativeHandlerOf<T>::Handle>(
            internal::getProcAddrOf(handle), internal::handleOf(handle)) {
    if (!internal::isExtensionEnabled(handle, ExtensionName<id>::value))
      throw ExtensionMissing(id, ExtensionName<id>::value);
  }

  constexpr static const char *name = ExtensionName<id>::value;
};

template <ext name> class Extension {};

#define VKW_DUMP_EXTENSION_CLASSES
#include "SymbolTable.inc"
#undef VKW_DUMP_EXTENSION_CLASSES

} // namespace vkw
#endif // VKWRAPPER_EXTENSIONS_HPP
