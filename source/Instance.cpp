#include "Instance.hpp"
#include "Device.hpp"
#include "Exception.hpp"
#include "Utils.hpp"
#include "loader/DynamicLoader.hpp"
#include <cassert>

namespace vkw {

Instance::Instance(Library const &library,
                   std::vector<std::string> reqExtensions,
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

  std::vector<const char *> reqExtensionsNames{};
  reqExtensionsNames.reserve(reqExtensions.size());

  for (auto &ext : reqExtensions)
    reqExtensionsNames.emplace_back(ext.c_str());

  VkInstanceCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;

  if (m_validation) {
    const char *validationLayerName = "VK_LAYER_KHRONOS_validation";
    uint32_t instanceLayerCount;
    m_vulkanLib.get().vkEnumerateInstanceLayerProperties(&instanceLayerCount,
                                                         nullptr);
    std::vector<VkLayerProperties> instanceLayerProperties(instanceLayerCount);
    m_vulkanLib.get().vkEnumerateInstanceLayerProperties(
        &instanceLayerCount, instanceLayerProperties.data());
    bool validationLayerPresent = false;
    for (VkLayerProperties layer : instanceLayerProperties) {
      if (strcmp(layer.layerName, validationLayerName) == 0) {
        validationLayerPresent = true;
        break;
      }
    }
    if (validationLayerPresent) {
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
    reqExtensionsNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

  createInfo.enabledExtensionCount = reqExtensionsNames.size();
  createInfo.ppEnabledExtensionNames = reqExtensionsNames.data();

  VK_CHECK_RESULT(
      m_vulkanLib.get().vkCreateInstance(&createInfo, nullptr, &m_instance))

  m_coreInstanceSymbols = std::make_unique<InstanceCore<1, 0>>(
      m_vulkanLib.get().vkGetInstanceProcAddr, m_instance);

  for (auto &extName : reqExtensionsNames) {
    std::unique_ptr<InstanceExtensionBase> ext;
    auto initializer = m_instanceExtInitializers.find(extName);
    assert(initializer != m_instanceExtInitializers.end() &&
           "Bad extension name");
    m_extensions.emplace(
        extName,
        std::unique_ptr<InstanceExtensionBase>(initializer->second->initialize(
            m_vulkanLib.get().vkGetInstanceProcAddr, m_instance)));
  }

  std::cout << "Vulkan initialized successfully" << std::endl;
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
      m_extensions(std::move(another.m_extensions)),
      m_vulkanLib(another.m_vulkanLib),
      m_coreInstanceSymbols(std::move(another.m_coreInstanceSymbols)) {
  another.m_instance = VK_NULL_HANDLE;
}

Instance &Instance::operator=(Instance &&another) noexcept {
  m_instance = another.m_instance;
  m_validation = another.m_validation;
  m_extensions = std::move(another.m_extensions);
  m_vulkanLib = another.m_vulkanLib;
  m_coreInstanceSymbols = std::move(another.m_coreInstanceSymbols);
  another.m_instance = VK_NULL_HANDLE;

  return *this;
}

Instance::~Instance() {
  if (m_instance == VK_NULL_HANDLE)
    return;

  core<1, 0>().vkDestroyInstance(m_instance, nullptr);
}

} // namespace vkw
