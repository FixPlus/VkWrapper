#ifndef VKWRAPPER_SAMPLER_HPP
#define VKWRAPPER_SAMPLER_HPP

#include <vkw/Device.hpp>

namespace vkw {

class Sampler : public UniqueVulkanObject<VkSampler> {
public:
  Sampler(Device const &device,
          VkSamplerCreateInfo createInfo) noexcept(ExceptionsDisabled);

  auto &info() const noexcept { return m_createInfo; }

private:
  VkSamplerCreateInfo m_createInfo{};
};
} // namespace vkw
#endif // VKWRAPPER_SAMPLER_HPP
