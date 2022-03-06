#include "Surface.hpp"
#include "Instance.hpp"
#include "Utils.hpp"

namespace vkw {

Surface::~Surface() {
  if (m_surface != VK_NULL_HANDLE)
    m_surface_ext->vkDestroySurfaceKHR(m_parent, m_surface, nullptr);
}
#if _WIN32
Surface::Surface(Instance &parent, HINSTANCE hinstance, HWND hwnd)
    : m_parent(parent) {
  m_surface_ext = static_cast<VkSurfaceKHRExtension const *>(
      parent.getExtension(VK_KHR_SURFACE_EXTENSION_NAME));
  if (!m_surface_ext)
    throw ExtensionMissing(VK_KHR_SURFACE_EXTENSION_NAME);
  auto *win32SurfaceExt = static_cast<VkSurfaceWin32KHRExtension const *>(
      parent.getExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME));
  if (!win32SurfaceExt)
    throw ExtensionMissing(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

  VkWin32SurfaceCreateInfoKHR createInfo{};

  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.hinstance = hinstance;
  createInfo.hwnd = hwnd;

  VK_CHECK_RESULT(win32SurfaceExt->vkCreateWin32SurfaceKHR(
      m_parent, &createInfo, nullptr, &m_surface))
}
#endif
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
  m_parent.core_1_0().vkGetPhysicalDeviceQueueFamilyProperties(
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
