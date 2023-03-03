#ifndef VKWRAPPER_EXTENSIONS_HPP
#define VKWRAPPER_EXTENSIONS_HPP

#include <vkw/Exception.hpp>
#include <vkw/SymbolTable.hpp>

namespace vkw {

class Instance;
class Device;
template <typename T> struct NativeHandlerOf {};

template <> struct NativeHandlerOf<Instance> { using Handle = VkInstance; };

template <> struct NativeHandlerOf<Device> { using Handle = VkDevice; };

enum class ext;

namespace internal {
bool isExtensionEnabled(Instance const &instance,
                        const char *extName) noexcept(ExceptionsDisabled);
bool isExtensionEnabled(Device const &device,
                        const char *extName) noexcept(ExceptionsDisabled);

const char *ext_name(ext id) noexcept(ExceptionsDisabled);

template <typename T>
using PFN_getProcAddr =
    PFN_vkVoidFunction (*)(typename NativeHandlerOf<T>::Handle, const char *);

template <typename T>
PFN_getProcAddr<T> getProcAddrOf(T const &instance) noexcept;

extern template PFN_getProcAddr<Device>
getProcAddrOf(Device const &instance) noexcept;

extern template PFN_getProcAddr<Instance>
getProcAddrOf(Instance const &instance) noexcept;

VkInstance handleOf(Instance const &instance) noexcept;
VkDevice handleOf(Device const &device) noexcept;
} // namespace internal

template <ext id, typename T>
class ExtensionBase
    : public SymbolTableBase<typename NativeHandlerOf<T>::Handle> {
public:
  explicit ExtensionBase(T const &handle) noexcept(ExceptionsDisabled)
      : SymbolTableBase<typename NativeHandlerOf<T>::Handle>(
            internal::getProcAddrOf(handle), internal::handleOf(handle)) {
    if (!internal::isExtensionEnabled(handle, internal::ext_name(id)))
      postError(ExtensionMissing(id, internal::ext_name(id)));
  }
};

template <ext name> class Extension {};

enum class ext {
#define VKW_DUMP_EXTENSION_MAP
#define VKW_EXTENSION_ENTRY(X) X,
#include "SymbolTable.inc"
#undef VKW_EXTENSION_ENTRY
#undef VKW_DUMP_EXTENSION_MAP
};
#define VKW_DUMP_EXTENSION_CLASSES
#include "SymbolTable.inc"
#undef VKW_DUMP_EXTENSION_CLASSES

} // namespace vkw
#endif // VKWRAPPER_EXTENSIONS_HPP
