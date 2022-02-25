#include "Pipeline.hpp"
#include "Device.hpp"
#include "RenderPass.hpp"
#include "Shader.hpp"
#include "Utils.hpp"

namespace vkw {

PipelineLayout::PipelineLayout(DeviceRef device) : m_device(device) {
  VkPipelineLayoutCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = 0;
  createInfo.setLayoutCount = 0;
  createInfo.pushConstantRangeCount = 0;

  VK_CHECK_RESULT(
      vkCreatePipelineLayout(m_device.get(), &createInfo, nullptr, &m_layout))
}

PipelineLayout::~PipelineLayout() {
  if (m_layout == VK_NULL_HANDLE)
    return;

  vkDestroyPipelineLayout(m_device.get(), m_layout, nullptr);
}

Pipeline::Pipeline(Device &device, GraphicsPipelineCreateInfo const &createInfo)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkGraphicsPipelineCreateInfo CI = createInfo; // TODO: remove this extra copy
  VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &CI,
                                            nullptr, &m_pipeline))
}

Pipeline::Pipeline(Device &device, ComputePipelineCreateInfo const &createInfo)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkComputePipelineCreateInfo CI = createInfo; // TODO: remove this extra copy
  VK_CHECK_RESULT(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &CI,
                                           nullptr, &m_pipeline))
}

Pipeline::~Pipeline() {
  if (m_pipeline == VK_NULL_HANDLE)
    return;

  vkDestroyPipeline(m_device, m_pipeline, nullptr);
}

VertexInputStateCreateInfoBase::VertexInputStateCreateInfoBase(
    uint32_t attributeDescCount,
    const VkVertexInputAttributeDescription *pAttributeDesc,
    uint32_t bindingDescCount,
    const VkVertexInputBindingDescription *pBindingDesc,
    VkPipelineVertexInputStateCreateFlags flags) {

  m_createInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.pVertexAttributeDescriptions = pAttributeDesc;
  m_createInfo.pVertexBindingDescriptions = pBindingDesc;
  m_createInfo.vertexAttributeDescriptionCount = attributeDescCount;
  m_createInfo.vertexBindingDescriptionCount = bindingDescCount;
}

VkVertexInputAttributeDescription
VertexInputStateCreateInfoBase::attribute(uint32_t index) const {
  if (index >= totalAttributes())
    throw Error(
        "VertexInputStateCreateInfoBase::attribute(" + std::to_string(index) +
        ") exceeded pVertexAttributeDescriptions array bounds (size = " +
        std::to_string(totalAttributes()) + ")");

  return m_createInfo.pVertexAttributeDescriptions[index];
}

VkVertexInputBindingDescription
VertexInputStateCreateInfoBase::binding(uint32_t index) const {
  if (index >= totalBindings())
    throw Error("VertexInputStateCreateInfoBase::binding(" +
                std::to_string(index) +
                ") exceeded pVertexBindingDescriptions array bounds (size = " +
                std::to_string(totalBindings()) + ")");

  return m_createInfo.pVertexBindingDescriptions[index];
}

InputAssemblyStateCreateInfo::InputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology, VkBool32 restartEnable) {
  m_createInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.topology = topology;
  m_createInfo.primitiveRestartEnable = restartEnable;
  m_createInfo.flags = 0;
}

