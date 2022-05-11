#ifndef VKRENDERER_SURFACE_HPP
#define VKRENDERER_SURFACE_HPP

#include "vkw/Instance.hpp"
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Instance;
class VkKhrSurface;

#define VK_CHECK_RESULT_(...)                                                  \
  {                                                                            \
    VkResult res = (__VA_ARGS__);                                              \
    if (res != VK_SUCCESS)                                                     \
      throw VulkanError(res, __FILE__, __LINE__);                              \
  }

class Surface final {
public:
#ifdef _WIN32
  Surface(Instance &parent, HINSTANCE hinstance, HWND hwnd) : m_parent(parent) {
    m_surface_ext = static_cast<VkKhrSurface const *>(
        parent.extension(VK_KHR_SURFACE_EXTENSION_NAME));
    if (!m_surface_ext)
      throw ExtensionMissing(VK_KHR_SURFACE_EXTENSION_NAME);
    auto *win32SurfaceExt = static_cast<VkKhrWin32Surface const *>(
        parent.extension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME));
    if (!win32SurfaceExt)
      throw ExtensionMissing(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

    VkWin32SurfaceCreateInfoKHR createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.hinstance = hinstance;
    createInfo.hwnd = hwnd;

    VK_CHECK_RESULT_(win32SurfaceExt->vkCreateWin32SurfaceKHR(
        m_parent.get(), &createInfo, nullptr, &m_surface))
  }
#elif defined __linux__
#ifdef VK_USE_PLATFORM_XLIB_KHR
  Surface(Instance &parent, Display *display, Window window)
      : m_parent(parent) {
    m_surface_ext = static_cast<VkKhrSurface const *>(
        parent.extension(VK_KHR_SURFACE_EXTENSION_NAME));
    if (!m_surface_ext)
      throw ExtensionMissing(VK_KHR_SURFACE_EXTENSION_NAME);
    auto *xlibSurfaceExt = static_cast<VkKhrXlibSurface const *>(
        parent.extension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME));
    if (!xlibSurfaceExt)
      throw ExtensionMissing(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);

    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.dpy = display;
    createInfo.window = window;

    VK_CHECK_RESULT_(xlibSurfaceExt->vkCreateXlibSurfaceKHR(
        m_parent.get(), &createInfo, nullptr, &m_surface))
  };
#elif defined VK_USE_PLATFORM_XCB_KHR
  Surface(Instance &parent, xcb_connection_t *connection, xcb_window_t window)
      : m_parent(parent) {
    m_surface_ext = static_cast<VkKhrSurface const *>(
        parent.extension(VK_KHR_SURFACE_EXTENSION_NAME));
    if (!m_surface_ext)
      throw ExtensionMissing(VK_KHR_SURFACE_EXTENSION_NAME);
    auto *xcbSurfaceExt = static_cast<VkKhrXcbSurface const *>(
        parent.extension(VK_KHR_XCB_SURFACE_EXTENSION_NAME));
    if (!xlibSurfaceExt)
      throw ExtensionMissing(VK_KHR_XCB_SURFACE_EXTENSION_NAME);

    VkXcbSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.connection = connection;
    createInfo.window = window;

    VK_CHECK_RESULT_(xcbSurfaceExt->vkCreateXCBSurfaceKHR(m_parent, &createInfo,
                                                          nullptr, &m_surface))
  };
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
  Surface(Instance &parent, wl_display *display, wl_surface *surface)
      : m_parent(parent) {
    m_surface_ext = static_cast<VkKhrSurface const *>(
        parent.extension(VK_KHR_SURFACE_EXTENSION_NAME));
    if (!m_surface_ext)
      throw ExtensionMissing(VK_KHR_SURFACE_EXTENSION_NAME);
    auto *waylandSurfaceExt = static_cast<VkKhrWaylandSurface const *>(
        parent.extension(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME));
    if (!waylandSurfaceExt)
      throw ExtensionMissing(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);

    VkWaylandSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = nullptr;
    createInfo.display = display;
    createInfo.surface = surface;

    VK_CHECK_RESULT_(waylandSurfaceExt->vkCreateWaylandSurfaceKHR(
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

  Instance &getParent() const { return m_parent; };

  std::vector<VkPresentModeKHR>
  getAvailablePresentModes(VkPhysicalDevice device) const;
  std::vector<VkSurfaceFormatKHR>
  getAvailableFormats(VkPhysicalDevice device) const;
  VkSurfaceCapabilitiesKHR
  getSurfaceCapabilities(VkPhysicalDevice device) const;
  std::vector<uint32_t>
  getQueueFamilyIndicesThatSupportPresenting(VkPhysicalDevice device) const;

  operator VkSurfaceKHR() const { return m_surface; }

  ~Surface();

private:
  VkSurfaceKHR m_surface;
  VkKhrSurface const *m_surface_ext{};
  InstanceRef m_parent;
};
} // namespace vkw

#undef VK_CHECK_RESULT_
#endif // VKRENDERER_SURFACE_HPP
