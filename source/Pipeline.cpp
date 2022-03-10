#include "vkw/Pipeline.hpp"
#include "Utils.hpp"
#include "vkw/DescriptorSet.hpp"
#include "vkw/Device.hpp"
#include "vkw/RenderPass.hpp"
#include "vkw/Shader.hpp"

namespace vkw {

PipelineLayout::PipelineLayout(DeviceRef device,
                               VkPipelineLayoutCreateFlags flags)
    : m_device(device) {
  m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.setLayoutCount = 0;
  m_createInfo.pushConstantRangeCount = 0;

  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreatePipelineLayout(
      m_device.get(), &m_createInfo, nullptr, &m_layout))
}
PipelineLayout::PipelineLayout(
    DeviceRef device, DescriptorSetLayoutConstRefArray const &setLayouts,
    VkPipelineLayoutCreateFlags flags)
    : m_device(device) {
  m_descriptorLayouts.reserve(setLayouts.size());

  for (auto const &layout : setLayouts) {
    m_descriptorLayouts.emplace_back(layout);
  }

  m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.setLayoutCount = setLayouts.size();
  m_createInfo.pSetLayouts = setLayouts;
  m_createInfo.pushConstantRangeCount = 0; // TODO

  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreatePipelineLayout(
      m_device.get(), &m_createInfo, nullptr, &m_layout))
}

PipelineLayout::~PipelineLayout() {
  if (m_layout == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroyPipelineLayout(m_device.get(), m_layout,
                                                      nullptr);
}
bool PipelineLayout::operator==(PipelineLayout const &rhs) const {
  if (m_createInfo.flags != rhs.m_createInfo.flags ||
      m_descriptorLayouts.size() != rhs.m_descriptorLayouts.size())
    return false;
  auto rhsLayoutIter = rhs.m_descriptorLayouts.begin();
  return std::all_of(m_descriptorLayouts.begin(), m_descriptorLayouts.end(),
                     [&rhsLayoutIter](DescriptorSetLayout const &layout) {
                       return layout == *(rhsLayoutIter++);
                     });
}

Pipeline::Pipeline(Device &device, GraphicsPipelineCreateInfo const &createInfo)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkGraphicsPipelineCreateInfo CI = createInfo; // TODO: remove this extra copy
  VK_CHECK_RESULT(m_device.core<1, 0>().vkCreateGraphicsPipelines(
      device, VK_NULL_HANDLE, 1, &CI, nullptr, &m_pipeline))
}

Pipeline::Pipeline(Device &device, ComputePipelineCreateInfo const &createInfo)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkComputePipelineCreateInfo CI = createInfo; // TODO: remove this extra copy
  VK_CHECK_RESULT(m_device.core<1, 0>().vkCreateComputePipelines(
      device, VK_NULL_HANDLE, 1, &CI, nullptr, &m_pipeline))
}

