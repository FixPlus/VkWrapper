#include "vkw/Instance.hpp"
#include "Utils.hpp"
#include "Validation.hpp"
#include "loader/DynamicLoader.hpp"
#include "vkw/Device.hpp"
#include "vkw/Exception.hpp"
#include "vkw/Extensions.hpp"
#include <cassert>
#include <cstring>

namespace vkw {

Instance::Instance(Library const &library, std::vector<ext> reqExtensions,
                   bool enableValidation)
    : m_validation(enableValidation), m_vulkanLib(library) {
  VkApplicationInfo appInfo{};

  // TODO: all this information should be filled externally using wrapper
  // structures to hold it

  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = "APITest";
  appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.pEngineName = "APITest";
  appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
  appInfo.apiVersion = VK_API_VERSION_1_0;

  m_apiVer = {1, 0, 0};

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  const char *validationLayerName = "VK_LAYER_KHRONOS_validation";

  if (m_validation) {

    if (library.hasLayer(validationLayerName)) {
      createInfo.ppEnabledLayerNames = &validationLayerName;
      createInfo.enabledLayerCount = 1;
    } else {
      createInfo.enabledLayerCount = 0;
      m_validation = false;
      throw Error("Validation layer VK_LAYER_KHRONOS_validation not present!");
    }
  }

  // validation layer requires VK_EXT_debug_utils extension
  if (m_validation)
    reqExtensions.push_back(ext::EXT_debug_utils);

  std::vector<const char *> reqExtensionsNames{};
  reqExtensionsNames.reserve(reqExtensions.size());

  std::transform(reqExtensions.begin(), reqExtensions.end(),
                 std::back_inserter(reqExtensionsNames), [this](ext id) {
                   return m_vulkanLib.get().extensionName(id);
                 });

  createInfo.enabledExtensionCount = reqExtensionsNames.size();
  createInfo.ppEnabledExtensionNames = reqExtensionsNames.data();

  VK_CHECK_RESULT(
      m_vulkanLib.get().vkCreateInstance(&createInfo, nullptr, &m_instance))

  // TODO: add option to load specific Vulkan version symbols
  m_coreInstanceSymbols = std::make_unique<InstanceCore<1, 0>>(
      m_vulkanLib.get().vkGetInstanceProcAddr, m_instance);

  for (auto &extension : reqExtensions) {
    m_enabledExtensions.emplace(extension);
  }

  if (m_validation)
    debug::setupDebugging(*this, 0, nullptr);
}

std::vector<PhysicalDevice> Instance::enumerateAvailableDevices() const {
  std::vector<VkPhysicalDevice> devs;
  uint32_t deviceCount = 0;
  core<1, 0>().vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
  if (deviceCount == 0)
    return {};

  devs.resize(deviceCount);

  core<1, 0>().vkEnumeratePhysicalDevices(m_instance, &deviceCount,
                                          devs.data());

  std::vector<PhysicalDevice> ret;
  ret.reserve(devs.size());

  std::transform(devs.begin(), devs.end(), std::back_inserter(ret),
                 [this](VkPhysicalDevice device) {
                   return PhysicalDevice{*this, device};
                 });

  return ret;
}

Instance::Instance(Instance &&another) noexcept
    : m_instance(another.m_instance), m_validation(another.m_validation),
      m_enabledExtensions(std::move(another.m_enabledExtensions)),
      m_vulkanLib(another.m_vulkanLib),
      m_coreInstanceSymbols(std::move(another.m_coreInstanceSymbols)) {
  another.m_instance = VK_NULL_HANDLE;
}

Instance &Instance::operator=(Instance &&another) noexcept {
  m_instance = another.m_instance;
  m_validation = another.m_validation;
  m_enabledExtensions = std::move(another.m_enabledExtensions);
  m_vulkanLib = another.m_vulkanLib;
  m_coreInstanceSymbols = std::move(another.m_coreInstanceSymbols);
  another.m_instance = VK_NULL_HANDLE;

  return *this;
}

Instance::~Instance() {
  if (m_instance == VK_NULL_HANDLE)
    return;

  if (m_validation)
    debug::freeDebugCallback(*this);

  core<1, 0>().vkDestroyInstance(m_instance, nullptr);
}

} // namespace vkw
