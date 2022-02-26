#ifndef VKWRAPPER_DESCRIPTORSET_HPP
#define VKWRAPPER_DESCRIPTORSET_HPP

#include "Common.hpp"

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
  DescriptorSetLayout(Device const &device,
                      DescriptorSetLayoutBindingConstRefArray bindings,
                      VkDescriptorSetLayoutCreateFlags flags = 0);
  DescriptorSetLayout(DescriptorSetLayout const &another) = delete;
  DescriptorSetLayout(DescriptorSetLayout &&another) noexcept
      : m_device(another.m_device), m_bindings(std::move(another.m_bindings)),
        m_createInfo(another.m_createInfo), m_layout(another.m_layout) {
    another.m_layout = VK_NULL_HANDLE;
  };

  DescriptorSetLayout const &
  operator=(DescriptorSetLayout const &another) = delete;
  DescriptorSetLayout &operator=(DescriptorSetLayout &&another) noexcept {
    m_layout = another.m_layout;
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
  DeviceCRef m_device;
  std::vector<DescriptorSetLayoutBinding> m_bindings;
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
  struct M_DynamicOffset {
    uint32_t binding;
    uint32_t offset{0};
    M_DynamicOffset(uint32_t bind) : binding(bind){};
  };
  std::vector<M_DynamicOffset> m_dynamicOffsets{};

  friend class DescriptorPool;

  DescriptorPoolRef m_pool;
  DescriptorSetLayoutCRef m_layout;
  VkDescriptorSet m_set{};
};
} // namespace vkw
#endif // VKWRAPPER_DESCRIPTORSET_HPP
