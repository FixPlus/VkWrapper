#include "vkw/PhysicalDevice.hpp"
#include "Utils.hpp"
#include "vkw/Instance.hpp"
#include <sstream>
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

  std::vector<VkQueueFamilyProperties> rawQueueProps;
  rawQueueProps.resize(queueFamilyCount);
  instance.core<1, 0>().vkGetPhysicalDeviceQueueFamilyProperties(
      m_physicalDevice, &queueFamilyCount, rawQueueProps.data());
  unsigned indexAcc = 0;
  std::transform(rawQueueProps.begin(), rawQueueProps.end(),
                 std::back_inserter(m_queueFamilyProperties),
                 [&indexAcc](auto rawProp) {
                   return QueueFamily{rawProp, indexAcc++};
                 });

  // Get list of supported extensions
  uint32_t extCount = 0;
  instance.core<1, 0>().vkEnumerateDeviceExtensionProperties(
      m_physicalDevice, nullptr, &extCount, nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (instance.core<1, 0>().vkEnumerateDeviceExtensionProperties(
            m_physicalDevice, nullptr, &extCount, &extensions.front()) ==
        VK_SUCCESS) {

      /*
       *
       * If there are no corresponding entry in extension map
       * (which throws Error("Bad extension name")),
       * do not list it as supported
       *
       */
      auto newEnd =
          std::remove_if(extensions.begin(), extensions.end(),
                         [this](VkExtensionProperties const &props) {
                           try {
                             m_supportedExtensions.emplace_back(
                                 Library::ExtensionId(props.extensionName));
                             return false;
                           } catch (Error &e) {
                             return true;
                           }
                         });

      extensions.erase(newEnd, extensions.end());
    }
  }
}

namespace {

void unhandledFeatureEntry(PhysicalDevice::feature feature) {
  std::stringstream ss;
  ss << "Unhandled feature entry: " << static_cast<unsigned>(feature);
  throw Error(ss.view());
}
const char *featureNameMap(PhysicalDevice::feature feature) {
  switch (feature) {
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature::X:                                             \
    return #X;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
  default:
    unhandledFeatureEntry(feature);
    return nullptr;
  }
}
} // namespace

bool PhysicalDevice::isFeatureSupported(feature feature) const {
  switch (feature) {
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature::X:                                             \
    return m_features.X;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
  default:
    unhandledFeatureEntry(feature);
    return false;
  }
}

void PhysicalDevice::enableFeature(feature feature) {
  if (!isFeatureSupported(feature))
    throw FeatureUnsupported(feature, featureNameMap(feature));

  switch (feature) {
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature::X:                                             \
    m_enabledFeatures.X = true;                                                \
    break;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
  default:
    unhandledFeatureEntry(feature);
  };
}

bool PhysicalDevice::extensionSupported(ext extension) const {
  return std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(),
                   extension) != m_supportedExtensions.end();
}

void PhysicalDevice::enableExtension(ext extension) {
  if (!extensionSupported(extension))
    throw ExtensionUnsupported(
        extension, m_instance.get().parent().ExtensionName(extension));

  if (std::find(m_enabledExtensions.begin(), m_enabledExtensions.end(),
                extension) != m_enabledExtensions.end())
    return;

  m_enabledExtensions.emplace_back(extension);
}
} // namespace vkw