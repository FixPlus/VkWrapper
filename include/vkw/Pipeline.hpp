#ifndef VKRENDERER_PIPELINE_HPP
#define VKRENDERER_PIPELINE_HPP

#include "Common.hpp"
#include "VertexBuffer.hpp"
#include <functional>
#include <optional>
#include <vulkan/vulkan.h>

namespace vkw {

class SpecializationConstants {
public:
  SpecializationConstants();

  SpecializationConstants(SpecializationConstants &&another) noexcept
      : m_entries(std::move(another.m_entries)),
        m_data(std::move(another.m_data)), m_info(another.m_info) {
    m_info.pMapEntries = m_entries.data();
    m_info.pData = m_data.data();
  };
  SpecializationConstants(SpecializationConstants const &another)
      : m_entries(another.m_entries), m_data(another.m_data),
        m_info(another.m_info) {
    m_info.pMapEntries = m_entries.data();
    m_info.pData = m_data.data();
  };

  SpecializationConstants &
  operator=(SpecializationConstants &&another) noexcept {
    m_entries = std::move(another.m_entries);
    m_data = std::move(another.m_data);
    m_info = another.m_info;
    m_info.pMapEntries = m_entries.data();
    m_info.pData = m_data.data();
    return *this;
  }

  SpecializationConstants &operator=(SpecializationConstants const &another) {
    m_entries = another.m_entries;
    m_data = another.m_data;
    m_info = another.m_info;
    m_info.pMapEntries = m_entries.data();
    m_info.pData = m_data.data();
    return *this;
  }

  bool empty() const { return m_entries.empty(); }

  template <typename T> void addConstant(T const &constant, uint32_t id) {
    m_addConstant(&constant, sizeof(constant), id);
  }

  void clear() {
    m_entries.clear();
    m_data.clear();
    m_info.mapEntryCount = 0;
  }

  operator VkSpecializationInfo const &() const { return m_info; }

private:
  void m_addConstant(const void *constant, size_t size, uint32_t id);

  std::vector<VkSpecializationMapEntry> m_entries;
  std::vector<unsigned char> m_data;
  VkSpecializationInfo m_info{};
};

class PipelineLayout {
public:
  PipelineLayout(DeviceRef device, VkPipelineLayoutCreateFlags flags = 0);
  PipelineLayout(DeviceRef device,
                 DescriptorSetLayoutConstRefArray const &setLayouts,
                 std::vector<VkPushConstantRange> pushConstants = {},
                 VkPipelineLayoutCreateFlags flags = 0);

  PipelineLayout(PipelineLayout const &another) = delete;
  PipelineLayout const &operator=(PipelineLayout const &another) = delete;

  PipelineLayout(PipelineLayout &&another) noexcept
      : m_device(another.m_device), m_layout(another.m_layout),
        m_createInfo(another.m_createInfo),
        m_descriptorLayouts(std::move(another.m_descriptorLayouts)),
        m_pushConstants(std::move(another.m_pushConstants)) {
    another.m_layout = VK_NULL_HANDLE;
  }
  PipelineLayout &operator=(PipelineLayout &&another) noexcept {
    m_device = another.m_device;
    m_createInfo = another.m_createInfo;
    m_layout = another.m_layout;
    m_descriptorLayouts = std::move(another.m_descriptorLayouts);
    m_pushConstants = std::move(another.m_pushConstants);
    another.m_layout = VK_NULL_HANDLE;
    return *this;
  };

  bool operator==(PipelineLayout const &rhs) const;

  bool operator!=(PipelineLayout const &rhs) const { return !(*this == rhs); }

  auto begin() { return m_descriptorLayouts.begin(); }

  auto end() { return m_descriptorLayouts.begin(); }

  auto begin() const { return m_descriptorLayouts.begin(); }

  auto end() const { return m_descriptorLayouts.begin(); }

  operator VkPipelineLayout() const { return m_layout; }

  virtual ~PipelineLayout();

private:
  DeviceRef m_device;
  std::vector<DescriptorSetLayoutCRef> m_descriptorLayouts{};
  std::vector<VkPushConstantRange> m_pushConstants{};
  VkPipelineLayoutCreateInfo m_createInfo{};
  VkPipelineLayout m_layout{};
};

class VertexInputStateCreateInfoBase {
public:
  VertexInputStateCreateInfoBase(
      uint32_t attributeDescCount,
      VkVertexInputAttributeDescription const *pAttributeDesc,
      uint32_t bindingDescCount,
      VkVertexInputBindingDescription const *pBindingDesc,
      VkPipelineVertexInputStateCreateFlags flags = 0);

  operator VkPipelineVertexInputStateCreateInfo const &() const {
    return m_createInfo;
  }

