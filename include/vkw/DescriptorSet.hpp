#ifndef VKWRAPPER_DESCRIPTORSET_HPP
#define VKWRAPPER_DESCRIPTORSET_HPP

#include <vkw/Buffer.hpp>
#include <vkw/DescriptorPool.hpp>
#include <vkw/Image.hpp>
#include <vkw/RangeConcepts.hpp>
#include <vkw/Sampler.hpp>

#include <algorithm>
#include <ranges>

namespace vkw {

class DescriptorSetLayoutBinding final : public VkDescriptorSetLayoutBinding {
public:
  DescriptorSetLayoutBinding(
      uint32_t binding, VkDescriptorType type,
      VkShaderStageFlags shaderStages = VK_SHADER_STAGE_ALL,
      uint32_t descriptorCount = 1,
      VkSampler *pImmutableSamplers = nullptr) noexcept
      : VkDescriptorSetLayoutBinding{.binding = binding,
                                     .descriptorType = type,
                                     .descriptorCount = descriptorCount,
                                     .stageFlags = shaderStages,
                                     .pImmutableSamplers = pImmutableSamplers} {
  }

  bool hasDynamicOffset() const noexcept {
    return descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
           descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
  }

  bool operator==(DescriptorSetLayoutBinding const &rhs) const noexcept {
    return binding == rhs.binding && descriptorType == rhs.descriptorType &&
           descriptorCount == rhs.descriptorCount &&
           stageFlags == rhs.stageFlags;
  }

  bool operator!=(DescriptorSetLayoutBinding const &rhs) const noexcept {
    return !(*this == rhs);
  }
};

static_assert(sizeof(DescriptorSetLayoutBinding) ==
              sizeof(VkDescriptorSetLayoutBinding));

class DescriptorSetLayoutInfo {
public:
  template <forward_range_of<DescriptorSetLayoutBinding const> BindTs>
  explicit DescriptorSetLayoutInfo(
      BindTs &&bindings,
      VkDescriptorSetLayoutCreateFlags flags = 0) noexcept(ExceptionsDisabled)
      : m_flags(flags) {
    std::ranges::copy(bindings, std::back_inserter(m_bindings));
    std::ranges::sort(m_bindings, [](auto &&lhs, auto &&rhs) {
      return lhs.binding > rhs.binding;
    });
  }

  explicit DescriptorSetLayoutInfo(
      VkDescriptorSetLayoutCreateFlags flags = 0) noexcept(ExceptionsDisabled)
      : m_flags(flags) {}

  auto bindings() const noexcept {
    return std::ranges::subrange(m_bindings.begin(), m_bindings.end());
  }

  const DescriptorSetLayoutBinding &binding(unsigned binding) const {
    auto found = std::ranges::find_if(
        m_bindings, [&](auto &bnd) { return binding == bnd.binding; });
    /// TODO: postError ?
    assert(found != m_bindings.end());
    return *found;
  }

  bool operator==(DescriptorSetLayoutInfo const &rhs) const noexcept {
    if (m_bindings.size() != rhs.m_bindings.size() || m_flags != rhs.m_flags)
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

  VkDescriptorSetLayoutCreateFlags flags() const noexcept { return m_flags; }

  [[nodiscard]] VkDescriptorSetLayoutCreateInfo info() const &noexcept {
    VkDescriptorSetLayoutCreateInfo ret{};
    ret.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    ret.pNext = nullptr;
    ret.bindingCount = m_bindings.size();
    ret.pBindings = m_bindings.data();
    ret.flags = m_flags;
    return ret;
  }
  virtual ~DescriptorSetLayoutInfo() = default;

private:
  cntr::vector<DescriptorSetLayoutBinding, 3> m_bindings;
  VkDescriptorSetLayoutCreateFlags m_flags;
};

class DescriptorSetLayout : public DescriptorSetLayoutInfo,
                            public vk::DescriptorSetLayout {
public:
  template <typename BindingRange>
  DescriptorSetLayout(
      Device const &device, BindingRange &&bindings,
      VkDescriptorSetLayoutCreateFlags flags = 0) noexcept(ExceptionsDisabled)
      : DescriptorSetLayoutInfo(std::forward<BindingRange>(bindings), flags),
        vk::DescriptorSetLayout(device, info()) {}

  DescriptorSetLayout(
      Device const &device,
      VkDescriptorSetLayoutCreateFlags flags = 0) noexcept(ExceptionsDisabled)
      : DescriptorSetLayoutInfo(flags), vk::DescriptorSetLayout(device,
                                                                info()) {}

  bool operator==(DescriptorSetLayout const &rhs) const noexcept {
    return DescriptorSetLayoutInfo::operator==(rhs);
  }

  bool operator!=(DescriptorSetLayout const &rhs) const noexcept {
    return !(*this == rhs);
  }
};

class DescriptorWrite : private VkWriteDescriptorSet {
public:
  DescriptorWrite(unsigned binding, VkDescriptorType type) {
    this->dstBinding = binding;
    sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    pNext = nullptr;
    descriptorType = type;
    descriptorCount = 0;
    dstArrayElement = 0;
    pTexelBufferView = nullptr;
  }

