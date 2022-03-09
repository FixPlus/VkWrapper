#include "PhysicalDevice.hpp"
#include "Instance.hpp"
#include "Utils.hpp"
namespace vkw {

namespace {

VkPhysicalDevice enumerateAndGet(Instance const &instance, uint32_t id) {
  uint32_t device_count{};
  VK_CHECK_RESULT(instance.core<1, 0>().vkEnumeratePhysicalDevices(
      instance, &device_count, nullptr))

  if (device_count < id)
    throw Error("Failed to retrieve device info for #" + std::to_string(id) +
                ": Vulkan has only " + std::to_string(device_count) +
                " devices available");

  std::vector<VkPhysicalDevice> m_all_devs{device_count};

  VK_CHECK_RESULT(instance.core<1, 0>().vkEnumeratePhysicalDevices(
      instance, &device_count, m_all_devs.data()))

  return m_all_devs.at(id);
}

} // namespace

PhysicalDevice::PhysicalDevice(Instance const &instance, uint32_t id)
    : PhysicalDevice(instance, enumerateAndGet(instance, id)) {}
PhysicalDevice::PhysicalDevice(Instance const &instance,
                               VkPhysicalDevice device)
    : m_instance(instance), m_physicalDevice(device) {

  // Store Properties features, limits and properties of the physical device for
  // later use Device properties also contain limits and sparse properties
  instance.core<1, 0>().vkGetPhysicalDeviceProperties(m_physicalDevice,
                                                      &m_properties);
  // Features should be checked by the examples before using them
  instance.core<1, 0>().vkGetPhysicalDeviceFeatures(m_physicalDevice,
                                                    &m_features);
  // Memory properties are used regularly for creating all kinds of buffers
  instance.core<1, 0>().vkGetPhysicalDeviceMemoryProperties(
      m_physicalDevice, &m_memoryProperties);
  // Queue family properties, used for setting up requested queues upon device
  // creation
  uint32_t queueFamilyCount;
  instance.core<1, 0>().vkGetPhysicalDeviceQueueFamilyProperties(
      m_physicalDevice, &queueFamilyCount, nullptr);

  m_queueFamilyProperties.resize(queueFamilyCount);
  instance.core<1, 0>().vkGetPhysicalDeviceQueueFamilyProperties(
      m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

  // Get list of supported extensions
  uint32_t extCount = 0;
  instance.core<1, 0>().vkEnumerateDeviceExtensionProperties(
      m_physicalDevice, nullptr, &extCount, nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (instance.core<1, 0>().vkEnumerateDeviceExtensionProperties(
            m_physicalDevice, nullptr, &extCount, &extensions.front()) ==
        VK_SUCCESS) {
      for (auto ext : extensions) {
        m_supportedExtensions.emplace_back(ext.extensionName);
      }
    }
  }
}

bool PhysicalDevice::isFeatureSupported(
    feature::FeatureBase const &feature) const {
  return *feature.feature_location(&m_features);
}

void PhysicalDevice::enableFeature(feature::FeatureBase const &feature) {
  if (!isFeatureSupported(feature))
    throw Error("Asked to enable feature " + std::string(feature.name()) +
                " which is not supported by physicalDevice " +
                std::to_string((unsigned long long)m_physicalDevice));

  *feature.feature_location(&m_enabledFeatures) = VK_TRUE;
}

bool PhysicalDevice::extensionSupported(std::string const &extension) const {
  return std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(),
                   extension) != m_supportedExtensions.end();
}

void PhysicalDevice::enableExtension(std::string const &extension) {
  if (!extensionSupported(extension))
    throw Error("Asked to enable extension \"" + extension +
                    "\", which is not supported by physicalDevice " +
                    std::to_string((unsigned long long)m_physicalDevice),
                ErrorCode::EXTENSION_MISSING);

  if (std::find(m_enabledExtensions.begin(), m_enabledExtensions.end(),
                extension) != m_enabledExtensions.end())
    return;

  m_enabledExtensions.emplace_back(extension.c_str());
}
} // namespace vkw