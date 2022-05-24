#include "vkw/Library.hpp"
#include "loader/DynamicLoader.hpp"
#include "vkw/Exception.hpp"
#include <cassert>

namespace vkw {

std::unordered_map<std::string, InstanceExtensionInitializerBase const *>
    m_instanceExtInitializers{};
std::unordered_map<std::string, DeviceExtensionInitializerBase const *>
    m_deviceExtInitializers{};

Library::Library(PFN_m_fill_extension_map p) {
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
  p(); // fill extension table

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
VkLayerProperties
Library::layerProperties(const std::string_view &layerName) const {
  auto found =
      std::find_if(m_layer_properties.begin(), m_layer_properties.end(),
                   [&layerName](VkLayerProperties const &layer) {
                     return !strcmp(layer.layerName, layerName.data());
                   });

  if (found == m_layer_properties.end())
    throw vkw::Error(std::string(layerName) + " is missing");

  return *found;
}

bool Library::hasLayer(const std::string_view &layerName) const {
  return std::any_of(m_layer_properties.begin(), m_layer_properties.end(),
                     [&layerName](VkLayerProperties const &layer) {
                       return !strcmp(layer.layerName, layerName.data());
                     });
}
bool Library::hasInstanceExtension(
    const std::string_view &extensionName) const {
  return std::any_of(m_instance_extension_properties.begin(),
                     m_instance_extension_properties.end(),
                     [&extensionName](VkExtensionProperties const &layer) {
                       return !strcmp(layer.extensionName,
                                      extensionName.data());
                     });
}
VkExtensionProperties Library::instanceExtensionProperties(
    std::string_view const &extensionName) const {
  auto found =
      std::find_if(m_instance_extension_properties.begin(),
                   m_instance_extension_properties.end(),
                   [&extensionName](VkExtensionProperties const &layer) {
                     return !strcmp(layer.extensionName, extensionName.data());
                   });

  if (found == m_instance_extension_properties.end())
    throw vkw::ExtensionMissing(std::string(extensionName));

  return *found;
}

} // namespace vkw
