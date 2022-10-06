#ifndef VKRENDERER_SURFACE_HPP
#define VKRENDERER_SURFACE_HPP

#include "Extensions.hpp"
#include "vkw/Instance.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Instance;

#define VK_CHECK_RESULT_(...)                                                  \
  {                                                                            \
    VkResult res = (__VA_ARGS__);                                              \
    if (res != VK_SUCCESS)                                                     \
      throw VulkanError(res, __FILE__, __LINE__);                              \
  }

class Surface {
public:
  Surface(Instance const &parent, VkSurfaceKHR surface)
      : m_parent(parent), m_surface_ext(parent), m_surface(surface) {}
#ifdef _WIN32
  Surface(Instance const &parent, HINSTANCE hinstance, HWND hwnd)
      : m_parent(parent), m_surface_ext(parent) {
    Extension<ext::KHR_win32_surface> win32SurfaceExt{parent};

    VkWin32SurfaceCreateInfoKHR createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = hinstance;
    createInfo.hwnd = hwnd;

    VK_CHECK_RESULT_(win32SurfaceExt.vkCreateWin32SurfaceKHR(
        m_parent.get(), &createInfo, nullptr, &m_surface))
  }
#elif defined __linux__
#ifdef VK_USE_PLATFORM_XLIB_KHR
  Surface(Instance const &parent, Display *display, Window window)
      : m_parent(parent), m_surface_ext(parent) {
    Extension<ext::KHR_xlib_surface> xlibSurfaceExt{parent};

    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.dpy = display;
    createInfo.window = window;

    VK_CHECK_RESULT_(xlibSurfaceExt.vkCreateXlibSurfaceKHR(
        m_parent.get(), &createInfo, nullptr, &m_surface))
  };
#elif defined VK_USE_PLATFORM_XCB_KHR
  Surface(Instance const &parent, xcb_connection_t *connection,
          xcb_window_t window)
      : m_parent(parent), m_surface_ext(parent) {
    Extension<ext::KHR_xcb_surface> xcbSurfaceExt{parent};

    VkXcbSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.connection = connection;
    createInfo.window = window;

    VK_CHECK_RESULT_(xcbSurfaceExt.vkCreateXCBSurfaceKHR(m_parent, &createInfo,
                                                         nullptr, &m_surface))
  };
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
  Surface(Instance const &parent, wl_display *display, wl_surface *surface)
      : m_parent(parent), m_surface_ext(parent) {
    Extension<ext::KHR_wayland_surface> waylandSurfaceExt{parent};

    VkWaylandSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.display = display;
    createInfo.surface = surface;

    VK_CHECK_RESULT_(waylandSurfaceExt.vkCreateWaylandSurfaceKHR(
        m_parent, &createInfo, nullptr, &m_surface))
  };
#endif
#endif

  Surface(Surface const &another) = delete;
  Surface(Surface &&another) noexcept
      : m_parent(another.m_parent), m_surface(another.m_surface),
        m_surface_ext(another.m_surface_ext) {
    another.m_surface = VK_NULL_HANDLE;
  }
  Surface const &operator=(Surface const &another) = delete;
  Surface &operator=(Surface &&another) noexcept {
    m_parent = another.m_parent;
    m_surface_ext = another.m_surface_ext;
    std::swap(m_surface, another.m_surface);
    return *this;
  };

  Instance const &getParent() const { return m_parent; };

  auto const &ext() const { return m_surface_ext; }

  std::vector<VkPresentModeKHR>
  getAvailablePresentModes(VkPhysicalDevice device) const;
  std::vector<VkSurfaceFormatKHR>
  getAvailableFormats(VkPhysicalDevice device) const;
  VkSurfaceCapabilitiesKHR
  getSurfaceCapabilities(VkPhysicalDevice device) const;
  std::vector<uint32_t>
  getQueueFamilyIndicesThatSupportPresenting(VkPhysicalDevice device) const;

  operator VkSurfaceKHR() const { return m_surface; }

  virtual ~Surface();

private:
  VkSurfaceKHR m_surface;
  Extension<ext::KHR_surface> m_surface_ext;
  InstanceCRef m_parent;
};
} // namespace vkw

#undef VK_CHECK_RESULT_
#endif // VKRENDERER_SURFACE_HPP
