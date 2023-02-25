#include "vkw/DescriptorPool.hpp"
#include "Utils.hpp"
#include "vkw/DescriptorSet.hpp"
#include "vkw/Device.hpp"

namespace vkw {

DescriptorPool::DescriptorPool(Device const &device, uint32_t maxSets,
                               std::span<const VkDescriptorPoolSize> poolSizes,
                               VkDescriptorPoolCreateFlags flags)
    : m_device(device), m_maxSets(maxSets) {
  std::copy(poolSizes.begin(), poolSizes.end(),
            std::back_inserter(m_poolSizes));
  m_createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.maxSets = maxSets;
  m_createInfo.poolSizeCount = m_poolSizes.size();
  m_createInfo.pPoolSizes = m_poolSizes.data();

  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateDescriptorPool(
      m_device.get(), &m_createInfo, nullptr, &m_descriptorPool))
}

DescriptorPool::~DescriptorPool() {
  if (m_descriptorPool == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroyDescriptorPool(
      m_device.get(), m_descriptorPool, nullptr);
}
VkDescriptorSet DescriptorPool::allocateSet(const DescriptorSetLayout &layout) {
  VkDescriptorSetAllocateInfo allocateInfo{};
  allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocateInfo.pNext = nullptr;
  allocateInfo.descriptorPool = m_descriptorPool;
  allocateInfo.descriptorSetCount = 1;
  VkDescriptorSetLayout v_layout = layout;
  allocateInfo.pSetLayouts = &v_layout;

  VkDescriptorSet set;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkAllocateDescriptorSets(
      m_device.get(), &allocateInfo, &set))

  m_setCount++;

  return set;
}
void DescriptorPool::freeSet(const DescriptorSet &set) {
  if (!(m_createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT))
    return;
  VkDescriptorSet vSet = set;

  // This function is meant to be called inside the DescriptorSet destructor.
  // That means it is not allowed to pass any exceptions out.
  // That's why irrecoverableError() is issued instead.
  try {
    VK_CHECK_RESULT(m_device.get().core<1, 0>().vkFreeDescriptorSets(
        m_device.get(), m_descriptorPool, 1, &vSet))
  } catch (VulkanError &e) {
    irrecoverableError(e);
  }
  m_setCount--;
}

} // namespace vkw