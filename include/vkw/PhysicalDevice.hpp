#ifndef VKWRAPPER_PHYSICALDEVICE_HPP
#define VKWRAPPER_PHYSICALDEVICE_HPP

#include <vkw/Instance.hpp>

namespace vkw {

enum class ext;

class QueueFamily {
public:
  enum { GRAPHICS = 0x1, TRANSFER = 0x2, COMPUTE = 0x4 };
  struct Type {
    unsigned value : 3;
    Type(unsigned val) noexcept : value(val) {}
  };

  QueueFamily(VkQueueFamilyProperties family, unsigned index) noexcept
      : m_family(family), m_index(index) {}

  bool strictly(Type type) const noexcept {
    VkQueueFlags flags = 0;
    if (type.value & GRAPHICS)
      flags |= VK_QUEUE_GRAPHICS_BIT;
    if (type.value & TRANSFER)
      flags |= VK_QUEUE_TRANSFER_BIT;
    if (type.value & COMPUTE)
      flags |= VK_QUEUE_COMPUTE_BIT;

    auto result = flags ^ m_family.queueFlags;
    return !(result & VK_QUEUE_GRAPHICS_BIT) &&
           !(result & VK_QUEUE_TRANSFER_BIT) &&
           !(result & VK_QUEUE_COMPUTE_BIT);
  }
  bool graphics() const noexcept {
    return m_family.queueFlags & VK_QUEUE_GRAPHICS_BIT;
  }

  bool transfer() const noexcept {
    return m_family.queueFlags & VK_QUEUE_TRANSFER_BIT;
  }

  bool compute() const noexcept {
    return m_family.queueFlags & VK_QUEUE_COMPUTE_BIT;
  }

  unsigned queueCount() const noexcept { return m_family.queueCount; }
  unsigned queueRequestedCount() const noexcept {
    return m_queuesRequested.size();
  }
  float queuePriority(unsigned id) const noexcept(ExceptionsDisabled) {
    return m_queuesRequested.at(id);
  }

  bool hasRequestedQueues() const noexcept {
    return !m_queuesRequested.empty();
  }
  float const *queuePrioritiesRaw() const noexcept {
    return m_queuesRequested.data();
  }

  unsigned index() const noexcept { return m_index; }

  void requestQueue(float priority = 0.0f) noexcept(ExceptionsDisabled) {
    if (m_queuesRequested.size() == m_family.queueCount) {
      std::stringstream ss;
      ss << "Requested for " << (m_family.queueCount + 1)
         << " queues in queue family index " << m_index
         << ", when it only supports at max " << m_family.queueCount;
      postError(Error(ss.str()));
    }
    m_queuesRequested.emplace_back(priority);
  }

private:
  std::vector<float> m_queuesRequested;
  unsigned m_index;
  VkQueueFamilyProperties m_family;
};
class PhysicalDevice : public ReferenceGuard {
public:
  using QueueFamilyContainerT = boost::container::small_vector<QueueFamily, 3>;
  enum class feature {
#define VKW_FEATURE_ENTRY(X) X,
#include "DeviceFeatures.inc"
#undef VKW_FEATURE_ENTRY
  };

  PhysicalDevice(Instance const &instance,
                 uint32_t id) noexcept(ExceptionsDisabled);
  PhysicalDevice(Instance const &instance,
                 VkPhysicalDevice device) noexcept(ExceptionsDisabled);
  PhysicalDevice(PhysicalDevice const &another) = default;
  PhysicalDevice(PhysicalDevice &&another) noexcept = default;
  PhysicalDevice &operator=(PhysicalDevice const &another) = default;
  PhysicalDevice &operator=(PhysicalDevice &&another) noexcept = default;
  virtual ~PhysicalDevice() = default;

  operator VkPhysicalDevice() const noexcept { return m_physicalDevice; }

  VkPhysicalDeviceProperties const &properties() const noexcept {
    return m_properties;
  }

  VkPhysicalDeviceMemoryProperties const &memoryProperties() const noexcept {
    return m_memoryProperties;
  }

  VkPhysicalDeviceFeatures const &supportedFeatures() const noexcept {
    return m_features;
  }

  VkPhysicalDeviceFeatures const &enabledFeatures() const noexcept {
    return m_enabledFeatures;
  }

  std::vector<ext> const &supportedExtensions() const noexcept {
    return m_supportedExtensions;
  }

  std::vector<ext> const &enabledExtensions() const noexcept {
    return m_enabledExtensions;
  }

  ApiVersion supportedApiVersion() const noexcept {
    return m_properties.apiVersion;
  }

  void requestApiVersion(ApiVersion version) noexcept(ExceptionsDisabled);

  ApiVersion requestedApiVersion() const noexcept {
    return m_requestedApiVersion;
  }

  bool isFeatureSupported(feature feature) const noexcept(ExceptionsDisabled);

  void enableFeature(feature feature) noexcept(ExceptionsDisabled);

  bool extensionSupported(ext extension) const noexcept(ExceptionsDisabled);

  void enableExtension(ext extension) noexcept(ExceptionsDisabled);

  QueueFamilyContainerT &queueFamilies() noexcept {
    return m_queueFamilyProperties;
  }

  QueueFamilyContainerT const &queueFamilies() const noexcept {
    return m_queueFamilyProperties;
  }

protected:
  /** @brief Properties of the physical device including limits that the
   * application can check against */
  VkPhysicalDeviceProperties m_properties{};
  /** @brief Features of the physical device that an application can use to
   * check if a feature is supported */
  VkPhysicalDeviceFeatures m_features{};
  /** @brief Features that have been enabled for use on the physical device */
  VkPhysicalDeviceFeatures m_enabledFeatures{};
  /** @brief Memory types and heaps of the physical device */
  VkPhysicalDeviceMemoryProperties m_memoryProperties{};
  /** @brief Queue family properties of the physical device */
  QueueFamilyContainerT m_queueFamilyProperties{};
  /** @brief List of extensions supported by the device */
  std::vector<ext> m_supportedExtensions{};

  std::vector<ext> m_enabledExtensions{};

  VkPhysicalDevice m_physicalDevice{};
  StrongReference<Instance const> m_instance;
  ApiVersion m_requestedApiVersion = ApiVersion{1, 0, 0};
};

class FeatureUnsupported : public Error {
public:
  FeatureUnsupported(PhysicalDevice::feature feature,
                     std::string_view featureName) noexcept
      : Error(std::string("Feature ")
                  .append(featureName)
                  .append(" is unsupported"),
              ErrorCode::FEATURE_UNSUPPORTED),
        m_feature(feature) {}
  PhysicalDevice::feature feature() const noexcept { return m_feature; }

private:
  PhysicalDevice::feature m_feature;
};

} // namespace vkw
#endif // VKWRAPPER_PHYSICALDEVICE_HPP
