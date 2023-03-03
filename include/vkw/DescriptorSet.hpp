#ifndef VKWRAPPER_DESCRIPTORSET_HPP
#define VKWRAPPER_DESCRIPTORSET_HPP

#include <vkw/DescriptorPool.hpp>
#include <vkw/Image.hpp>
#include <vkw/RangeConcepts.hpp>
#include <vkw/Sampler.hpp>
#include <vkw/UniformBuffer.hpp>

#include <algorithm>
#include <iterator>

namespace vkw {

class DescriptorSetLayoutBinding {
public:
  DescriptorSetLayoutBinding(
      uint32_t binding, VkDescriptorType type,
      VkShaderStageFlags shaderStages = VK_SHADER_STAGE_ALL,
      uint32_t descriptorCount = 1,
      VkSampler *pImmutableSamplers = nullptr) noexcept;
  virtual ~DescriptorSetLayoutBinding() = default;

  uint32_t binding() const noexcept { return m_binding.binding; }

  VkDescriptorType type() const noexcept { return m_binding.descriptorType; }

  bool hasDynamicOffset() const noexcept;

  uint32_t descriptorCount() const noexcept {
    return m_binding.descriptorCount;
  }

  operator VkDescriptorSetLayoutBinding const &() const noexcept {
    return m_binding;
  }

  bool operator==(DescriptorSetLayoutBinding const &rhs) const noexcept {
    return m_binding.binding == rhs.m_binding.binding &&
           m_binding.descriptorType == rhs.m_binding.descriptorType &&
           m_binding.descriptorCount == rhs.m_binding.descriptorCount &&
           m_binding.stageFlags == rhs.m_binding.stageFlags;
  }

  bool operator!=(DescriptorSetLayoutBinding const &rhs) const noexcept {
    return !(*this == rhs);
  }

private:
  VkDescriptorSetLayoutBinding m_binding;
};

class DescriptorSetLayoutInfo {
public:
  template <forward_range_of<DescriptorSetLayoutBinding> T>
  explicit DescriptorSetLayoutInfo(
      T const &bindings,
      VkDescriptorSetLayoutCreateFlags flags = 0) noexcept(ExceptionsDisabled) {
    auto bindingsSubrange =
        ranges::make_subrange<DescriptorSetLayoutBinding>(bindings);
    using bindingsSubrangeT = decltype(bindingsSubrange);
    std::transform(bindingsSubrange.begin(), bindingsSubrange.end(),
                   std::back_inserter(m_bindings), [](auto const &entry) {
                     return bindingsSubrangeT::get(entry);
                   });
    m_fillInfo(flags);
  }

  auto begin() noexcept { return m_bindings.begin(); }

  auto end() noexcept { return m_bindings.end(); }

  auto begin() const noexcept { return m_bindings.begin(); }

  auto end() const noexcept { return m_bindings.end(); }

  bool operator==(DescriptorSetLayoutInfo const &rhs) const noexcept {
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

  bool operator!=(DescriptorSetLayoutInfo const &rhs) const noexcept {
    return !(*this == rhs);
  }

  VkDescriptorSetLayoutCreateFlags flags() const noexcept {
    return m_createInfo.flags;
  }

  auto &info() const noexcept { return m_createInfo; }

private:
  void m_fillInfo(VkDescriptorSetLayoutCreateFlags flags) noexcept(
      ExceptionsDisabled);
  boost::container::small_vector<DescriptorSetLayoutBinding, 3> m_bindings;
  boost::container::small_vector<VkDescriptorSetLayoutBinding, 5> m_rawBindings;
  VkDescriptorSetLayoutCreateInfo m_createInfo{};
};

class DescriptorSetLayout : public DescriptorSetLayoutInfo,
                            public UniqueVulkanObject<VkDescriptorSetLayout> {
public:
  template <forward_range_of<DescriptorSetLayoutBinding> T>
  DescriptorSetLayout(
      Device const &device, T const &bindings,
      VkDescriptorSetLayoutCreateFlags flags = 0) noexcept(ExceptionsDisabled)
      : DescriptorSetLayoutInfo(bindings, flags),
        UniqueVulkanObject<VkDescriptorSetLayout>(device, info()) {}

  bool operator==(DescriptorSetLayout const &rhs) const noexcept {
    return DescriptorSetLayoutInfo::operator==(rhs);
  }

  bool operator!=(DescriptorSetLayout const &rhs) const noexcept {
    return !(*this == rhs);
  }
};

class DescriptorSet : public ReferenceGuard {
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
             VkDeviceSize offset = 0,
             VkDeviceSize range = VK_WHOLE_SIZE) noexcept;

  template <typename T>
  void write(uint32_t binding, UniformBuffer<T> const &uniformBuffer) noexcept {
    write(binding, uniformBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
          sizeof(T));
  }

  template <typename T>
  void write(uint32_t binding, StorageBuffer<T> const &storageBuffer) noexcept {
    write(binding, storageBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 0,
          sizeof(T));
  }

  void write(uint32_t binding, ImageViewBase const &image, VkImageLayout layout,
             Sampler const &sampler) noexcept {
    m_write_combined_image_sampler(binding, {sampler, image, layout});
  }

  void
  writeStorageImage(uint32_t binding, ImageViewBase const &image,
                    VkImageLayout layout = VK_IMAGE_LAYOUT_GENERAL) noexcept;

  uint32_t dynamicOffsetsCount() const { return m_dynamicOffsets.size(); }

  void copyOffsets(uint32_t *pOffsets) const {
    uint32_t counter = 0;
    for (auto const &offset : m_dynamicOffsets)
      pOffsets[counter++] = offset.offset;
  }

  void setDynamicOffset(uint32_t binding,
                        uint32_t offset) noexcept(ExceptionsDisabled);

  DescriptorSetLayout const &layout() const { return m_layout; }

  operator VkDescriptorSet() const { return m_set; }

  virtual ~DescriptorSet();
  DescriptorSet(DescriptorPool &pool,
                DescriptorSetLayout const &layout) noexcept(ExceptionsDisabled);

protected:
  void m_write(uint32_t writeCount, VkWriteDescriptorSet *pWrites) noexcept;

private:
  void m_write_combined_image_sampler(uint32_t binding,
                                      VkDescriptorImageInfo imageInfo) noexcept;
  void m_write_storage_image(uint32_t binding,
                             VkDescriptorImageInfo imageInfo) noexcept;
  void m_write_uniformBuffer(uint32_t binding,
                             VkDescriptorBufferInfo bufferInfo) noexcept;
  void m_write_storageBuffer(uint32_t binding,
                             VkDescriptorBufferInfo bufferInfo) noexcept;
  struct M_DynamicOffset {
    uint32_t binding;
    uint32_t offset{0};
    M_DynamicOffset(uint32_t bind) noexcept : binding(bind){};
  };
  boost::container::small_vector<M_DynamicOffset, 2> m_dynamicOffsets{};

  friend class DescriptorPool;

  StrongReference<DescriptorPool> m_pool;
  StrongReference<DescriptorSetLayout const> m_layout;
  VkDescriptorSet m_set{};
};
} // namespace vkw
#endif // VKWRAPPER_DESCRIPTORSET_HPP
