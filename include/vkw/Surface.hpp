#ifndef VKRENDERER_SURFACE_HPP
#define VKRENDERER_SURFACE_HPP

#include <vkw/Extensions.hpp>
#include <vkw/Instance.hpp>

namespace vkw {

class Instance;

class Surface : public ReferenceGuard {
public:
  Surface(Instance const &parent,
          VkSurfaceKHR surface) noexcept(ExceptionsDisabled)
      : m_parent(parent), m_surface_ext(parent), m_surface(surface) {}
#ifdef VK_USE_PLATFORM_WIN32_KHR
  Surface(Instance const &parent, HINSTANCE hinstance,
          HWND hwnd) noexcept(ExceptionsDisabled);
#elif VK_USE_PLATFORM_XLIB_KHR
  Surface(Instance const &parent, Display *display,
          Window window) noexcept(ExceptionsDisabled);
#elif defined VK_USE_PLATFORM_XCB_KHR
  Surface(Instance const &parent, xcb_connection_t *connection,
          xcb_window_t window) noexcept(ExceptionsDisabled);
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
  Surface(Instance const &parent, wl_display *display,
          wl_surface *surface) noexcept(ExceptionsDisabled);
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

  Instance const &getParent() const noexcept { return m_parent; };

  auto const &ext() const noexcept { return m_surface_ext; }

  std::vector<VkPresentModeKHR>
  getAvailablePresentModes(VkPhysicalDevice device) const
      noexcept(ExceptionsDisabled);
  std::vector<VkSurfaceFormatKHR>
  getAvailableFormats(VkPhysicalDevice device) const
      noexcept(ExceptionsDisabled);
  VkSurfaceCapabilitiesKHR getSurfaceCapabilities(VkPhysicalDevice device) const
      noexcept(ExceptionsDisabled);
  std::vector<uint32_t>
  getQueueFamilyIndicesThatSupportPresenting(VkPhysicalDevice device) const
      noexcept(ExceptionsDisabled);

  operator VkSurfaceKHR() const noexcept { return m_surface; }

  virtual ~Surface();

private:
  VkSurfaceKHR m_surface;
  Extension<ext::KHR_surface> m_surface_ext;
  StrongReference<Instance const> m_parent;
};
} // namespace vkw

#endif // VKRENDERER_SURFACE_HPP
