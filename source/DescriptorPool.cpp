#include "DescriptorPool.hpp"
#include "DescriptorSet.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

DescriptorPool::DescriptorPool(Device const &device, uint32_t maxSets,
                               std::vector<VkDescriptorPoolSize> poolSizes,
                               VkDescriptorPoolCreateFlags flags)
    : m_device(device), m_poolSizes(std::move(poolSizes)) {
  m_createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.maxSets = maxSets;
  m_createInfo.poolSizeCount = m_poolSizes.size();
  m_createInfo.pPoolSizes = m_poolSizes.data();

  VK_CHECK_RESULT(vkCreateDescriptorPool(m_device.get(), &m_createInfo, nullptr,
                                         &m_descriptorPool))
}

DescriptorPool::~DescriptorPool() {
  if (m_descriptorPool == VK_NULL_HANDLE)
    return;

  vkDestroyDescriptorPool(m_device.get(), m_descriptorPool, nullptr);
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
  VK_CHECK_RESULT(vkAllocateDescriptorSets(m_device.get(), &allocateInfo, &set))

  return set;
}
void DescriptorPool::freeSet(const DescriptorSet &set) {
  if (!(m_createInfo.flags & VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT))
    return;
  VkDescriptorSet vSet = set;

  // This function is meant to be called inside the DescriptorSet destructor.
  // That means we are not allowed to pass any exceptions out and handle them
  // here.
  try {
    VK_CHECK_RESULT(
        vkFreeDescriptorSets(m_device.get(), m_descriptorPool, 1, &vSet))
  } catch (VulkanError &e) {
    // do something
  }
}

} // namespace vkw