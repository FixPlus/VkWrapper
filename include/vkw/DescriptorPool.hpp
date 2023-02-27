#ifndef VKWRAPPER_DESCRIPTORPOOL_HPP
#define VKWRAPPER_DESCRIPTORPOOL_HPP

#include "vkw/Device.hpp"
#include "vkw/UniqueVulkanObject.hpp"

#include <boost/container/small_vector.hpp>
#include <span>

namespace vkw {

class DescriptorSetLayout;
class DescriptorSet;

class DescriptorPoolInfo {
public:
  DescriptorPoolInfo(uint32_t maxSets,
                     std::span<const VkDescriptorPoolSize> poolSizes,
                     VkDescriptorPoolCreateFlags flags = 0);

  uint32_t maxSets() const { return m_createInfo.maxSets; }

  auto &info() const { return m_createInfo; }

private:
  boost::container::small_vector<VkDescriptorPoolSize, 3> m_poolSizes;
  VkDescriptorPoolCreateInfo m_createInfo{};
};

class DescriptorPool : public DescriptorPoolInfo,
                       public UniqueVulkanObject<VkDescriptorPool> {
public:
  DescriptorPool(Device const &device, uint32_t maxSets,
                 std::span<const VkDescriptorPoolSize> poolSizes,
                 VkDescriptorPoolCreateFlags flags = 0)
      : DescriptorPoolInfo(maxSets, poolSizes, flags),
        UniqueVulkanObject<VkDescriptorPool>(device, info()) {}

  uint32_t currentSetsCount() const { return m_setCount; }

private:
  VkDescriptorSet allocateSet(DescriptorSetLayout const &layout);

  void freeSet(DescriptorSet const &set);

  uint32_t m_setCount = 0;

  friend class DescriptorSet;
};

} // namespace vkw
#endif // VKWRAPPER_DESCRIPTORPOOL_HPP
