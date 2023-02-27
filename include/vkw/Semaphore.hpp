#ifndef VKRENDERER_SEMAPHORE_HPP
#define VKRENDERER_SEMAPHORE_HPP

#include "vkw/Device.hpp"
#include "vkw/UniqueVulkanObject.hpp"

namespace vkw {

class Semaphore : public UniqueVulkanObject<VkSemaphore> {
public:
  Semaphore(Device const &device);
};

} // namespace vkw
#endif // VKRENDERER_SEMAPHORE_HPP
