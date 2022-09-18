#include "vkw/DescriptorSet.hpp"
#include "Utils.hpp"
#include "vkw/DescriptorPool.hpp"
#include "vkw/Device.hpp"
#include "vkw/Image.hpp"
#include "vkw/Sampler.hpp"

namespace vkw {

DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(
    uint32_t binding, VkDescriptorType type, VkShaderStageFlags shaderStages,
    uint32_t descriptorCount, VkSampler *pImmutableSamplers)
    : m_binding{.binding = binding,
                .descriptorType = type,
                .descriptorCount = descriptorCount,
                .stageFlags = shaderStages,
                .pImmutableSamplers = pImmutableSamplers} {}
bool DescriptorSetLayoutBinding::hasDynamicOffset() const {
  return m_binding.descriptorType ==
             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
         m_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
}

DescriptorSetLayout::DescriptorSetLayout(
    Device const &device, DescriptorSetLayoutBindingConstRefArray bindings,
    VkDescriptorSetLayoutCreateFlags flags)
    : m_device(device) {
  m_bindings.reserve(bindings.size());
  for (auto const &binding : bindings) {
    m_bindings.emplace_back(binding);
  }

  // sort bindings by binding index (this will be useful)
  std::sort(m_bindings.begin(), m_bindings.end(),
            [](DescriptorSetLayoutBinding const &lhs,
               DescriptorSetLayoutBinding const &rhs) {
              return lhs.binding() > rhs.binding();
            });

  // validate bindings

  // 1. check for binding duplicates
  if (bindings.size() > 1) {
    auto predIt = m_bindings.begin();
    if (std::any_of(m_bindings.begin() + 1, m_bindings.end(),
                    [&predIt](DescriptorSetLayoutBinding const &elem) {
                      return elem.binding() == (predIt++)->binding();
                    })) {
      throw Error("DescriptorSetLayout create failed: bindings array has "
                  "duplicate binding index of '" +
                  std::to_string(predIt->binding()) + "'");
    }
  }

  m_createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.bindingCount = bindings.size();
  m_createInfo.pBindings = bindings;

  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateDescriptorSetLayout(
      m_device.get(), &m_createInfo, nullptr, &m_layout))
}
DescriptorSetLayout::~DescriptorSetLayout() {
  if (m_layout == VK_NULL_HANDLE)
    return;
  m_device.get().core<1, 0>().vkDestroyDescriptorSetLayout(m_device.get(),
                                                           m_layout, nullptr);
}

DescriptorSet::DescriptorSet(DescriptorPool &pool,
                             DescriptorSetLayout const &layout)
    : m_pool(pool), m_layout(layout), m_set(pool.allocateSet(layout)) {
  for (auto const &binding : layout) {
    if (binding.hasDynamicOffset())
      m_dynamicOffsets.emplace_back(binding.binding());
  }
}

DescriptorSet::~DescriptorSet() {
  if (m_set == VK_NULL_HANDLE)
    return;
  m_pool.get().freeSet(*this);
}
void DescriptorSet::m_write(uint32_t writeCount,
                            VkWriteDescriptorSet *pWrites) {
  for (uint32_t i = 0; i < writeCount; ++i)
    pWrites[i].dstSet = m_set;

  m_pool.get().device().core<1, 0>().vkUpdateDescriptorSets(
      m_pool.get().device(), writeCount, pWrites, 0, nullptr);
}
void DescriptorSet::setDynamicOffset(uint32_t binding, uint32_t offset) {
  auto found = std::find_if(m_dynamicOffsets.begin(), m_dynamicOffsets.end(),
                            [binding](M_DynamicOffset const &elem) {
                              return elem.binding == binding;
                            });

  if (found == m_dynamicOffsets.end())
    throw Error("Bad binding(" + std::to_string(binding) +
                ") was given to setDynamicOffset()");

  found->offset = offset;
}

void DescriptorSet::m_write_uniformBuffer(uint32_t binding,
                                          VkDescriptorBufferInfo bufferInfo) {
  VkWriteDescriptorSet writeSet{};
  writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSet.pNext = nullptr;
  writeSet.descriptorCount = 1;
  writeSet.dstSet = m_set;
  writeSet.dstArrayElement = 0;
  writeSet.pBufferInfo = &bufferInfo;
  writeSet.dstBinding = binding;
  writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  m_write(1, &writeSet);
}

void DescriptorSet::m_write_combined_image_sampler(
    uint32_t binding, VkDescriptorImageInfo imageInfo) {
  VkWriteDescriptorSet writeSet{};
  writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSet.pNext = nullptr;
  writeSet.descriptorCount = 1;
  writeSet.dstSet = m_set;
  writeSet.dstArrayElement = 0;
  writeSet.pImageInfo = &imageInfo;
  writeSet.dstBinding = binding;
  writeSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  m_write(1, &writeSet);
}
void DescriptorSet::writeStorageImage(uint32_t binding,
                                      const ImageViewBase &image,
                                      VkImageLayout layout) {
  VkDescriptorImageInfo info{};
  info.imageView = image;
  info.imageLayout = layout;
  m_write_storage_image(binding, info);
}
void DescriptorSet::m_write_storage_image(uint32_t binding,
                                          VkDescriptorImageInfo imageInfo) {
  VkWriteDescriptorSet writeSet{};
  writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSet.pNext = nullptr;
  writeSet.descriptorCount = 1;
  writeSet.dstSet = m_set;
  writeSet.dstArrayElement = 0;
  writeSet.pImageInfo = &imageInfo;
  writeSet.dstBinding = binding;
  writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
  m_write(1, &writeSet);
}
void DescriptorSet::m_write_storageBuffer(uint32_t binding,
                                          VkDescriptorBufferInfo bufferInfo) {
  VkWriteDescriptorSet writeSet{};
  writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSet.pNext = nullptr;
  writeSet.descriptorCount = 1;
  writeSet.dstSet = m_set;
  writeSet.dstArrayElement = 0;
  writeSet.pBufferInfo = &bufferInfo;
  writeSet.dstBinding = binding;
  writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  m_write(1, &writeSet);
}

} // namespace vkw