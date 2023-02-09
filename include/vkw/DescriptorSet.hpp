#ifndef VKWRAPPER_DESCRIPTORSET_HPP
#define VKWRAPPER_DESCRIPTORSET_HPP

#include "Image.hpp"
#include "Sampler.hpp"
#include "UniformBuffer.hpp"
#include "vkw/Common.hpp"
#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <iterator>

namespace vkw {

class DescriptorSetLayoutBinding {
public:
  DescriptorSetLayoutBinding(
      uint32_t binding, VkDescriptorType type,
      VkShaderStageFlags shaderStages = VK_SHADER_STAGE_ALL,
      uint32_t descriptorCount = 1, VkSampler *pImmutableSamplers = nullptr);
  virtual ~DescriptorSetLayoutBinding() = default;

  uint32_t binding() const { return m_binding.binding; }

  VkDescriptorType type() const { return m_binding.descriptorType; }

  bool hasDynamicOffset() const;

  uint32_t descriptorCount() const { return m_binding.descriptorCount; }

  operator VkDescriptorSetLayoutBinding const &() const { return m_binding; }

  bool operator==(DescriptorSetLayoutBinding const &rhs) const {
    return m_binding.binding == rhs.m_binding.binding &&
           m_binding.descriptorType == rhs.m_binding.descriptorType &&
           m_binding.descriptorCount == rhs.m_binding.descriptorCount &&
           m_binding.stageFlags == rhs.m_binding.stageFlags;
  }

  bool operator!=(DescriptorSetLayoutBinding const &rhs) const {
    return !(*this == rhs);
  }

private:
  VkDescriptorSetLayoutBinding m_binding;
};

class DescriptorSetLayout {
public:
  template <forward_range_of<DescriptorSetLayoutBinding> T>
  DescriptorSetLayout(Device const &device, T const &bindings,
                      VkDescriptorSetLayoutCreateFlags flags = 0)
      : m_device(device) {
    auto bindingsSubrange =
        ranges::make_subrange<DescriptorSetLayoutBinding>(bindings);
    using bindingsSubrangeT = decltype(bindingsSubrange);
    std::transform(bindingsSubrange.begin(), bindingsSubrange.end(),
                   std::back_inserter(m_bindings), [](auto const &entry) {
                     return bindingsSubrangeT::get(entry);
                   });
    m_init(flags);
  }
  DescriptorSetLayout(DescriptorSetLayout const &another) = delete;
  DescriptorSetLayout(DescriptorSetLayout &&another) noexcept
      : m_device(another.m_device), m_bindings(std::move(another.m_bindings)),
        m_createInfo(another.m_createInfo), m_layout(another.m_layout) {
    another.m_layout = VK_NULL_HANDLE;
  };

  DescriptorSetLayout const &
  operator=(DescriptorSetLayout const &another) = delete;
  DescriptorSetLayout &operator=(DescriptorSetLayout &&another) noexcept {
    m_createInfo = another.m_createInfo;
    m_device = another.m_device;
    m_bindings = std::move(another.m_bindings);
    std::swap(m_layout, another.m_layout);
    return *this;
  }

  bool operator==(DescriptorSetLayout const &rhs) const {
    if (m_bindings.size() != rhs.m_bindings.size() ||
        m_createInfo.flags != rhs.m_createInfo.flags)
      return false;

    auto rhsBindingIter = rhs.m_bindings.begin();
    return std::all_of(
        m_bindings.begin(), m_bindings.end(),
        [&rhsBindingIter](DescriptorSetLayoutBinding const &binding) {
          return binding == *(rhsBindingIter++);
        });
  }

  bool operator!=(DescriptorSetLayout const &rhs) const {
    return !(*this == rhs);
  }

  virtual ~DescriptorSetLayout();

  auto begin() { return m_bindings.begin(); }

  auto end() { return m_bindings.end(); }

  auto begin() const { return m_bindings.begin(); }

  auto end() const { return m_bindings.end(); }

  VkDescriptorSetLayoutCreateFlags flags() const { return m_createInfo.flags; }

  operator VkDescriptorSetLayout() const { return m_layout; }

private:
  void m_init(VkDescriptorSetLayoutCreateFlags flags);
  DeviceCRef m_device;
  boost::container::small_vector<DescriptorSetLayoutBinding, 3> m_bindings;
  VkDescriptorSetLayoutCreateInfo m_createInfo{};
  VkDescriptorSetLayout m_layout{};
};

class DescriptorSet {
public:
  DescriptorSet(DescriptorSet const &another) = delete;
  DescriptorSet(DescriptorSet &&another) noexcept
      : m_pool(another.m_pool), m_layout(another.m_layout),
        m_set(another.m_set) {
    another.m_set = VK_NULL_HANDLE;
  }

  DescriptorSet const &operator=(DescriptorSet const &another) = delete;
  DescriptorSet &operator=(DescriptorSet &&another) noexcept {
    m_set = another.m_set;
    m_layout = another.m_layout;
    m_pool = another.m_pool;
    std::swap(m_set, another.m_set);
    return *this;
  };

  void write(uint32_t binding, BufferBase const &buffer, VkDescriptorType type,
             VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE);

  template <typename T>
  void write(uint32_t binding, UniformBuffer<T> const &uniformBuffer) {
    write(binding, uniformBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
          sizeof(T));
  }

  template <typename T>
  void write(uint32_t binding, StorageBuffer<T> const &storageBuffer) {
    write(binding, storageBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0,
          sizeof(T));
  }

  void write(uint32_t binding, ImageViewBase const &image, VkImageLayout layout,
             Sampler const &sampler) {
    m_write_combined_image_sampler(binding, {sampler, image, layout});
  }

  void writeStorageImage(uint32_t binding, ImageViewBase const &image,
                         VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL);

  uint32_t dynamicOffsetsCount() const { return m_dynamicOffsets.size(); }

  void copyOffsets(uint32_t *pOffsets) const {
    uint32_t counter = 0;
    for (auto const &offset : m_dynamicOffsets)
      pOffsets[counter++] = offset.offset;
  }

  void setDynamicOffset(uint32_t binding, uint32_t offset);

  DescriptorSetLayout const &layout() const { return m_layout; }

  operator VkDescriptorSet() const { return m_set; }

  virtual ~DescriptorSet();
  DescriptorSet(DescriptorPool &pool, DescriptorSetLayout const &layout);

protected:
  void m_write(uint32_t writeCount, VkWriteDescriptorSet *pWrites);

private:
  void m_write_combined_image_sampler(uint32_t binding,
                                      VkDescriptorImageInfo imageInfo);
  void m_write_storage_image(uint32_t binding, VkDescriptorImageInfo imageInfo);
  void m_write_uniformBuffer(uint32_t binding,
                             VkDescriptorBufferInfo bufferInfo);
  void m_write_storageBuffer(uint32_t binding,
                             VkDescriptorBufferInfo bufferInfo);
  struct M_DynamicOffset {
    uint32_t binding;
    uint32_t offset{0};
    M_DynamicOffset(uint32_t bind) : binding(bind){};
  };
  boost::container::small_vector<M_DynamicOffset, 2> m_dynamicOffsets{};

  friend class DescriptorPool;

  DescriptorPoolRef m_pool;
  DescriptorSetLayoutCRef m_layout;
  VkDescriptorSet m_set{};
};
} // namespace vkw
#endif // VKWRAPPER_DESCRIPTORSET_HPP
