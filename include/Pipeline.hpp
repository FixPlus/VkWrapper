#ifndef VKRENDERER_PIPELINE_HPP
#define VKRENDERER_PIPELINE_HPP

#include "Common.hpp"
#include "VertexBuffer.hpp"
#include <functional>
#include <optional>
#include <vulkan/vulkan.h>

namespace vkw {

class PipelineLayout {
public:
  // TODO

  operator VkPipelineLayout() const { return m_layout; }

private:
  VkPipelineLayout m_layout;
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
    void m_init_array(uint32_t initIndex = 0) {
      uint32_t offset = 0u;
      uint32_t location = 0u;
      for (uint32_t i = initIndex;
           i < initIndex + M_binding::Attributes::count(); ++i) {
        auto attrType = M_binding::Attributes::getAttrType(i);
        m_attr_desc[i].binding = M_binding::binding;
        m_attr_desc[i].location = location;
        m_attr_desc[i].offset = offset;
        m_attr_desc[i].format = format_of(attrType);
        location += locations_hold(attrType);
        offset += size_of(attrType);
      }
      m_init_array<M_dummy, M_bindings...>(initIndex +
                                           M_binding::Attributes::count());
    }

    template <BindingPointDescriptionLike M_binding>
    void m_init_array(uint32_t initIndex = 0) {
      uint32_t offset = 0u;
      uint32_t location = 0u;
      for (uint32_t i = initIndex;
           i < initIndex + M_binding::Attributes::count(); ++i) {
        auto attrType = M_binding::Attributes::getAttrType(i);
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
      VkBool32 depthClampEnable = true, VkBool32 rasterizerDiscardEnable = true,
      VkPolygonMode polygonMode = VK_POLYGON_MODE_FILL,
      VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
      VkFrontFace frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      VkBool32 depthBiasEnable = false, float depthBiasConstantFactor = 0,
      float depthBiasClamp = 0, float depthBiasSlopeFactor = 0,
      float lineWidth = 1.0f);

  virtual ~RasterizationStateCreateInfo() = default;

  operator VkPipelineRasterizationStateCreateInfo const &() const {
    return m_createInfo;
  }

private:
  VkPipelineRasterizationStateCreateInfo m_createInfo;
};
class GraphicsPipelineCreateInfo {
public:
  GraphicsPipelineCreateInfo(
      RenderPassCRef renderPass, uint32_t subpass, PipelineLayoutCRef layout,
      ShaderBaseConstRefArray const &shaderStages,
      VertexInputStateCreateInfoBaseHandle vertexInputState =
          NullVertexInputState(),
      InputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {},
      RasterizationStateCreateInfo rasterizationStateCreateInfo = {});

  operator VkGraphicsPipelineCreateInfo() const { return m_createInfo; }

  VertexInputStateCreateInfoBase const &vertexInputState() const {
    return *m_vertexInputStateCreateInfo;
  }

  InputAssemblyStateCreateInfo const &inputAssemblyState() const {
    return m_inputAssemblyStateCreateInfo;
  }

  RasterizationStateCreateInfo const &rasterizationState() const {
    return m_rasterizationStateCreateInfo;
  }

  PipelineLayout const &layout() const { return m_layout; }

  RenderPass const &pass() const { return m_renderPass; }

  uint32_t subpass() const { return m_createInfo.subpass; }

  VertexShader const &vertexShader() const { return m_vertexShader; }

  std::optional<FragmentShaderCRef> fragmentShader() const {
    return m_fragmentShader;
  }

private:
  static VertexShaderCRef
  m_find_vertex_shader(ShaderBaseConstRefArray const &shaderStages);

  RenderPassCRef m_renderPass;

  // Shader Stages

  VertexShaderCRef m_vertexShader; // Vertex Shader stage is obligatory
                                   // according to Vulkan Spec
  std::optional<FragmentShaderCRef> m_fragmentShader;
  // TODO: support other shader stages

  // Fixed pipeline stages

  VertexInputStateCreateInfoBaseHandle m_vertexInputStateCreateInfo;
  InputAssemblyStateCreateInfo m_inputAssemblyStateCreateInfo;
  RasterizationStateCreateInfo m_rasterizationStateCreateInfo;

  // TODO: support configure for these stages
  VkPipelineViewportStateCreateInfo m_viewportState{};
  VkPipelineMultisampleStateCreateInfo m_multisampleState{};
  VkPipelineDepthStencilStateCreateInfo m_depthStencilState{};
  VkPipelineColorBlendStateCreateInfo m_colorBlendState{};
  VkPipelineDynamicStateCreateInfo m_dynamicState{};

  PipelineLayoutCRef m_layout;
  VkGraphicsPipelineCreateInfo m_createInfo{};
};

class ComputePipelineCreateInfo {
public:
  // TODO

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

  Pipeline(Pipeline &&another) noexcept
      : m_device(another.m_device), m_pipeline(another.m_pipeline),
        m_pipelineLayout(another.m_pipelineLayout) {
    another.m_pipeline = VK_NULL_HANDLE;
  }

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

  GraphicsPipelineCreateInfo const &info() const { return m_createInfo; }

private:
  GraphicsPipelineCreateInfo m_createInfo;
};

class ComputePipeline : public Pipeline {
public:
  ComputePipeline(Device &device, ComputePipelineCreateInfo createInfo)
      : Pipeline(device, createInfo), m_createInfo(createInfo){

                                      };

  ComputePipelineCreateInfo const &info() const { return m_createInfo; }

private:
  ComputePipelineCreateInfo m_createInfo;
};
} // namespace vkw
#endif // VKRENDERER_PIPELINE_HPP
