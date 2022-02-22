#ifndef VKRENDERER_SEMAPHORE_HPP
#define VKRENDERER_SEMAPHORE_HPP

#include "Common.hpp"
#include <vulkan/vulkan.h>
namespace vkw {

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

} // namespace vkw
#endif // VKRENDERER_SEMAPHORE_HPP
