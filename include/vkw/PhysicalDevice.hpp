#ifndef VKWRAPPER_PHYSICALDEVICE_HPP
#define VKWRAPPER_PHYSICALDEVICE_HPP

#include "Common.hpp"
#include <string>

namespace vkw {

namespace feature {

class FeatureBase {
public:
  virtual VkBool32 const *
  feature_location(VkPhysicalDeviceFeatures const *featureList) const = 0;
  virtual VkBool32 *
  feature_location(VkPhysicalDeviceFeatures *featureList) const = 0;
  virtual const char *name() const = 0;
};

#include "DeviceFeatures.inc"

} // namespace feature

enum class ext;

class PhysicalDevice {
public:
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

  bool isFeatureSupported(feature::FeatureBase const &feature) const;

  void enableFeature(feature::FeatureBase const &feature);

  bool extensionSupported(ext extension) const;

  void enableExtension(ext extension);

  std::vector<VkQueueFamilyProperties> const &queueProperties() const {
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
  std::vector<VkQueueFamilyProperties> m_queueFamilyProperties{};
  /** @brief List of extensions supported by the device */
  std::vector<ext> m_supportedExtensions{};

  std::vector<ext> m_enabledExtensions{};

  VkPhysicalDevice m_physicalDevice{};
  InstanceCRef m_instance;
};
} // namespace vkw
#endif // VKWRAPPER_PHYSICALDEVICE_HPP
