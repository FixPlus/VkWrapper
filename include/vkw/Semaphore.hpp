#ifndef VKRENDERER_SEMAPHORE_HPP
#define VKRENDERER_SEMAPHORE_HPP

#include "vkw/Device.hpp"

namespace vkw {

class Semaphore : public ReferenceGuard {
public:
  Semaphore(Device &device);
  Semaphore(Semaphore &&another)
      : m_device(another.m_device), m_semaphore(another.m_semaphore) {
    another.m_semaphore = VK_NULL_HANDLE;
  }
  Semaphore &operator=(Semaphore &&another) noexcept {
    m_device = another.m_device;
    std::swap(m_semaphore, another.m_semaphore);
    return *this;
  }

  virtual ~Semaphore();

  operator VkSemaphore() const { return m_semaphore; }

private:
  StrongReference<Device> m_device;
  VkSemaphore m_semaphore;
};

} // namespace vkw
#endif // VKRENDERER_SEMAPHORE_HPP
