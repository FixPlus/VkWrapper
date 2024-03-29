#include "vkw/DescriptorSet.hpp"
#include "Utils.hpp"
#include "vkw/DescriptorPool.hpp"
#include "vkw/Device.hpp"
#include "vkw/Image.hpp"
#include "vkw/Sampler.hpp"

namespace vkw {

DescriptorSetLayoutBinding::DescriptorSetLayoutBinding(
    uint32_t binding, VkDescriptorType type, VkShaderStageFlags shaderStages,
    uint32_t descriptorCount, VkSampler *pImmutableSamplers) noexcept
    : m_binding{.binding = binding,
                .descriptorType = type,
                .descriptorCount = descriptorCount,
                .stageFlags = shaderStages,
                .pImmutableSamplers = pImmutableSamplers} {}
bool DescriptorSetLayoutBinding::hasDynamicOffset() const noexcept {
  return m_binding.descriptorType ==
             VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
         m_binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
}

void DescriptorSetLayoutInfo::m_fillInfo(
    VkDescriptorSetLayoutCreateFlags flags) noexcept(ExceptionsDisabled) {
  // sort bindings by binding index (this will be useful)
  std::sort(m_bindings.begin(), m_bindings.end(),
            [](DescriptorSetLayoutBinding const &lhs,
               DescriptorSetLayoutBinding const &rhs) {
              return lhs.binding() > rhs.binding();
            });

  // validate bindings

  // 1. check for binding duplicates
  if (m_bindings.size() > 1) {
    auto predIt = m_bindings.begin();
    if (std::any_of(m_bindings.begin() + 1, m_bindings.end(),
                    [&predIt](DescriptorSetLayoutBinding const &elem) {
                      return elem.binding() == (predIt++)->binding();
                    })) {
      postError(Error("DescriptorSetLayout create failed: bindings array has "
                      "duplicate binding index of '" +
                      std::to_string(predIt->binding()) + "'"));
    }
  }

  std::transform(m_bindings.begin(), m_bindings.end(),
                 std::back_inserter(m_rawBindings),
                 [](DescriptorSetLayoutBinding const &entry) { return entry; });

  m_createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.bindingCount = m_rawBindings.size();
  m_createInfo.pBindings = m_rawBindings.data();
}

DescriptorSet::DescriptorSet(
    DescriptorPool &pool,
    DescriptorSetLayout const &layout) noexcept(ExceptionsDisabled)
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
                            VkWriteDescriptorSet *pWrites) noexcept {
  for (uint32_t i = 0; i < writeCount; ++i)
    pWrites[i].dstSet = m_set;

  m_pool.get().parent().core<1, 0>().vkUpdateDescriptorSets(
      m_pool.get().parent(), writeCount, pWrites, 0, nullptr);
}
void DescriptorSet::setDynamicOffset(
    uint32_t binding, uint32_t offset) noexcept(ExceptionsDisabled) {
  auto found = std::find_if(m_dynamicOffsets.begin(), m_dynamicOffsets.end(),
                            [binding](M_DynamicOffset const &elem) {
                              return elem.binding == binding;
                            });

  if (found == m_dynamicOffsets.end())
    postError(Error("Bad binding(" + std::to_string(binding) +
                    ") was given to setDynamicOffset()"));

  found->offset = offset;
}

void DescriptorSet::m_write_uniformBuffer(
    uint32_t binding, VkDescriptorBufferInfo bufferInfo) noexcept {
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
    uint32_t binding, VkDescriptorImageInfo imageInfo) noexcept {
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
                                      VkImageLayout layout) noexcept {
  VkDescriptorImageInfo info{};
  info.imageView = image;
  info.imageLayout = layout;
  m_write_storage_image(binding, info);
}
void DescriptorSet::m_write_storage_image(
    uint32_t binding, VkDescriptorImageInfo imageInfo) noexcept {
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
void DescriptorSet::m_write_storageBuffer(
    uint32_t binding, VkDescriptorBufferInfo bufferInfo) noexcept {
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
void DescriptorSet::write(uint32_t binding, BufferBase const &buffer,
                          VkDescriptorType type, VkDeviceSize offset,
                          VkDeviceSize range) noexcept {
  VkWriteDescriptorSet writeSet{};
  VkDescriptorBufferInfo bufferInfo;
  bufferInfo.buffer = buffer;
  bufferInfo.offset = offset;
  bufferInfo.range = range;
  writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  writeSet.pNext = nullptr;
  writeSet.descriptorCount = 1;
  writeSet.dstSet = m_set;
  writeSet.dstArrayElement = 0;
  writeSet.pBufferInfo = &bufferInfo;
  writeSet.dstBinding = binding;
  writeSet.descriptorType = type;
  m_write(1, &writeSet);
}

} // namespace vkw