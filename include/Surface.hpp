#ifndef VKRENDERER_SURFACE_HPP
#define VKRENDERER_SURFACE_HPP

#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Instance;

class Surface final {
public:
  Surface(Surface const &another) = delete;
  Surface(Surface &&another) noexcept
      : m_parent(another.m_parent), m_surface(another.m_surface) {
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
  Surface(Instance &parent, VkSurfaceKHR surface)
      : m_parent(parent), m_surface(surface){};
  VkSurfaceKHR m_surface;
  Instance &m_parent;

  friend class Instance;
};
} // namespace vkr
#endif // VKRENDERER_SURFACE_HPP
