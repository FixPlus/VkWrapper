#include "vkw/Sampler.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

Sampler::Sampler(Device const &device,
                 VkSamplerCreateInfo createInfo) noexcept(ExceptionsDisabled)
    : UniqueVulkanObject<VkSampler>(device, createInfo),
      m_createInfo(createInfo) {}

} // namespace vkw