RasterizationStateCreateInfo::RasterizationStateCreateInfo(
    VkBool32 depthClampEnable, VkBool32 rasterizerDiscardEnable,
    VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace,
    VkBool32 depthBiasEnable, float depthBiasConstantFactor,
    float depthBiasClamp, float depthBiasSlopeFactor, float lineWidth) {
  m_createInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = 0;
  m_createInfo.depthClampEnable = depthBiasEnable;
  m_createInfo.rasterizerDiscardEnable = rasterizerDiscardEnable;
  m_createInfo.polygonMode = polygonMode;
  m_createInfo.cullMode = cullMode;
  m_createInfo.frontFace = frontFace;
  m_createInfo.depthBiasEnable = depthBiasEnable;
  m_createInfo.depthBiasConstantFactor = depthBiasConstantFactor;
  m_createInfo.depthBiasClamp = depthBiasClamp;
  m_createInfo.depthBiasSlopeFactor = depthBiasSlopeFactor;
  m_createInfo.lineWidth = lineWidth;
}
GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(
    RenderPassCRef renderPass, uint32_t subpass, PipelineLayoutCRef layout,
    const ShaderBaseConstRefArray &shaderStages,
    VertexInputStateCreateInfoBaseHandle vertexInputState,
    InputAssemblyStateCreateInfo inputAssemblyStateCreateInfo,
    RasterizationStateCreateInfo rasterizationStateCreateInfo)
    : m_layout(layout), m_inputAssemblyStateCreateInfo{std::move(
                            inputAssemblyStateCreateInfo)},
      m_renderPass(renderPass),
      m_rasterizationStateCreateInfo(rasterizationStateCreateInfo),
      m_vertexShader(m_find_vertex_shader(shaderStages)),
      m_vertexInputStateCreateInfo(std::move(vertexInputState)) {

  // TODO: this should wrapped around somewhere else and be configurable

  // MultisampleStateCreateInfo
  m_multisampleState.sampleShadingEnable = VK_FALSE;
  m_multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  m_multisampleState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_multisampleState.pNext = nullptr;
  m_multisampleState.flags = 0;

  // Depth/stencil state create info
  m_depthStencilState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_depthStencilState.pNext = nullptr;
  m_depthStencilState.depthTestEnable = VK_FALSE;
  m_depthStencilState.depthWriteEnable = VK_FALSE;
  m_depthStencilState.stencilTestEnable = VK_FALSE;
  m_depthStencilState.depthBoundsTestEnable = VK_FALSE;

  // Color blending state create info
  m_colorBlendState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  m_colorBlendState.pNext = nullptr;
  m_colorBlendState.flags = 0;
  m_colorBlendState.attachmentCount =
      m_renderPass.get().info().subpassInfo(subpass).colorAttachments.size();
  VkPipelineColorBlendAttachmentState state{};
  state.blendEnable = VK_FALSE;
  state.colorWriteMask = 0xf;
  m_blendStates.resize(m_colorBlendState.attachmentCount, state);
  m_colorBlendState.pAttachments = m_blendStates.data();
  m_colorBlendState.logicOpEnable = VK_FALSE;

  // for now will assume viewport state to be dynamic
  m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  m_viewportState.pNext = nullptr;
  m_viewportState.flags = 0;
  m_viewportState.viewportCount = m_viewportState.scissorCount = 1;
  m_dynStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
  m_dynStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
  m_dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  m_dynamicState.pNext = nullptr;
  m_dynamicState.dynamicStateCount = m_dynStates.size();
  m_dynamicState.pDynamicStates = m_dynStates.data();
  m_dynamicState.flags = 0;

  m_createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = 0;
  m_createInfo.renderPass = m_renderPass.get();
  m_createInfo.layout = m_layout.get();
  m_createInfo.subpass = subpass;
  m_createInfo.pColorBlendState = &m_colorBlendState;
  m_createInfo.pDepthStencilState = &m_depthStencilState;
  m_createInfo.pDynamicState = &m_dynamicState;
  m_createInfo.pViewportState = &m_viewportState;
  m_createInfo.pInputAssemblyState =
      &(m_inputAssemblyStateCreateInfo.
        operator const VkPipelineInputAssemblyStateCreateInfo &());
  m_createInfo.pMultisampleState = &m_multisampleState;
  m_createInfo.pRasterizationState =
      &(m_rasterizationStateCreateInfo.
        operator const VkPipelineRasterizationStateCreateInfo &());
  m_createInfo.pTessellationState =
      nullptr; // TODO: implement tesselation shader support
  m_createInfo.pVertexInputState =
      &(m_vertexInputStateCreateInfo->
        operator const VkPipelineVertexInputStateCreateInfo &());

  m_shaderStages.reserve(shaderStages.size());
  for (auto const &shader : shaderStages) {
    VkPipelineShaderStageCreateInfo ci{};
    ci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ci.flags = 0;
    ci.pNext = nullptr;
    ci.module = shader.get();
    ci.pName = "main"; // TODO: allow configure shader/kernel entry point name
    ci.stage = shader.get().stage();
    ci.pSpecializationInfo = nullptr; // TODO: implement spec constant support
    m_shaderStages.push_back(ci);
  }

  m_createInfo.stageCount = m_shaderStages.size();
  m_createInfo.pStages = m_shaderStages.data();
}

VertexShaderCRef GraphicsPipelineCreateInfo::m_find_vertex_shader(
    ShaderBaseConstRefArray const &shaderStages) {
  auto found =
      std::find_if(shaderStages.begin(), shaderStages.end(),
                   [](ShaderBaseConstRefArray::TRef const &shader) {
                     return shader.get().stage() == VK_SHADER_STAGE_VERTEX_BIT;
                   });

  if (found == shaderStages.end())
    throw Error("GraphicsPipelineCreateInfo construction failed: no vertex "
                "shader passed");

  return {(dynamic_cast<VertexShader const &>(found->get()))};
}

} // namespace vkw