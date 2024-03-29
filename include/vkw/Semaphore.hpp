#ifndef VKRENDERER_SEMAPHORE_HPP
#define VKRENDERER_SEMAPHORE_HPP

#include <vkw/Device.hpp>

namespace vkw {

class Semaphore : public UniqueVulkanObject<VkSemaphore> {
public:
  Semaphore(Device const &device) noexcept(ExceptionsDisabled);
};

} // namespace vkw
#endif // VKRENDERER_SEMAPHORE_HPP
