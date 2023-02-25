#ifndef VKWRAPPER_SAMPLER_HPP
#define VKWRAPPER_SAMPLER_HPP

#include "vkw/Device.hpp"

namespace vkw {

class Sampler : public ReferenceGuard {
public:
  Sampler(Device const &device, VkSamplerCreateInfo createInfo);
  Sampler(Sampler const &another) = delete;
  Sampler(Sampler &&another) noexcept
      : m_device(another.m_device), m_createInfo(another.m_createInfo),
        m_sampler(another.m_sampler) {
    another.m_sampler = VK_NULL_HANDLE;
  }
  Sampler const &operator=(Sampler const &another) = delete;
  Sampler &operator=(Sampler &&another) noexcept {
    m_device = another.m_device;
    m_createInfo = another.m_createInfo;
    std::swap(m_sampler, another.m_sampler);
    return *this;
  }

  operator VkSampler() const { return m_sampler; }
  virtual ~Sampler();

private:
  StrongReference<Device const> m_device;
  VkSamplerCreateInfo m_createInfo{};
  VkSampler m_sampler{};
};
} // namespace vkw
#endif // VKWRAPPER_SAMPLER_HPP