  DescriptorWrite &addImage(VkSampler sampler, VkImageView view,
                            VkImageLayout layout) & {
    m_images.emplace_back(VkDescriptorImageInfo{sampler, view, layout});
    pImageInfo = m_images.data();
    ++descriptorCount;
    return *this;
  }
  DescriptorWrite &addBuffer(VkBuffer buffer, VkDeviceSize offset,
                             VkDeviceSize size) & {
    m_buffers.emplace_back(VkDescriptorBufferInfo{buffer, offset, size});
    pBufferInfo = m_buffers.data();
    ++descriptorCount;
    return *this;
  }
  DescriptorWrite &&addImage(VkSampler sampler, VkImageView view,
                             VkImageLayout layout) && {
    m_images.emplace_back(VkDescriptorImageInfo{sampler, view, layout});
    pImageInfo = m_images.data();
    ++descriptorCount;
    return std::move(*this);
  }
  DescriptorWrite &&addBuffer(VkBuffer buffer, VkDeviceSize offset,
                              VkDeviceSize size) && {
    m_buffers.emplace_back(VkDescriptorBufferInfo{buffer, offset, size});
    pBufferInfo = m_buffers.data();
    ++descriptorCount;
    return std::move(*this);
  }
  VkWriteDescriptorSet get() const & {
    VkWriteDescriptorSet copy =
        static_cast<const VkWriteDescriptorSet &>(*this);
    copy.pBufferInfo = m_buffers.data();
    copy.pImageInfo = m_images.data();
    return copy;
  }

private:
  cntr::vector<VkDescriptorImageInfo, 2> m_images;
  cntr::vector<VkDescriptorBufferInfo, 2> m_buffers;
};

class DescriptorSet : public ReferenceGuard {
public:
  DescriptorSet(DescriptorPool &pool,
                DescriptorSetLayout const &layout) noexcept(ExceptionsDisabled)
      : m_set(pool.allocateSet(layout), pool) {
    for (auto const &binding : layout.bindings()) {
      if (binding.hasDynamicOffset())
        m_dynamicOffsets.emplace_back(binding.binding);
    }
  }

  struct DynamicOffset {
    uint32_t binding;
    uint32_t offset{0};
    DynamicOffset(uint32_t bind) noexcept : binding(bind){};
  };

  void write(const DescriptorWrite &write) noexcept {
    VkWriteDescriptorSet copy = write.get();
    m_write(1, &copy);
  }

  void write(std::span<const DescriptorWrite> writes) noexcept {
    cntr::vector<VkWriteDescriptorSet, 4> rawWrites;
    std::ranges::transform(writes, std::back_inserter(rawWrites),
                           [](auto &&w) { return w.get(); });
    m_write(rawWrites.size(), rawWrites.data());
  }

  auto dynamicOffsets() const noexcept(ExceptionsDisabled) {
    return std::ranges::subrange(m_dynamicOffsets.begin(),
                                 m_dynamicOffsets.end());
  }

  void setDynamicOffset(uint32_t binding,
                        uint32_t offset) noexcept(ExceptionsDisabled) {
    auto found = std::find_if(m_dynamicOffsets.begin(), m_dynamicOffsets.end(),
                              [binding](DynamicOffset const &elem) {
                                return elem.binding == binding;
                              });

    /// TODO: postError() ?
    assert(found != m_dynamicOffsets.end() && "no dyn offset at binding");

    found->offset = offset;
  }

  operator VkDescriptorSet() const { return m_set.get(); }

protected:
  void m_write(uint32_t writeCount, VkWriteDescriptorSet *pWrites) noexcept {
    for (uint32_t i = 0; i < writeCount; ++i)
      pWrites[i].dstSet = m_set.get();
    auto &device = m_set.get_deleter().pool.get().parent();
    device.core<1, 0>().vkUpdateDescriptorSets(device, writeCount, pWrites, 0,
                                               nullptr);
  }

private:
  cntr::vector<DynamicOffset, 2> m_dynamicOffsets{};

  struct SetDestructor {
    SetDestructor(DescriptorPool &pool) : pool(pool){};
    void operator()(VkDescriptorSet set) const {
      if (set)
        pool.get().freeSet(set);
    };
    StrongReference<DescriptorPool> pool;
  };
  std::unique_ptr<VkDescriptorSet_T, SetDestructor> m_set;
};
} // namespace vkw
#endif // VKWRAPPER_DESCRIPTORSET_HPP
