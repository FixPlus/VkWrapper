#include "vkw/Library.hpp"
#include "loader/DynamicLoader.hpp"
#include "vkw/Exception.hpp"
#include "vkw/Extensions.hpp"
#include <cassert>
#include <cstring>
#include <sstream>
namespace vkw {

Library::Library() {
  static constexpr const char *libName =
#ifdef _WIN32
      "vulkan-1.dll";
#elif defined __linux__
      "libvulkan.so.1";
#endif
  m_loader = std::make_unique<DynamicLoader>(libName);

  vkCreateInstance = reinterpret_cast<PFN_vkCreateInstance>(
      m_loader->getSymbol("vkCreateInstance"));
  vkEnumerateInstanceExtensionProperties =
      reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
          m_loader->getSymbol("vkEnumerateInstanceExtensionProperties"));
  vkEnumerateInstanceLayerProperties =
      reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
          m_loader->getSymbol("vkEnumerateInstanceLayerProperties"));
  vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(
      m_loader->getSymbol("vkGetInstanceProcAddr"));
  try {
    vkEnumerateInstanceVersion =
        reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
            m_loader->getSymbol("vkEnumerateInstanceVersion"));
  } catch (Error &e) {
    if (e.code() == ErrorCode::DYNAMIC_LIBRARY_SYMBOL_MISSING) {
      // it's okay, because Vulkan 1.0 doesn't have this function yet
    } else
      throw;
  }

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
ApiVersion Library::instanceAPIVersion() const {
  if (this->vkEnumerateInstanceVersion == nullptr)
    return {1, 0, 0};

  uint32_t encoded;
  this->vkEnumerateInstanceVersion(&encoded);
  return ApiVersion{encoded};
}
Library::~Library() {}
VkLayerProperties Library::layerProperties(layer layerId) const {
  std::string_view layerName = LayerName(layerId);
  auto found =
      std::find_if(m_layer_properties.begin(), m_layer_properties.end(),
                   [&layerName](VkLayerProperties const &layer) {
                     return !layerName.compare(layer.layerName);
                   });

  if (found == m_layer_properties.end())
    throw vkw::Error(std::string(layerName) + " is missing");

  return *found;
}

bool Library::hasLayer(layer layerId) const {
  std::string_view layerName = LayerName(layerId);
  return std::any_of(m_layer_properties.begin(), m_layer_properties.end(),
                     [&layerName](VkLayerProperties const &layer) {
                       return !layerName.compare(layer.layerName);
                     });
}
bool Library::hasInstanceExtension(ext extensionId) const {

  std::string_view name = ExtensionName(extensionId);

  return std::any_of(m_instance_extension_properties.begin(),
                     m_instance_extension_properties.end(),
                     [&name](VkExtensionProperties const &layer) {
                       return !name.compare(layer.extensionName);
                     });
}
VkExtensionProperties
Library::instanceExtensionProperties(ext extensionId) const {
  std::string_view name = ExtensionName(extensionId);

  auto found = std::find_if(m_instance_extension_properties.begin(),
                            m_instance_extension_properties.end(),
                            [&name](VkExtensionProperties const &layer) {
                              return !name.compare(layer.extensionName);
                            });

  if (found == m_instance_extension_properties.end())
    throw vkw::ExtensionMissing(extensionId, name);

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

const char *Library::ExtensionName(ext id) {
  auto index = static_cast<unsigned>(id);
  if (index >= sizeof(ExtensionNames) / sizeof(const char *)) {
    std::stringstream ss;
    ss << "Unhandled extension ID: vkw::ext::" << index;
    throw vkw::Error(ss.str());
  }
  return ExtensionNames[static_cast<unsigned>(id)];
}

ext Library::ExtensionId(std::string_view extensionName) {
  auto *begin = ExtensionNames;
  auto *end = ExtensionNames + (sizeof(ExtensionNames) / sizeof(const char *));
  auto *found = std::find_if(begin, end, [extensionName](const char *entry) {
    return !extensionName.compare(entry);
  });
  if (found == end) {
    std::stringstream ss;
    ss << "Bad extension name: " << extensionName;
    throw Error(ss.str());
  }

  return static_cast<ext>(found - begin);
}
const char *Library::LayerName(layer id) {
  auto index = static_cast<unsigned>(id);
  if (index >= sizeof(LayerNames) / sizeof(const char *)) {
    std::stringstream ss;
    ss << "Unhandled layer ID: vkw::ext::" << index;
    throw vkw::Error(ss.str());
  }
  return LayerNames[static_cast<unsigned>(id)];
}
layer Library::LayerId(std::string_view layerName) {
  auto *begin = LayerNames;
  auto *end = LayerNames + (sizeof(LayerNames) / sizeof(const char *));
  auto *found = std::find_if(begin, end, [layerName](const char *entry) {
    return !layerName.compare(entry);
  });
  if (found == end) {
    std::stringstream ss;
    ss << "Bad extension name: " << layerName;
    throw Error(ss.str());
  }

  return static_cast<layer>(found - begin);
}

namespace internal {
const char *ext_name(ext id) { return Library::ExtensionName(id); }
const char *layer_name(layer id) { return Library::LayerName(id); }
} // namespace internal

} // namespace vkw
