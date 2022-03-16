#ifndef VKWRAPPER_DESCRIPTORPOOL_HPP
#define VKWRAPPER_DESCRIPTORPOOL_HPP

#include "Common.hpp"
#include "DescriptorSet.hpp"

namespace vkw {

class DescriptorPool {
public:
  DescriptorPool(Device const &device, uint32_t maxSets,
                 std::vector<VkDescriptorPoolSize> poolSizes,
                 VkDescriptorPoolCreateFlags flags = 0);
  DescriptorPool(DescriptorPool const &another) = delete;
  DescriptorPool(DescriptorPool &&another) noexcept
      : m_poolSizes(std::move(another.m_poolSizes)),
        m_descriptorPool(another.m_descriptorPool), m_device(another.m_device),
        m_createInfo(another.m_createInfo), m_maxSets(another.m_maxSets),
        m_setCount(another.m_setCount) {
    another.m_descriptorPool = VK_NULL_HANDLE;
  }
  DescriptorPool const &operator=(DescriptorPool const &another) = delete;
  DescriptorPool &operator=(DescriptorPool &&another) noexcept {
    m_device = another.m_device;
    m_createInfo = another.m_createInfo;
    m_poolSizes = std::move(another.m_poolSizes);
    m_maxSets = another.m_maxSets;
    m_setCount = another.m_setCount;
    std::swap(another.m_descriptorPool, m_descriptorPool);
    return *this;
  }

  virtual ~DescriptorPool();

  operator VkDescriptorPool() const { return m_descriptorPool; }

  Device const &device() const { return m_device; }

  uint32_t maxSets() const { return m_maxSets; }

  uint32_t currentSetsCount() const { return m_setCount; }

private:
  VkDescriptorSet allocateSet(DescriptorSetLayout const &layout);

  void freeSet(DescriptorSet const &set);

  uint32_t m_maxSets;
  uint32_t m_setCount = 0;

  friend class DescriptorSet;
  DeviceCRef m_device;
  std::vector<VkDescriptorPoolSize> m_poolSizes;
  VkDescriptorPoolCreateInfo m_createInfo{};
  VkDescriptorPool m_descriptorPool{};
};

} // namespace vkw
#endif // VKWRAPPER_DESCRIPTORPOOL_HPP
