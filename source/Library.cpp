#include "vkw/Library.hpp"
#include "loader/DynamicLoader.hpp"
#include "vkw/Exception.hpp"
#include "vkw/Extensions.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <sstream>

namespace vkw {

class VulkanDefaultLoader : public VulkanLibraryLoader, private DynamicLoader {
public:
  VulkanDefaultLoader() : DynamicLoader(m_libname){};

  PFN_vkGetInstanceProcAddr getInstanceProcAddr() override {
    return reinterpret_cast<PFN_vkGetInstanceProcAddr>(
        getSymbol("vkGetInstanceProcAddr"));
  };

private:
  static constexpr const char *m_libname =
#ifdef _WIN32
      "vulkan-1.dll";
#elif defined __linux__
      "libvulkan.so.1";
#else
      "";
#endif
};

Library::Library(VulkanLibraryLoader *loader, HostAllocator *allocator,
                 ApiVersion vkwVersion) noexcept(ExceptionsDisabled)
    : m_default_allocator(allocator ? nullptr : new HostAllocator(false)),
      m_allocator(allocator ? *allocator : *m_default_allocator) {
  auto dll_vkw_version = ApiVersion{MajorVersion, MinorVersion, 0};
  if (vkwVersion != dll_vkw_version) {
    postError(ApiVersionUnsupported{
        "vkw version mismatch", dll_vkw_version, vkwVersion,
        ApiVersionUnsupported::CompatibilityFactor::EXACT_VERSION});
  }
  if (loader == nullptr) {
    m_embedded_loader = std::make_unique<VulkanDefaultLoader>();
    loader = m_embedded_loader.get();
  }

  vkGetInstanceProcAddr = loader->getInstanceProcAddr();
  vkCreateInstance = reinterpret_cast<decltype(vkCreateInstance)>(
      vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
  vkEnumerateInstanceExtensionProperties =
      reinterpret_cast<decltype(vkEnumerateInstanceExtensionProperties)>(
          vkGetInstanceProcAddr(nullptr,
                                "vkEnumerateInstanceExtensionProperties"));
  vkEnumerateInstanceLayerProperties =
      reinterpret_cast<decltype(vkEnumerateInstanceLayerProperties)>(
          vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties"));
  vkEnumerateInstanceVersion =
      reinterpret_cast<decltype(vkEnumerateInstanceVersion)>(
          vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

  uint32_t layerCount;

  // enumerate all instance layers

  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  m_layer_properties.resize(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, m_layer_properties.data());

  // enumerate all instance-level extensions

  uint32_t extensionCount;

  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  m_instance_extension_properties.resize(extensionCount);
  vkEnumerateInstanceExtensionProperties(
      nullptr, &extensionCount, m_instance_extension_properties.data());

  uint32_t extensionAccumulated = extensionCount;

  for (auto &layer : m_layer_properties) {
    vkEnumerateInstanceExtensionProperties(layer.layerName, &extensionCount,
                                           nullptr);
    m_instance_extension_properties.resize(extensionAccumulated +
                                           extensionCount);
    vkEnumerateInstanceExtensionProperties(
        layer.layerName, &extensionCount,
        m_instance_extension_properties.data() + extensionAccumulated);
    extensionAccumulated += extensionCount;
  }
}

ApiVersion::ApiVersion(uint32_t encoded)
    : major(VK_API_VERSION_MAJOR(encoded)),
      minor(VK_API_VERSION_MINOR(encoded)),
      revision(VK_API_VERSION_PATCH(encoded)) {}
ApiVersion Library::instanceAPIVersion() const noexcept {
  if (this->vkEnumerateInstanceVersion == nullptr)
    return {1, 0, 0};

  uint32_t encoded;
  this->vkEnumerateInstanceVersion(&encoded);
  return ApiVersion{encoded};
}
Library::~Library() {}
VkLayerProperties Library::layerProperties(layer layerId) const
    noexcept(ExceptionsDisabled) {
  std::string_view layerName = LayerName(layerId);
  auto found =
      std::find_if(m_layer_properties.begin(), m_layer_properties.end(),
                   [&layerName](VkLayerProperties const &layer) {
                     return !layerName.compare(layer.layerName);
                   });

  if (found == m_layer_properties.end())
    postError(vkw::Error(std::string(layerName) + " is missing"));

  return *found;
}

bool Library::hasLayer(layer layerId) const noexcept(ExceptionsDisabled) {
  std::string_view layerName = LayerName(layerId);
  return std::any_of(m_layer_properties.begin(), m_layer_properties.end(),
                     [&layerName](VkLayerProperties const &layer) {
                       return !layerName.compare(layer.layerName);
                     });
}
bool Library::hasInstanceExtension(ext extensionId) const
    noexcept(ExceptionsDisabled) {

  std::string_view name = ExtensionName(extensionId);

  return std::any_of(m_instance_extension_properties.begin(),
                     m_instance_extension_properties.end(),
                     [&name](VkExtensionProperties const &layer) {
                       return !name.compare(layer.extensionName);
                     });
}
VkExtensionProperties
Library::instanceExtensionProperties(ext extensionId) const
    noexcept(ExceptionsDisabled) {
  std::string_view name = ExtensionName(extensionId);

  auto found = std::find_if(m_instance_extension_properties.begin(),
                            m_instance_extension_properties.end(),
                            [&name](VkExtensionProperties const &layer) {
                              return !name.compare(layer.extensionName);
                            });

  if (found == m_instance_extension_properties.end())
    postError(vkw::ExtensionMissing(extensionId, name));

  return *found;
}

namespace {
constexpr const char *ExtensionNames[] = {
#define STRINGIFY(X) #X
#define VKW_DUMP_EXTENSION_MAP
#define VKW_EXTENSION_ENTRY(X) STRINGIFY(VK_##X),
#include "SymbolTable.inc"
#undef VKW_EXTENSION_ENTRY
#undef VKW_DUMP_EXTENSION_MAP
};

constexpr const char *LayerNames[] = {
#define VKW_LAYER_MAP_ENTRY(X) STRINGIFY(VK_LAYER_##X),
#include "LayerMap.inc"
#undef VKW_EXTENSION_ENTRY
};

} // namespace

const char *Library::ExtensionName(ext id) noexcept(ExceptionsDisabled) {
  auto index = static_cast<unsigned>(id);
  if (index >= sizeof(ExtensionNames) / sizeof(const char *)) {
    std::stringstream ss;
    ss << "Unhandled extension ID: vkw::ext::" << index;
    postError(vkw::Error(ss.str()));
  }
  return ExtensionNames[static_cast<unsigned>(id)];
}

bool Library::ValidExtensionName(std::string_view extensionName) noexcept {
  auto *begin = ExtensionNames;
  auto *end = ExtensionNames + (sizeof(ExtensionNames) / sizeof(const char *));
  auto *found = std::find_if(begin, end, [extensionName](const char *entry) {
    return !extensionName.compare(entry);
  });

  return found != end;
}

ext Library::ExtensionId(std::string_view extensionName) noexcept(
    ExceptionsDisabled) {
  auto *begin = ExtensionNames;
  auto *end = ExtensionNames + (sizeof(ExtensionNames) / sizeof(const char *));
  auto *found = std::find_if(begin, end, [extensionName](const char *entry) {
    return !extensionName.compare(entry);
  });
  if (found == end) {
    std::stringstream ss;
    ss << "Bad extension name: " << extensionName;
    postError(Error(ss.str()));
  }

  return static_cast<ext>(found - begin);
}

bool Library::ValidLayerName(std::string_view layerName) noexcept {
  auto *begin = LayerNames;
  auto *end = LayerNames + (sizeof(LayerNames) / sizeof(const char *));
  auto *found = std::find_if(begin, end, [layerName](const char *entry) {
    return !layerName.compare(entry);
  });

  return found != end;
}

const char *Library::LayerName(layer id) noexcept(ExceptionsDisabled) {
  auto index = static_cast<unsigned>(id);
  if (index >= sizeof(LayerNames) / sizeof(const char *)) {
    std::stringstream ss;
    ss << "Unhandled layer ID: vkw::ext::" << index;
    postError(vkw::Error(ss.str()));
  }
  return LayerNames[static_cast<unsigned>(id)];
}
layer Library::LayerId(std::string_view layerName) noexcept(
    ExceptionsDisabled) {
  auto *begin = LayerNames;
  auto *end = LayerNames + (sizeof(LayerNames) / sizeof(const char *));
  auto *found = std::find_if(begin, end, [layerName](const char *entry) {
    return !layerName.compare(entry);
  });
  if (found == end) {
    std::stringstream ss;
    ss << "Bad extension name: " << layerName;
    postError(Error(ss.str()));
  }

  return static_cast<layer>(found - begin);
}

ApiVersion Library::version() noexcept {
  return ApiVersion(MajorVersion, MinorVersion, 0);
}

ApiVersionUnsupported::ApiVersionUnsupported(
    std::string_view details, ApiVersion lastSupported, ApiVersion unsupported,
    CompatibilityFactor compatibility) noexcept
    : Error(std::string(details)
                .append(": ")
                .append(unsupported)
                .append(" is unsupported (")
                .append(compatibility == CompatibilityFactor::EARLIER_VERSION
                            ? "last supported:"
                            : "only supported:")
                .append(lastSupported)
                .append(")"),
            ErrorCode::API_VERSION_UNSUPPORTED),
      m_unsupported(unsupported), m_lastSupported(lastSupported),
      m_compatibility(compatibility) {}

namespace internal {
const char *ext_name(ext id) noexcept(ExceptionsDisabled) {
  return Library::ExtensionName(id);
}
const char *layer_name(layer id) noexcept(ExceptionsDisabled) {
  return Library::LayerName(id);
}
} // namespace internal

} // namespace vkw
