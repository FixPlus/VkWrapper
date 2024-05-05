#include "vkw/PhysicalDevice.hpp"
#include "Utils.hpp"
#include "vkw/Instance.hpp"
#include <algorithm>
#include <sstream>

namespace vkw {

namespace {

VkPhysicalDevice enumerateAndGet(Instance const &instance,
                                 uint32_t id) noexcept(ExceptionsDisabled) {
  uint32_t device_count{};
  VK_CHECK_RESULT(instance.core<1, 0>().vkEnumeratePhysicalDevices(
      instance, &device_count, nullptr))

  if (device_count < id)
    postError(Error("Failed to retrieve device info for #" +
                    std::to_string(id) + ": Vulkan has only " +
                    std::to_string(device_count) + " devices available"));

  std::vector<VkPhysicalDevice> m_all_devs{device_count};

  VK_CHECK_RESULT(instance.core<1, 0>().vkEnumeratePhysicalDevices(
      instance, &device_count, m_all_devs.data()))

  return m_all_devs.at(id);
}

} // namespace

PhysicalDevice::PhysicalDevice(Instance const &instance,
                               uint32_t id) noexcept(ExceptionsDisabled)
    : PhysicalDevice(instance, enumerateAndGet(instance, id)) {}
PhysicalDevice::PhysicalDevice(
    Instance const &instance,
    VkPhysicalDevice device) noexcept(ExceptionsDisabled)
    : m_instance(instance), m_physicalDevice(device) {

  // Store Properties features, limits and properties of the physical device for
  // later use Device properties also contain limits and sparse properties
  instance.core<1, 0>().vkGetPhysicalDeviceProperties(m_physicalDevice,
                                                      &m_properties);
  // Features should be checked by the examples before using them
  instance.core<1, 0>().vkGetPhysicalDeviceFeatures(m_physicalDevice,
                                                    &m_features);
#ifdef VK_VERSION_1_2
  m_vulkan11Features.pNext = nullptr;
  m_vulkan11Features.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
  m_enabledVulkan11Features.pNext = nullptr;
  m_enabledVulkan11Features.sType =
      VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;

  VkPhysicalDeviceFeatures2 feats{};
  feats.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
  feats.pNext = &m_vulkan11Features;
  instance.core<1, 1>().vkGetPhysicalDeviceFeatures2(m_physicalDevice, &feats);
#endif
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

      for (auto &props : extensions) {
        if (!Library::ValidExtensionName(props.extensionName))
          continue;
        m_supportedExtensions.emplace_back(
            Library::ExtensionId(props.extensionName));
      }
    }
  }
}

namespace {

void unhandledFeatureEntry(auto feature) {
  std::stringstream ss;
  ss << "Unhandled feature entry: " << static_cast<unsigned>(feature);
  postError(Error(ss.view()));
}

const char *featureNameMap(PhysicalDevice::feature feature) {
  switch (feature) {
#define VKW_DUMP_FEATURES
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature::X:                                             \
    return #X;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
#undef VKW_DUMP_FEATURES
  default:
    unhandledFeatureEntry(feature);
    return nullptr;
  }
}
#ifdef VK_VERSION_1_2
const char *featureNameMap(PhysicalDevice::feature_v11 feature) {
  switch (feature) {
#define VKW_DUMP_VULKAN11_FEATURES
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature_v11::X:                                         \
    return #X;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
#undef VKW_DUMP_VULKAN11_FEATURES
  default:
    unhandledFeatureEntry(feature);
    return nullptr;
  }
}
#endif

} // namespace

bool PhysicalDevice::isFeatureSupported(feature feature) const
    noexcept(ExceptionsDisabled) {
  switch (feature) {
#define VKW_DUMP_FEATURES
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature::X:                                             \
    return m_features.X;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
#undef VKW_DUMP_FEATURES
  default:
    unhandledFeatureEntry(feature);
    return false;
  }
}

void PhysicalDevice::enableFeature(feature feature) noexcept(
    ExceptionsDisabled) {
  if (!isFeatureSupported(feature))
    postError(FeatureUnsupported(feature, featureNameMap(feature)));

  switch (feature) {
#define VKW_DUMP_FEATURES
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature::X:                                             \
    m_enabledFeatures.X = true;                                                \
    break;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
#undef VKW_DUMP_FEATURES
  default:
    unhandledFeatureEntry(feature);
  };
}

#ifdef VK_VERSION_1_2
bool PhysicalDevice::isFeatureSupported(feature_v11 feature) const
    noexcept(ExceptionsDisabled) {
  switch (feature) {
#define VKW_DUMP_VULKAN11_FEATURES
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature_v11::X:                                         \
    return m_vulkan11Features.X;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
#undef VKW_DUMP_VULKAN11_FEATURES
  default:
    unhandledFeatureEntry(feature);
    return false;
  }
}

void PhysicalDevice::enableFeature(feature_v11 feature) noexcept(
    ExceptionsDisabled) {
  if (!isFeatureSupported(feature))
    postError(FeatureUnsupported(feature, featureNameMap(feature)));

  switch (feature) {
#define VKW_DUMP_VULKAN11_FEATURES
#define VKW_FEATURE_ENTRY(X)                                                   \
  case PhysicalDevice::feature_v11::X:                                         \
    m_enabledVulkan11Features.X = true;                                        \
    break;
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
#undef VKW_DUMP_VULKAN11_FEATURES
  default:
    unhandledFeatureEntry(feature);
  };
}
#endif

bool PhysicalDevice::extensionSupported(ext extension) const
    noexcept(ExceptionsDisabled) {
  return std::find(m_supportedExtensions.begin(), m_supportedExtensions.end(),
                   extension) != m_supportedExtensions.end();
}

void PhysicalDevice::enableExtension(ext extension) noexcept(
    ExceptionsDisabled) {
  if (!extensionSupported(extension))
    postError(ExtensionUnsupported(
        extension, m_instance.get().parent().ExtensionName(extension)));

  if (std::find(m_enabledExtensions.begin(), m_enabledExtensions.end(),
                extension) != m_enabledExtensions.end())
    return;

  m_enabledExtensions.emplace_back(extension);
}
void PhysicalDevice::requestApiVersion(ApiVersion version) noexcept(
    ExceptionsDisabled) {
  if (version > supportedApiVersion())
    postError(
        ApiVersionUnsupported("Cannot create device with requested version",
                              supportedApiVersion(), version));
  m_requestedApiVersion = version;
}
} // namespace vkw