  uint32_t totalAttributes() const {
    return m_createInfo.vertexAttributeDescriptionCount;
  }
  VkVertexInputAttributeDescription attribute(uint32_t index) const;

  uint32_t totalBindings() const {
    return m_createInfo.vertexBindingDescriptionCount;
  }
  VkVertexInputBindingDescription binding(uint32_t index) const;

  virtual ~VertexInputStateCreateInfoBase() = default;

private:
  VkPipelineVertexInputStateCreateInfo m_createInfo{};
};

struct VertexInputStateCreateInfoBaseHandle
    : public std::unique_ptr<VertexInputStateCreateInfoBase> {
  template <typename... Args>
  VertexInputStateCreateInfoBaseHandle(Args... args)
      : std::unique_ptr<VertexInputStateCreateInfoBase>{
            std::make_unique<VertexInputStateCreateInfoBase>(args...)} {}
  VertexInputStateCreateInfoBaseHandle(
      VertexInputStateCreateInfoBaseHandle const &another)
      : std::unique_ptr<VertexInputStateCreateInfoBase>{
            std::make_unique<VertexInputStateCreateInfoBase>(*another.get())} {}

  VertexInputStateCreateInfoBaseHandle(
      VertexInputStateCreateInfoBaseHandle &&another) noexcept
      : std::unique_ptr<VertexInputStateCreateInfoBase>{std::move(another)} {}

  VertexInputStateCreateInfoBaseHandle &
  operator=(VertexInputStateCreateInfoBaseHandle &&another) noexcept {
    std::unique_ptr<VertexInputStateCreateInfoBase>::operator=(
        std::move(another));
    return *this;
  }
};

template <typename T>
concept BindingPointDescriptionLike = requires(T desc) {
  T::binding;
  std::same_as<decltype(T::binding), uint32_t>;
  T::value;
  std::same_as<decltype(T::value), VkVertexInputBindingDescription>;
  typename T::Attributes;
}
&&AttributeArray<typename T::Attributes>;

template <AttributeArray T, uint32_t bindingT = 0> struct per_vertex {
  using Attributes = T;
  constexpr static const uint32_t binding = bindingT;
  constexpr static const VkVertexInputBindingDescription value = {
      .binding = binding,
      .stride = sizeof(T),
      .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
};

template <AttributeArray T, uint32_t bindingT = 0> struct per_instance {
  using Attributes = T;
  constexpr static const uint32_t binding = bindingT;
  constexpr static const VkVertexInputBindingDescription value = {
      .binding = binding,
      .stride = sizeof(T),
      .inputRate = VK_VERTEX_INPUT_RATE_INSTANCE};
};

class NullVertexInputState : public VertexInputStateCreateInfoBase {
public:
  NullVertexInputState()
      : VertexInputStateCreateInfoBase(0, nullptr, 0, nullptr, 0){};
};

template <BindingPointDescriptionLike... Bindings>
class VertexInputStateCreateInfo : public VertexInputStateCreateInfoBase {
private:
  template <BindingPointDescriptionLike First,
            BindingPointDescriptionLike Second,
            BindingPointDescriptionLike... Rest>
  constexpr static uint32_t m_total_attributes() {
    return First::Attributes::count() + m_total_attributes<Second, Rest...>();
  }

  template <BindingPointDescriptionLike First>
  constexpr static uint32_t m_total_attributes() {
    return First::Attributes::count();
  }

public:
  VertexInputStateCreateInfo(VkPipelineVertexInputStateCreateFlags flags = 0)
      : VertexInputStateCreateInfoBase(m_total_attributes<Bindings...>(),
                                       m_AttributeDescHolder,
                                       sizeof...(Bindings), m_bind_desc) {}

private:
  struct M_AttributeDescHolder {

    M_AttributeDescHolder() { m_init_array<Bindings...>(); }

    operator VkVertexInputAttributeDescription const *() const {
      return m_attr_desc;
    }

  private:
    template <BindingPointDescriptionLike M_binding,
              BindingPointDescriptionLike M_dummy,
              BindingPointDescriptionLike... M_bindings>
    void m_init_array(uint32_t initIndex = 0, uint32_t location = 0) {
      uint32_t offset = 0u;
      for (uint32_t i = initIndex;
           i < initIndex + M_binding::Attributes::count(); ++i) {
        auto attrType = M_binding::Attributes::getAttrType(i - initIndex);
        m_attr_desc[i].binding = M_binding::binding;
        m_attr_desc[i].location = location;
        m_attr_desc[i].offset = offset;
        m_attr_desc[i].format = format_of(attrType);
        location += locations_hold(attrType);
        offset += size_of(attrType);
      }
      m_init_array<M_dummy, M_bindings...>(
          initIndex + M_binding::Attributes::count(), location);
    }

    template <BindingPointDescriptionLike M_binding>
    void m_init_array(uint32_t initIndex = 0, uint32_t location = 0) {
      uint32_t offset = 0u;
      for (uint32_t i = initIndex;
           i < initIndex + M_binding::Attributes::count(); ++i) {
        auto attrType = M_binding::Attributes::getAttrType(i - initIndex);
        m_attr_desc[i].binding = M_binding::binding;
        m_attr_desc[i].location = location;
        m_attr_desc[i].offset = offset;
        m_attr_desc[i].format = format_of(attrType);
        location += locations_hold(attrType);
        offset += size_of(attrType);
      }
    }
    VkVertexInputAttributeDescription
        m_attr_desc[m_total_attributes<Bindings...>()]{};
  };

  static const M_AttributeDescHolder m_AttributeDescHolder;

  constexpr static const VkVertexInputBindingDescription
      m_bind_desc[sizeof...(Bindings)] = {Bindings::value...};
};

template <BindingPointDescriptionLike... Bindings>
typename VertexInputStateCreateInfo<Bindings...>::M_AttributeDescHolder const
    VertexInputStateCreateInfo<Bindings...>::m_AttributeDescHolder = {};

class InputAssemblyStateCreateInfo {
public:
  InputAssemblyStateCreateInfo(
      VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
      VkBool32 restartEnable = false);

  operator VkPipelineInputAssemblyStateCreateInfo const &() const {
    return m_createInfo;
  }

  VkPrimitiveTopology topology() const { return m_createInfo.topology; }

  bool isPrimitivesRestartEnabled() const {
    return m_createInfo.primitiveRestartEnable;
  }

  virtual ~InputAssemblyStateCreateInfo() = default;

private:
  VkPipelineInputAssemblyStateCreateInfo m_createInfo{};
};

class RasterizationStateCreateInfo {
public:
  RasterizationStateCreateInfo(
      VkBool32 depthClampEnable = VK_FALSE,
      VkBool32 rasterizerDiscardEnable = VK_FALSE,
      VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
      VkCullModeFlags cullMode = VK_CULL_MODE_NONE,
      VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      VkBool32 depthBiasEnable = false, float depthBiasConstantFactor = 0,
      float depthBiasClamp = 0, float depthBiasSlopeFactor = 0,
      float lineWidth = 1.0f);

  virtual ~RasterizationStateCreateInfo() = default;

  operator VkPipelineRasterizationStateCreateInfo const &() const {
    return m_createInfo;
  }

private:
  VkPipelineRasterizationStateCreateInfo m_createInfo{};
};

class DepthTestStateCreateInfo {
public:
  DepthTestStateCreateInfo(VkCompareOp compareOp, bool writeEnable,
                           float minDepth = 0.0f, float maxDepth = 1.0f) {
    m_createInfo.depthCompareOp = compareOp;
    m_createInfo.depthWriteEnable = writeEnable;
    m_createInfo.minDepthBounds = minDepth;
    m_createInfo.maxDepthBounds = maxDepth;
  }
  operator VkPipelineDepthStencilStateCreateInfo const &() const {
    return m_createInfo;
  }

private:
  VkPipelineDepthStencilStateCreateInfo m_createInfo{};
};

class GraphicsPipelineCreateInfo {
public:
  GraphicsPipelineCreateInfo(RenderPassCRef renderPass, uint32_t subpass,
                             PipelineLayoutCRef layout);

  GraphicsPipelineCreateInfo(GraphicsPipelineCreateInfo &&another) noexcept;
  GraphicsPipelineCreateInfo(GraphicsPipelineCreateInfo const &another);

  GraphicsPipelineCreateInfo &
  addDepthTestState(DepthTestStateCreateInfo depthTest);
  GraphicsPipelineCreateInfo &
  addVertexShader(VertexShader const &shader,
                  SpecializationConstants const &constants = {});
  GraphicsPipelineCreateInfo &
  addFragmentShader(FragmentShader const &shader,
                    SpecializationConstants const &constants = {});
  GraphicsPipelineCreateInfo &
  addVertexInputState(VertexInputStateCreateInfoBaseHandle vertexInputState);
  GraphicsPipelineCreateInfo &
  addInputAssemblyState(InputAssemblyStateCreateInfo const &inputAssemblyState);
  GraphicsPipelineCreateInfo &
  addRasterizationState(RasterizationStateCreateInfo const &rasterizationState);
  GraphicsPipelineCreateInfo &
  addBlendState(VkPipelineColorBlendAttachmentState state, uint32_t attachment);
  GraphicsPipelineCreateInfo &addDynamicState(VkDynamicState state);

  operator VkGraphicsPipelineCreateInfo() const { return m_createInfo; }

  VertexInputStateCreateInfoBase const &vertexInputState() const {
    return *m_vertexInputStateCreateInfo;
  }

  VkPipelineInputAssemblyStateCreateInfo inputAssemblyState() const {
    return m_inputAssemblyStateCreateInfo;
  }

  VkPipelineRasterizationStateCreateInfo rasterizationState() const {
    return m_rasterizationStateCreateInfo;
  }

  PipelineLayout const &layout() const { return m_layout; }

  RenderPass const &pass() const { return m_renderPass; }

  uint32_t subpass() const { return m_createInfo.subpass; }

  std::optional<VertexShaderCRef> vertexShader() const {
    return m_vertexShader;
  }

  std::optional<FragmentShaderCRef> fragmentShader() const {
    return m_fragmentShader;
  }

private:
  RenderPassCRef m_renderPass;

  // Shader Stages

  std::vector<VkPipelineShaderStageCreateInfo> m_shaderStages;
  std::optional<VertexShaderCRef> m_vertexShader;
  std::optional<FragmentShaderCRef> m_fragmentShader;

  // TODO: support other shader stages

  // Fixed pipeline stages

  VertexInputStateCreateInfoBaseHandle m_vertexInputStateCreateInfo =
      NullVertexInputState();
  VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyStateCreateInfo;
  VkPipelineRasterizationStateCreateInfo m_rasterizationStateCreateInfo;

  // TODO: support configure for these stages
  VkPipelineViewportStateCreateInfo m_viewportState{};
  VkPipelineMultisampleStateCreateInfo m_multisampleState{};
  VkPipelineDepthStencilStateCreateInfo m_depthStencilState{};
  VkPipelineColorBlendStateCreateInfo m_colorBlendState{};
  std::vector<VkPipelineColorBlendAttachmentState> m_blendStates{};

  PipelineLayoutCRef m_layout;
  VkGraphicsPipelineCreateInfo m_createInfo{};

  // dynamic states

  VkPipelineDynamicStateCreateInfo m_dynamicState{};
  std::vector<VkDynamicState> m_dynStates;
};

class ComputePipelineCreateInfo {
public:
  ComputePipelineCreateInfo(PipelineLayout const &layout,
                            ComputeShader const &shader,
                            SpecializationConstants const &constants = {});

  operator VkComputePipelineCreateInfo() const { return m_createInfo; }

  PipelineLayoutCRef layout() const { return m_layout; }

private:
  PipelineLayoutCRef m_layout;
  VkComputePipelineCreateInfo m_createInfo;
};
class Pipeline {
public:
  Pipeline(Device &device, GraphicsPipelineCreateInfo const &createInfo);
  Pipeline(Device &device, ComputePipelineCreateInfo const &createInfo);

  Pipeline(Device &device, GraphicsPipelineCreateInfo const &createInfo,
           PipelineCache const &cache);
  Pipeline(Device &device, ComputePipelineCreateInfo const &createInfo,
           PipelineCache const &cache);

  Pipeline(Pipeline &&another) noexcept
      : m_device(another.m_device), m_pipeline(another.m_pipeline),
        m_pipelineLayout(another.m_pipelineLayout) {
    another.m_pipeline = VK_NULL_HANDLE;
  }

  PipelineLayout const &layout() const { return m_pipelineLayout; }

  virtual ~Pipeline();

  operator VkPipeline() const { return m_pipeline; }

private:
  Device &m_device;
  PipelineLayout const &m_pipelineLayout;
  VkPipeline m_pipeline = VK_NULL_HANDLE;
};

class GraphicsPipeline : public Pipeline {
public:
  GraphicsPipeline(Device &device, GraphicsPipelineCreateInfo createInfo)
      : Pipeline(device, createInfo), m_createInfo(std::move(createInfo)) {}

  GraphicsPipeline(Device &device, GraphicsPipelineCreateInfo createInfo,
                   PipelineCache const &cache)
      : Pipeline(device, createInfo, cache),
        m_createInfo(std::move(createInfo)) {}

  GraphicsPipelineCreateInfo const &info() const { return m_createInfo; }

private:
  GraphicsPipelineCreateInfo m_createInfo;
};

class ComputePipeline : public Pipeline {
public:
  ComputePipeline(Device &device, ComputePipelineCreateInfo createInfo)
      : Pipeline(device, createInfo), m_createInfo(createInfo){};

  ComputePipeline(Device &device, ComputePipelineCreateInfo createInfo,
                  PipelineCache const &cache)
      : Pipeline(device, createInfo, cache), m_createInfo(createInfo){};
  ComputePipelineCreateInfo const &info() const { return m_createInfo; }

private:
  ComputePipelineCreateInfo m_createInfo;
};
} // namespace vkw
#endif // VKRENDERER_PIPELINE_HPP
