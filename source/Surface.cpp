#include "Surface.hpp"
#include "Instance.hpp"
#include "Utils.hpp"

namespace vkw {

Surface::~Surface() {
  if (m_surface != VK_NULL_HANDLE)
    vkDestroySurfaceKHR(m_parent, m_surface, nullptr);
}

std::vector<VkPresentModeKHR>
Surface::getAvailablePresentModes(VkPhysicalDevice device) const {
  uint32_t modeCount = 0;

  std::vector<VkPresentModeKHR> ret{};
  VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &modeCount, nullptr))

  if (modeCount == 0)
    return ret;

  ret.resize(modeCount);
  VK_CHECK_RESULT(vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &modeCount, ret.data()))

  return ret;
}

std::vector<VkSurfaceFormatKHR>
Surface::getAvailableFormats(VkPhysicalDevice device) const {
  uint32_t modeCount = 0;

  std::vector<VkSurfaceFormatKHR> ret{};
  VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface,
                                                       &modeCount, nullptr))

  if (modeCount == 0)
    return ret;

  ret.resize(modeCount);
  VK_CHECK_RESULT(vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface,
                                                       &modeCount, ret.data()))

  return ret;
}

VkSurfaceCapabilitiesKHR
Surface::getSurfaceCapabilities(VkPhysicalDevice device) const {
  VkSurfaceCapabilitiesKHR ret{};
  VK_CHECK_RESULT(
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &ret))
  return ret;
}

std::vector<uint32_t> Surface::getQueueFamilyIndicesThatSupportPresenting(
    VkPhysicalDevice device) const {
  uint32_t queueFamilyCount;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

  std::vector<uint32_t> ret;

  for (uint32_t i = 0; i < queueFamilyCount; ++i) {
    VkBool32 supported;
    VK_CHECK_RESULT(
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &supported))
    if (supported)
      ret.push_back(i);
  }

  return ret;
}
} // namespace vkr
