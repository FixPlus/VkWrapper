#ifndef VKWRAPPER_PHYSICALDEVICE_HPP
#define VKWRAPPER_PHYSICALDEVICE_HPP

#include "vkw/Exception.hpp"
#include "vkw/Instance.hpp"
#include <boost/container/small_vector.hpp>
#include <sstream>
#include <string>
#include <vector>

namespace vkw {

enum class ext;

class QueueFamily {
public:
  enum { GRAPHICS = 0x1, TRANSFER = 0x2, COMPUTE = 0x4 };
  struct Type {
    unsigned value : 3;
    Type(unsigned val) : value(val) {}
  };

  QueueFamily(VkQueueFamilyProperties family, unsigned index)
      : m_family(family), m_index(index) {}

  bool strictly(Type type) const {
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
  bool graphics() const { return m_family.queueFlags & VK_QUEUE_GRAPHICS_BIT; }

  bool transfer() const { return m_family.queueFlags & VK_QUEUE_TRANSFER_BIT; }

  bool compute() const { return m_family.queueFlags & VK_QUEUE_COMPUTE_BIT; }

  unsigned queueCount() const { return m_family.queueCount; }
  unsigned queueRequestedCount() const { return m_queuesRequested.size(); }
  float queuePriority(unsigned id) const { return m_queuesRequested.at(id); }

  bool hasRequestedQueues() const { return !m_queuesRequested.empty(); }
  float const *queuePrioritiesRaw() const { return m_queuesRequested.data(); }

  unsigned index() const { return m_index; }

  void requestQueue(float priority = 0.0f) {
    if (m_queuesRequested.size() == m_family.queueCount) {
      std::stringstream ss;
      ss << "Requested for " << (m_family.queueCount + 1)
         << " queues in queue family index " << m_index
         << ", when it only supports at max " << m_family.queueCount;
      throw Error(ss.str());
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

  PhysicalDevice(Instance const &instance, uint32_t id);
  PhysicalDevice(Instance const &instance, VkPhysicalDevice device);
  PhysicalDevice(PhysicalDevice const &another) = default;
  PhysicalDevice(PhysicalDevice &&another) noexcept = default;
  PhysicalDevice &operator=(PhysicalDevice const &another) = default;
  PhysicalDevice &operator=(PhysicalDevice &&another) noexcept = default;
  virtual ~PhysicalDevice() = default;

  operator VkPhysicalDevice() const { return m_physicalDevice; }

  VkPhysicalDeviceProperties const &properties() const { return m_properties; }

  VkPhysicalDeviceMemoryProperties const &memoryProperties() const {
    return m_memoryProperties;
  }

  VkPhysicalDeviceFeatures const &supportedFeatures() const {
    return m_features;
  }

  VkPhysicalDeviceFeatures const &enabledFeatures() const {
    return m_enabledFeatures;
  }

  std::vector<ext> const &supportedExtensions() const {
    return m_supportedExtensions;
  }

  std::vector<ext> const &enabledExtensions() const {
    return m_enabledExtensions;
  }

  bool isFeatureSupported(feature feature) const;

  void enableFeature(feature feature);

  bool extensionSupported(ext extension) const;

  void enableExtension(ext extension);

  QueueFamilyContainerT &queueFamilies() { return m_queueFamilyProperties; }

  QueueFamilyContainerT const &queueFamilies() const {
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
};

class FeatureUnsupported : public Error {
public:
  FeatureUnsupported(PhysicalDevice::feature feature,
                     std::string_view featureName)
      : Error(std::string("Feature ")
                  .append(featureName)
                  .append(" is unsupported"),
              ErrorCode::FEATURE_UNSUPPORTED),
        m_feature(feature) {}
  PhysicalDevice::feature feature() const { return m_feature; }

private:
  PhysicalDevice::feature m_feature;
};

} // namespace vkw
#endif // VKWRAPPER_PHYSICALDEVICE_HPP
