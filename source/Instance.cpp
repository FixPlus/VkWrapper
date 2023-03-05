#include "vkw/Instance.hpp"
#include "Utils.hpp"
#include "loader/DynamicLoader.hpp"
#include "vkw/Device.hpp"
#include "vkw/Exception.hpp"
#include "vkw/Extensions.hpp"
#include "vkw/Validation.hpp"
#include <algorithm>
#include <cassert>
#include <cstring>
#include <vkw/PhysicalDevice.hpp>

#undef max
#include <limits>
namespace vkw {
namespace {
template <unsigned major = 1, unsigned minor = 0>
std::unique_ptr<InstanceCore<1, 0>>
loadInstanceSymbols(vkw::Library const &library, VkInstance instance,
                    ApiVersion version) noexcept(ExceptionsDisabled) {
  // Here template magic is being used to automatically generate load of
  // every available InstanceCore<major, minor> classes from SymbolTable.inc
  if (version >
      ApiVersion{major, minor, std::numeric_limits<unsigned>::max()}) {
    if constexpr (std::derived_from<InstanceCore<major, minor + 1>,
                                    SymbolTableBase<VkInstance>>)
      return loadInstanceSymbols<major, minor + 1>(library, instance, version);
    else if constexpr (std::derived_from<InstanceCore<major + 1, 0>,
                                         SymbolTableBase<VkInstance>>)
      return loadInstanceSymbols<major + 1, 0>(library, instance, version);
    else
      postError(ApiVersionUnsupported(
          "Could not load instance symbols for requested api version",
          ApiVersion{major, minor, 0}, version));
  } else {
    return std::make_unique<InstanceCore<major, minor>>(
        library.vkGetInstanceProcAddr, instance);
  }
}

} // namespace
Instance::Instance(Library const &library,
                   InstanceCreateInfo const &CI) noexcept(ExceptionsDisabled)
    : m_vulkanLib(library), m_apiVer(CI.apiVersion) {

  if (library.instanceAPIVersion() < CI.apiVersion)
    postError(ApiVersionUnsupported(
        "Cannot create instance with requested api version",
        library.instanceAPIVersion(), CI.apiVersion));

  VkApplicationInfo appInfo{};

  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = CI.applicationName.data();
  appInfo.applicationVersion = CI.applicationVersion;
  appInfo.pEngineName = CI.engineName.data();
  appInfo.engineVersion = CI.engineVersion;
  appInfo.apiVersion = CI.apiVersion;

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  // Check presence of all required layers and extensions

  std::for_each(CI.requestedLayers.begin(), CI.requestedLayers.end(),
                [&library](layer id) {
                  if (!library.hasLayer(id))
                    postError(LayerUnsupported{id, Library::LayerName(id)});
                });

  std::for_each(
      CI.requestedExtensions.begin(), CI.requestedExtensions.end(),
      [&library](ext id) {
        if (!library.hasInstanceExtension(id))
          postError(ExtensionUnsupported{id, Library::ExtensionName(id)});
      });

  std::vector<const char *> reqExtensionsNames{};
  std::vector<const char *> reqLayerNames{};

  std::transform(CI.requestedExtensions.begin(), CI.requestedExtensions.end(),
                 std::back_inserter(reqExtensionsNames),
                 [](auto id) { return Library::ExtensionName(id); });
  std::transform(CI.requestedLayers.begin(), CI.requestedLayers.end(),
                 std::back_inserter(reqLayerNames),
                 [](auto id) { return Library::LayerName(id); });

  createInfo.enabledExtensionCount = reqExtensionsNames.size();
  createInfo.ppEnabledExtensionNames = reqExtensionsNames.data();
  createInfo.enabledLayerCount = reqLayerNames.size();
  createInfo.ppEnabledLayerNames = reqLayerNames.data();

  VK_CHECK_RESULT(m_vulkanLib.get().vkCreateInstance(
      &createInfo, hostAllocator().allocator(), &m_instance))

  m_coreInstanceSymbols =
      loadInstanceSymbols(m_vulkanLib.get(), m_instance, CI.apiVersion);

  std::for_each(CI.requestedExtensions.begin(), CI.requestedExtensions.end(),
                [this](auto ext) { m_enabledExtensions.emplace(ext); });
  std::for_each(CI.requestedLayers.begin(), CI.requestedLayers.end(),
                [this](auto ext) { m_enabledLayers.emplace(ext); });
}

boost::container::small_vector<std::unique_ptr<PhysicalDevice>, 3>
Instance::enumerateAvailableDevices() const noexcept(ExceptionsDisabled) {
  boost::container::small_vector<VkPhysicalDevice, 3> devs;
  uint32_t deviceCount = 0;
  core<1, 0>().vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
  if (deviceCount == 0)
    return {};

  devs.resize(deviceCount);

  core<1, 0>().vkEnumeratePhysicalDevices(m_instance, &deviceCount,
                                          devs.data());

  boost::container::small_vector<std::unique_ptr<PhysicalDevice>, 3> ret;
  ret.reserve(devs.size());

  std::transform(devs.begin(), devs.end(), std::back_inserter(ret),
                 [this](VkPhysicalDevice device) {
                   return std::make_unique<PhysicalDevice>(*this, device);
                 });
  return ret;
}

Instance::Instance(Instance &&another) noexcept
    : m_instance(another.m_instance),
      m_enabledExtensions(std::move(another.m_enabledExtensions)),
      m_enabledLayers(std::move(another.m_enabledLayers)),
      m_vulkanLib(another.m_vulkanLib),
      m_coreInstanceSymbols(std::move(another.m_coreInstanceSymbols)),
      m_apiVer(another.m_apiVer) {
  another.m_instance = VK_NULL_HANDLE;
}

Instance &Instance::operator=(Instance &&another) noexcept {
  m_instance = another.m_instance;
  m_enabledExtensions = std::move(another.m_enabledExtensions);
  m_enabledLayers = std::move(another.m_enabledLayers);
  m_vulkanLib = another.m_vulkanLib;
  m_coreInstanceSymbols = std::move(another.m_coreInstanceSymbols);
  m_apiVer = another.m_apiVer;
  std::swap(m_instance, another.m_instance);

  return *this;
}

Instance::~Instance() {
  if (m_instance == VK_NULL_HANDLE)
    return;
  core<1, 0>().vkDestroyInstance(m_instance, hostAllocator().allocator());
}

} // namespace vkw