Pipeline::~Pipeline() {
  if (m_pipeline == VK_NULL_HANDLE)
    return;

  m_device.core<1, 0>().vkDestroyPipeline(m_device, m_pipeline, nullptr);
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
    RenderPassCRef renderPass, uint32_t subpass, PipelineLayoutCRef layout)
    : m_renderPass(renderPass.get()), m_layout(layout.get()) {

  m_inputAssemblyStateCreateInfo = vkw::InputAssemblyStateCreateInfo{};
  m_rasterizationStateCreateInfo = vkw::RasterizationStateCreateInfo{};

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
  m_createInfo.pInputAssemblyState = &m_inputAssemblyStateCreateInfo;
  m_createInfo.pMultisampleState = &m_multisampleState;
  m_createInfo.pRasterizationState = &m_rasterizationStateCreateInfo;
  m_createInfo.pTessellationState =
      nullptr; // TODO: implement tesselation shader support
  m_createInfo.pVertexInputState =
      &(m_vertexInputStateCreateInfo->
        operator const VkPipelineVertexInputStateCreateInfo &());

  m_createInfo.stageCount = m_shaderStages.size();
  m_createInfo.pStages = m_shaderStages.data();
}
void GraphicsPipelineCreateInfo::addVertexInputState(
    VertexInputStateCreateInfoBaseHandle vertexInputState) {
  m_vertexInputStateCreateInfo = std::move(vertexInputState);
  m_createInfo.pVertexInputState =
      &(m_vertexInputStateCreateInfo->
        operator const VkPipelineVertexInputStateCreateInfo &());
}
void GraphicsPipelineCreateInfo::addInputAssemblyState(
    InputAssemblyStateCreateInfo const &inputAssemblyState) {
  m_inputAssemblyStateCreateInfo = inputAssemblyState;
}
void GraphicsPipelineCreateInfo::addRasterizationState(
    const RasterizationStateCreateInfo &rasterizationState) {
  m_rasterizationStateCreateInfo = rasterizationState;
}
void GraphicsPipelineCreateInfo::addFragmentShader(
    const FragmentShader &shader) {
  if (m_fragmentShader.has_value()) {
    auto module = m_fragmentShader.value().get().operator VkShaderModule_T *();
    std::erase_if(m_shaderStages,
                  [module](VkPipelineShaderStageCreateInfo info) {
                    return info.module == module;
                  });
  }
  VkPipelineShaderStageCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.module = shader;
  createInfo.pName = "main";
  createInfo.pSpecializationInfo = nullptr;
  createInfo.flags = 0;
  createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

  m_shaderStages.push_back(createInfo);
  m_fragmentShader.emplace(shader);
  m_createInfo.pStages = m_shaderStages.data();
  m_createInfo.stageCount = m_shaderStages.size();
}
void GraphicsPipelineCreateInfo::addVertexShader(const VertexShader &shader) {
  if (m_vertexShader.has_value()) {
    auto module = m_fragmentShader.value().get().operator VkShaderModule_T *();
    std::erase_if(m_shaderStages,
                  [module](VkPipelineShaderStageCreateInfo info) {
                    return info.module == module;
                  });
  }
  VkPipelineShaderStageCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.module = shader;
  createInfo.pName = "main";
  createInfo.pSpecializationInfo = nullptr;
  createInfo.flags = 0;
  createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

  m_shaderStages.push_back(createInfo);
  m_vertexShader.emplace(shader);
  m_createInfo.pStages = m_shaderStages.data();
  m_createInfo.stageCount = m_shaderStages.size();
}
void GraphicsPipelineCreateInfo::addDepthTestState(
    DepthTestStateCreateInfo depthTest) {
  auto dTest =
      depthTest.operator const VkPipelineDepthStencilStateCreateInfo &();
  m_depthStencilState.depthTestEnable = VK_TRUE;
  m_depthStencilState.depthWriteEnable = dTest.depthWriteEnable;
  m_depthStencilState.depthCompareOp = dTest.depthCompareOp;
  m_depthStencilState.minDepthBounds = dTest.minDepthBounds;
  m_depthStencilState.maxDepthBounds = dTest.maxDepthBounds;
}
GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(
    GraphicsPipelineCreateInfo &&another) noexcept
    : m_renderPass(another.m_renderPass), m_layout(another.m_layout),
      m_depthStencilState(another.m_depthStencilState),
      m_createInfo(another.m_createInfo),
      m_colorBlendState(another.m_colorBlendState),
      m_dynamicState(another.m_dynamicState),
      m_multisampleState(another.m_multisampleState),
      m_viewportState(another.m_viewportState),
      m_vertexShader(another.m_vertexShader),
      m_fragmentShader(another.m_fragmentShader),
      m_shaderStages(std::move(another.m_shaderStages)),
      m_rasterizationStateCreateInfo(another.m_rasterizationStateCreateInfo),
      m_inputAssemblyStateCreateInfo(another.m_inputAssemblyStateCreateInfo),
      m_vertexInputStateCreateInfo(
          std::move(another.m_vertexInputStateCreateInfo)),
      m_blendStates(std::move(another.m_blendStates)),
      m_dynStates(std::move(another.m_dynStates)) {
  m_createInfo.pColorBlendState = &m_colorBlendState;
  m_createInfo.pDepthStencilState = &m_depthStencilState;
  m_createInfo.pDynamicState = &m_dynamicState;
  m_createInfo.pViewportState = &m_viewportState;
  m_createInfo.pInputAssemblyState = &m_inputAssemblyStateCreateInfo;
  m_createInfo.pMultisampleState = &m_multisampleState;
  m_createInfo.pRasterizationState = &m_rasterizationStateCreateInfo;
  m_createInfo.pTessellationState = nullptr;
  m_createInfo.pVertexInputState =
      &(m_vertexInputStateCreateInfo->
        operator const VkPipelineVertexInputStateCreateInfo &());
}
GraphicsPipelineCreateInfo::GraphicsPipelineCreateInfo(
    const GraphicsPipelineCreateInfo &another)
    : m_renderPass(another.m_renderPass), m_layout(another.m_layout),
      m_depthStencilState(another.m_depthStencilState),
      m_createInfo(another.m_createInfo),
      m_colorBlendState(another.m_colorBlendState),
      m_dynamicState(another.m_dynamicState),
      m_multisampleState(another.m_multisampleState),
      m_viewportState(another.m_viewportState),
      m_vertexShader(another.m_vertexShader),
      m_fragmentShader(another.m_fragmentShader),
      m_shaderStages(another.m_shaderStages),
      m_rasterizationStateCreateInfo(another.m_rasterizationStateCreateInfo),
      m_inputAssemblyStateCreateInfo(another.m_inputAssemblyStateCreateInfo),
      m_vertexInputStateCreateInfo(another.m_vertexInputStateCreateInfo),
      m_blendStates(another.m_blendStates), m_dynStates(another.m_dynStates) {
  m_createInfo.pColorBlendState = &m_colorBlendState;
  m_createInfo.pDepthStencilState = &m_depthStencilState;
  m_createInfo.pDynamicState = &m_dynamicState;
  m_createInfo.pViewportState = &m_viewportState;
  m_createInfo.pInputAssemblyState = &m_inputAssemblyStateCreateInfo;
  m_createInfo.pMultisampleState = &m_multisampleState;
  m_createInfo.pRasterizationState = &m_rasterizationStateCreateInfo;
  m_createInfo.pTessellationState = nullptr;
  m_createInfo.pVertexInputState =
      &(m_vertexInputStateCreateInfo->
        operator const VkPipelineVertexInputStateCreateInfo &());
  m_colorBlendState.pAttachments = m_blendStates.data();
  m_dynamicState.pDynamicStates = m_dynStates.data();
  m_createInfo.pStages = m_shaderStages.data();
};

} // namespace vkw