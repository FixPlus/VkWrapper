#include "vkw/Surface.hpp"
#include "Utils.hpp"
#include "vkw/Instance.hpp"

namespace vkw {

#ifdef VK_USE_PLATFORM_WIN32_KHR
Surface::Surface(Instance const &parent, HINSTANCE hinstance,
                 HWND hwnd) noexcept(ExceptionsDisabled)
    : m_parent(parent), m_surface_ext(parent) {
  Extension<ext::KHR_win32_surface> win32SurfaceExt{parent};

  VkWin32SurfaceCreateInfoKHR createInfo{};

  createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.hinstance = hinstance;
  createInfo.hwnd = hwnd;

  VK_CHECK_RESULT(win32SurfaceExt.vkCreateWin32SurfaceKHR(
      m_parent.get(), &createInfo, parent.hostAllocator().allocator(),
      &m_surface))
}
#elif VK_USE_PLATFORM_XLIB_KHR
Surface::Surface(Instance const &parent, Display *display,
                 Window window) noexcept(ExceptionsDisabled)
    : m_parent(parent), m_surface_ext(parent) {
  Extension<ext::KHR_xlib_surface> xlibSurfaceExt{parent};

  VkXlibSurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = nullptr;
  createInfo.dpy = display;
  createInfo.window = window;

  VK_CHECK_RESULT_(xlibSurfaceExt.vkCreateXlibSurfaceKHR(
      m_parent.get(), &createInfo, parent.hostAllocator().allocator(),
      &m_surface))
};
#elif defined VK_USE_PLATFORM_XCB_KHR
Surface::Surface(Instance const &parent, xcb_connection_t *connection,
                 xcb_window_t window) noexcept(ExceptionsDisabled)
    : m_parent(parent), m_surface_ext(parent) {
  Extension<ext::KHR_xcb_surface> xcbSurfaceExt{parent};

  VkXcbSurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = nullptr;
  createInfo.connection = connection;
  createInfo.window = window;

  VK_CHECK_RESULT_(xcbSurfaceExt.vkCreateXCBSurfaceKHR(
      m_parent, &createInfo, parent.hostAllocator().allocator(), &m_surface))
};
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
Surface::Surface(Instance const &parent, wl_display *display,
                 wl_surface *surface) noexcept(ExceptionsDisabled)
    : m_parent(parent), m_surface_ext(parent) {
  Extension<ext::KHR_wayland_surface> waylandSurfaceExt{parent};

  VkWaylandSurfaceCreateInfoKHR createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = nullptr;
  createInfo.display = display;
  createInfo.surface = surface;

  VK_CHECK_RESULT_(waylandSurfaceExt.vkCreateWaylandSurfaceKHR(
      m_parent, &createInfo, parent.hostAllocator().allocator(), &m_surface))
};
#endif

Surface::~Surface() {
  if (m_surface != VK_NULL_HANDLE)
    m_surface_ext.vkDestroySurfaceKHR(
        m_parent.get(), m_surface, m_parent.get().hostAllocator().allocator());
}
std::vector<VkPresentModeKHR>
Surface::getAvailablePresentModes(VkPhysicalDevice device) const
    noexcept(ExceptionsDisabled) {
  uint32_t modeCount = 0;

  std::vector<VkPresentModeKHR> ret{};
  VK_CHECK_RESULT(m_surface_ext.vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &modeCount, nullptr))

  if (modeCount == 0)
    return ret;

  ret.resize(modeCount);
  VK_CHECK_RESULT(m_surface_ext.vkGetPhysicalDeviceSurfacePresentModesKHR(
      device, m_surface, &modeCount, ret.data()))

  return ret;
}

std::vector<VkSurfaceFormatKHR>
Surface::getAvailableFormats(VkPhysicalDevice device) const
    noexcept(ExceptionsDisabled) {
  uint32_t modeCount = 0;

  std::vector<VkSurfaceFormatKHR> ret{};
  VK_CHECK_RESULT(m_surface_ext.vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, m_surface, &modeCount, nullptr))

  if (modeCount == 0)
    return ret;

  ret.resize(modeCount);
  VK_CHECK_RESULT(m_surface_ext.vkGetPhysicalDeviceSurfaceFormatsKHR(
      device, m_surface, &modeCount, ret.data()))

  return ret;
}

VkSurfaceCapabilitiesKHR
Surface::getSurfaceCapabilities(VkPhysicalDevice device) const
    noexcept(ExceptionsDisabled) {
  VkSurfaceCapabilitiesKHR ret{};
  VK_CHECK_RESULT(m_surface_ext.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      device, m_surface, &ret))
  return ret;
}

std::vector<uint32_t> Surface::getQueueFamilyIndicesThatSupportPresenting(
    VkPhysicalDevice device) const noexcept(ExceptionsDisabled) {
  uint32_t queueFamilyCount;
  m_parent.get().core<1, 0>().vkGetPhysicalDeviceQueueFamilyProperties(
      device, &queueFamilyCount, nullptr);

  std::vector<uint32_t> ret;

  for (uint32_t i = 0; i < queueFamilyCount; ++i) {
    VkBool32 supported;
    VK_CHECK_RESULT(m_surface_ext.vkGetPhysicalDeviceSurfaceSupportKHR(
        device, i, m_surface, &supported))
    if (supported)
      ret.push_back(i);
  }

  return ret;
}
} // namespace vkw
