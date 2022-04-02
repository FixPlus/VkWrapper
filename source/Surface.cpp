#include "vkw/Surface.hpp"
#include "Utils.hpp"
#include "vkw/Instance.hpp"

namespace vkw {

Surface::~Surface() {
  if (m_surface != VK_NULL_HANDLE)
    m_surface_ext->vkDestroySurfaceKHR(m_parent.get(), m_surface, nullptr);
}
std::vector<VkPresentModeKHR>
Surface::getAvailablePresentModes(VkPhysicalDevice device) const {
  uint32_t modeCount = 0;

  std::vector<VkPresentModeKHR> ret{};
  VK_CHECK_RESULT(m_surface_ext->vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &modeCount, nullptr))

  if (modeCount == 0)
    return ret;

  ret.resize(modeCount);
  VK_CHECK_RESULT(m_surface_ext->vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &modeCount, ret.data()))

  return ret;
}

std::vector<VkSurfaceFormatKHR>
Surface::getAvailableFormats(VkPhysicalDevice device) const {
  uint32_t modeCount = 0;

  std::vector<VkSurfaceFormatKHR> ret{};
  VK_CHECK_RESULT(m_surface_ext->vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, m_surface, &modeCount, nullptr))

  if (modeCount == 0)
    return ret;

  ret.resize(modeCount);
  VK_CHECK_RESULT(m_surface_ext->vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, m_surface, &modeCount, ret.data()))

  return ret;
}

VkSurfaceCapabilitiesKHR
Surface::getSurfaceCapabilities(VkPhysicalDevice device) const {
  VkSurfaceCapabilitiesKHR ret{};
  VK_CHECK_RESULT(m_surface_ext->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, m_surface, &ret))
  return ret;
}

std::vector<uint32_t> Surface::getQueueFamilyIndicesThatSupportPresenting(
    VkPhysicalDevice device) const {
  uint32_t queueFamilyCount;
  m_parent.get().core<1, 0>().vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queueFamilyCount, nullptr);

  std::vector<uint32_t> ret;

  for (uint32_t i = 0; i < queueFamilyCount; ++i) {
    VkBool32 supported;
    VK_CHECK_RESULT(m_surface_ext->vkGetPhysicalDeviceSurfaceSupportKHR(
        device, i, m_surface, &supported))
    if (supported)
      ret.push_back(i);
  }

  return ret;
}
} // namespace vkw
