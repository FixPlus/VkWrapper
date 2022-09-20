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

#define VKW_MAP_ENTRY(X, Y) m_extensions_name_map[X] = Y;
#define VKW_DUMP_EXTENSION_NAME_MAP_DEFINITION
#include "SymbolTable.inc"
#undef VKW_DUMP_EXTENSION_NAME_MAP_DEFINITION
#undef VKW_MAP_ENTRY
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
VkLayerProperties Library::layerProperties(std::string_view layerName) const {
  auto found =
      std::find_if(m_layer_properties.begin(), m_layer_properties.end(),
                   [&layerName](VkLayerProperties const &layer) {
                     return !layerName.compare(layer.layerName);
                   });

  if (found == m_layer_properties.end())
    throw vkw::Error(std::string(layerName) + " is missing");

  return *found;
}

bool Library::hasLayer(std::string_view layerName) const {
  return std::any_of(m_layer_properties.begin(), m_layer_properties.end(),
                     [&layerName](VkLayerProperties const &layer) {
                       return !layerName.compare(layer.layerName);
                     });
}
ext Library::getExtensionId(std::string_view extensionName) const {
  ext result;
  auto found = std::find_if(
      m_extensions_name_map.begin(), m_extensions_name_map.end(),
      [extensionName](
          std::unordered_map<ext, const char *>::value_type const &entry) {
        return !extensionName.compare(entry.second);
      });
  if (found == m_extensions_name_map.end()) {
    std::stringstream ss;
    ss << "Bad extension name: " << extensionName;
    throw Error(ss.str());
  }
  return found->first;
}
bool Library::hasInstanceExtension(ext extensionId) const {

  std::string_view name = extensionName(extensionId);

  return std::any_of(m_instance_extension_properties.begin(),
                     m_instance_extension_properties.end(),
                     [&name](VkExtensionProperties const &layer) {
                       return !name.compare(layer.extensionName);
                     });
}
VkExtensionProperties
Library::instanceExtensionProperties(ext extensionId) const {
  std::string_view name = extensionName(extensionId);

  auto found = std::find_if(m_instance_extension_properties.begin(),
                            m_instance_extension_properties.end(),
                            [&name](VkExtensionProperties const &layer) {
                              return !name.compare(layer.extensionName);
                            });

  if (found == m_instance_extension_properties.end())
    throw vkw::ExtensionMissing(extensionId, name);

  return *found;
}

const char *Library::extensionName(ext id) const {
  if (!m_extensions_name_map.contains(id)) {
    std::stringstream ss;
    ss << "Unhandled extension ID: vkw::ext::" << static_cast<unsigned>(id);
    throw vkw::Error(ss.str());
  }
  return m_extensions_name_map.at(id);
}

} // namespace vkw
