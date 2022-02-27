#include "Sampler.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

Sampler::Sampler(Device const &device, VkSamplerCreateInfo createInfo)
    : m_device(device),
      m_createInfo(createInfo){VK_CHECK_RESULT(
          vkCreateSampler(m_device.get(), &m_createInfo, nullptr, &m_sampler))}

      Sampler::~Sampler() {
  if (m_sampler == VK_NULL_HANDLE)
    return;

  vkDestroySampler(m_device.get(), m_sampler, nullptr);
}
} // namespace vkw