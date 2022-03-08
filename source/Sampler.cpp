#include "Sampler.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

Sampler::Sampler(Device const &device, VkSamplerCreateInfo createInfo)
    : m_device(device),
      m_createInfo(createInfo){
          VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateSampler(
              m_device.get(), &m_createInfo, nullptr, &m_sampler))}

      Sampler::~Sampler() {
  if (m_sampler == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroySampler(m_device.get(), m_sampler,
                                               nullptr);
}
} // namespace vkw