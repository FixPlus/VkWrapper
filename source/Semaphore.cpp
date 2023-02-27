#include "vkw/Semaphore.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

namespace {

VkSemaphoreCreateInfo fillCreateInfo() {
  VkSemaphoreCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  createInfo.pNext = nullptr;

  return createInfo;
}
} // namespace
Semaphore::Semaphore(Device const &device)
    : UniqueVulkanObject<VkSemaphore>(device, fillCreateInfo()) {}

} // namespace vkw