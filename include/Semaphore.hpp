#ifndef VKRENDERER_SEMAPHORE_HPP
#define VKRENDERER_SEMAPHORE_HPP

#include "Common.hpp"
#include <vulkan/vulkan.h>
namespace vkw {

class Device;

class Semaphore {
public:
  Semaphore(Device &device);
  Semaphore(Semaphore &&another)
      : m_device(another.m_device), m_semaphore(another.m_semaphore) {
    another.m_semaphore = VK_NULL_HANDLE;
  }

  virtual ~Semaphore();

  operator VkSemaphore() const { return m_semaphore; }

private:
  Device &m_device;
  VkSemaphore m_semaphore;
};

VKR_DECLARE_ARRAY_TYPES(Semaphore)

} // namespace vkr
#endif // VKRENDERER_SEMAPHORE_HPP
