#ifndef VKRENDERER_SURFACE_HPP
#define VKRENDERER_SURFACE_HPP

#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Instance;
class VkKhrSurface;

class Surface final {
public:
#ifdef _WIN32
  Surface(Instance &parent, HINSTANCE hinstance, HWND hwnd);
#endif
  Surface(Surface const &another) = delete;
  Surface(Surface &&another) noexcept
      : m_parent(another.m_parent), m_surface(another.m_surface),
        m_surface_ext(another.m_surface_ext) {
    another.m_surface = VK_NULL_HANDLE;
  }
  Surface const &operator=(Surface const &another) = delete;
  Surface &operator=(Surface &&another) = delete;

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
  Instance &m_parent;
};
} // namespace vkw
#endif // VKRENDERER_SURFACE_HPP
