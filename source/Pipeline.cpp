#include "vkw/Pipeline.hpp"
#include "Utils.hpp"
#include "vkw/DescriptorSet.hpp"
#include "vkw/Device.hpp"
#include "vkw/PipelineCache.hpp"
#include "vkw/RenderPass.hpp"
#include "vkw/Shader.hpp"
#include <cstring>

namespace vkw {

namespace {

unsigned getProperMaskLength(VkSampleCountFlagBits samples) {
  switch (samples) {
  case VK_SAMPLE_COUNT_64_BIT:
    return 2;
  default:
    return 1;
  }
}

} // namespace

bool PipelineLayoutInfo::operator==(
    PipelineLayoutInfo const &rhs) const noexcept {
  if (m_createInfo.flags != rhs.m_createInfo.flags ||
      m_descriptorLayouts.size() != rhs.m_descriptorLayouts.size())
    return false;
  auto rhsLayoutIter = rhs.m_descriptorLayouts.begin();
  return std::all_of(m_descriptorLayouts.begin(), m_descriptorLayouts.end(),
                     [&rhsLayoutIter](DescriptorSetLayout const &layout) {
                       return layout == *(rhsLayoutIter++);
                     });
}
void PipelineLayoutInfo::m_fillInfo(VkPipelineLayoutCreateFlags flags) noexcept(
    ExceptionsDisabled) {

  std::transform(m_descriptorLayouts.begin(), m_descriptorLayouts.end(),
                 std::back_inserter(m_rawLayout),
                 [](auto &layout) -> VkDescriptorSetLayout {
                   return layout.operator const vkw::DescriptorSetLayout &();
                 });
  m_createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = flags;
  m_createInfo.setLayoutCount = m_rawLayout.size();
  m_createInfo.pSetLayouts = m_rawLayout.data();
  // TODO : verify push constant ranges against the device limits
  m_createInfo.pushConstantRangeCount = m_pushConstants.size();
  m_createInfo.pPushConstantRanges = m_pushConstants.data();
}

Pipeline::Pipeline(
    Device &device,
    GraphicsPipelineCreateInfo const &createInfo) noexcept(ExceptionsDisabled)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkGraphicsPipelineCreateInfo const &CI = createInfo;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateGraphicsPipelines(
      device, VK_NULL_HANDLE, 1, &CI, device.hostAllocator().allocator(),
      &m_pipeline))
}

Pipeline::Pipeline(
    Device &device,
    ComputePipelineCreateInfo const &createInfo) noexcept(ExceptionsDisabled)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkComputePipelineCreateInfo const &CI = createInfo;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateComputePipelines(
      device, VK_NULL_HANDLE, 1, &CI, device.hostAllocator().allocator(),
      &m_pipeline))
}

Pipeline::Pipeline(Device &device, GraphicsPipelineCreateInfo const &createInfo,
                   PipelineCache const &cache) noexcept(ExceptionsDisabled)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkGraphicsPipelineCreateInfo const &CI = createInfo;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateGraphicsPipelines(
      device, cache, 1, &CI, device.hostAllocator().allocator(), &m_pipeline))
}

Pipeline::Pipeline(Device &device, ComputePipelineCreateInfo const &createInfo,
                   PipelineCache const &cache) noexcept(ExceptionsDisabled)
    : m_device(device), m_pipelineLayout(createInfo.layout()) {
  VkComputePipelineCreateInfo const &CI = createInfo;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateComputePipelines(
      device, cache, 1, &CI, device.hostAllocator().allocator(), &m_pipeline))
}

Pipeline::~Pipeline() {
  if (m_pipeline == VK_NULL_HANDLE)
    return;

  m_device.get().core<1, 0>().vkDestroyPipeline(
      m_device.get(), m_pipeline, m_device.get().hostAllocator().allocator());
}

VertexInputStateCreateInfoBase::VertexInputStateCreateInfoBase(
    uint32_t attributeDescCount,
    const VkVertexInputAttributeDescription *pAttributeDesc,
    uint32_t bindingDescCount,
    const VkVertexInputBindingDescription *pBindingDesc,
    VkPipelineVertexInputStateCreateFlags flags) noexcept {

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
VertexInputStateCreateInfoBase::attribute(uint32_t index) const
    noexcept(ExceptionsDisabled) {
  if (index >= totalAttributes())
    postError(Error(
        "VertexInputStateCreateInfoBase::attribute(" + std::to_string(index) +
        ") exceeded pVertexAttributeDescriptions array bounds (size = " +
        std::to_string(totalAttributes()) + ")"));

  return m_createInfo.pVertexAttributeDescriptions[index];
}

VkVertexInputBindingDescription
VertexInputStateCreateInfoBase::binding(uint32_t index) const
    noexcept(ExceptionsDisabled) {
  if (index >= totalBindings())
    postError(Error(
        "VertexInputStateCreateInfoBase::binding(" + std::to_string(index) +
        ") exceeded pVertexBindingDescriptions array bounds (size = " +
        std::to_string(totalBindings()) + ")"));

  return m_createInfo.pVertexBindingDescriptions[index];
}

NullVertexInputState &NullVertexInputState::get() noexcept {
  static NullVertexInputState handle;
  return handle;
}

InputAssemblyStateCreateInfo::InputAssemblyStateCreateInfo(
    VkPrimitiveTopology topology, VkBool32 restartEnable) noexcept {
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
    float depthBiasClamp, float depthBiasSlopeFactor,
    float lineWidth) noexcept {
  m_createInfo.sType =
      VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.flags = 0;
  m_createInfo.depthClampEnable = depthClampEnable;
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
    RenderPass const &renderPass, uint32_t subpass,
    PipelineLayout const &layout) noexcept(ExceptionsDisabled)
    : m_renderPass(renderPass), m_layout(layout) {

  m_inputAssemblyStateCreateInfo = vkw::InputAssemblyStateCreateInfo{};
  m_rasterizationStateCreateInfo = vkw::RasterizationStateCreateInfo{};

  // Default MultisampleStateCreateInfo
  m_multisampleState.sampleShadingEnable = VK_FALSE;
  m_multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
  m_multisampleState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  m_multisampleState.pNext = nullptr;
  m_multisampleState.flags = 0;

  // Default Depth/stencil state create info
  m_depthStencilState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  m_depthStencilState.pNext = nullptr;
  m_depthStencilState.depthTestEnable = VK_FALSE;
  m_depthStencilState.depthWriteEnable = VK_FALSE;
  m_depthStencilState.stencilTestEnable = VK_FALSE;
  m_depthStencilState.depthBoundsTestEnable = VK_FALSE;

  // Default Color blending state create info
  m_colorBlendState.sType =
      VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  m_colorBlendState.pNext = nullptr;
  m_colorBlendState.flags = 0;
  m_colorBlendState.attachmentCount =
      m_renderPass.get().subpassInfo(subpass).colorAttachments.size();
  VkPipelineColorBlendAttachmentState state{};
  state.blendEnable = VK_FALSE;
  state.colorWriteMask = 0xf;
  m_blendStates.resize(m_colorBlendState.attachmentCount, state);
  m_colorBlendState.pAttachments = m_blendStates.data();
  m_colorBlendState.logicOpEnable = VK_FALSE;

  m_viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  m_viewportState.pNext = nullptr;
  m_viewportState.flags = 0;
  m_viewportState.viewportCount = m_viewportState.scissorCount = 1;

  m_dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  m_dynamicState.pNext = nullptr;
  m_dynamicState.dynamicStateCount = 0;
  m_dynamicState.pDynamicStates = nullptr;
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
      &(m_vertexInputStateCreateInfo.get().
        operator const VkPipelineVertexInputStateCreateInfo &());

  m_createInfo.stageCount = m_shaderStages.size();
  m_createInfo.pStages = m_shaderStages.data();
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::addVertexInputState(
    VertexInputStateCreateInfoBase const &vertexInputState) noexcept {
  m_vertexInputStateCreateInfo = vertexInputState;
  m_createInfo.pVertexInputState =
      &(m_vertexInputStateCreateInfo.get().
        operator const VkPipelineVertexInputStateCreateInfo &());

  return *this;
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::addInputAssemblyState(
    InputAssemblyStateCreateInfo const &inputAssemblyState) noexcept {
  m_inputAssemblyStateCreateInfo = inputAssemblyState;
  return *this;
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::addRasterizationState(
    const RasterizationStateCreateInfo &rasterizationState) noexcept {
  m_rasterizationStateCreateInfo = rasterizationState;
  return *this;
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::addFragmentShader(
    const FragmentShader &shader,
    SpecializationConstants const &constants) noexcept {
  if (m_fragmentShader.has_value()) {
    auto module = m_fragmentShader.value().get().operator VkShaderModule_T *();
    auto newEnd =
        std::remove_if(m_shaderStages.begin(), m_shaderStages.end(),
                       [module](VkPipelineShaderStageCreateInfo info) {
                         return info.module == module;
                       });

    m_shaderStages.erase(newEnd);
  }
  VkPipelineShaderStageCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.module = shader;
  createInfo.pName = "main";
  if (!constants.empty())
    createInfo.pSpecializationInfo =
        &(constants.operator const VkSpecializationInfo &());
  createInfo.flags = 0;
  createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;

  m_shaderStages.push_back(createInfo);
  m_fragmentShader.emplace(shader);
  m_createInfo.pStages = m_shaderStages.data();
  m_createInfo.stageCount = m_shaderStages.size();
  return *this;
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::addVertexShader(
    const VertexShader &shader,
    SpecializationConstants const &constants) noexcept {
  if (m_vertexShader.has_value()) {
    auto module = m_fragmentShader.value().get().operator VkShaderModule_T *();
    auto newEnd =
        std::remove_if(m_shaderStages.begin(), m_shaderStages.end(),
                       [module](VkPipelineShaderStageCreateInfo info) {
                         return info.module == module;
                       });

    m_shaderStages.erase(newEnd);
  }
  VkPipelineShaderStageCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.module = shader;
  createInfo.pName = "main";
  if (!constants.empty())
    createInfo.pSpecializationInfo =
        &(constants.operator const VkSpecializationInfo &());
  createInfo.flags = 0;
  createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

  m_shaderStages.push_back(createInfo);
  m_vertexShader.emplace(shader);
  m_createInfo.pStages = m_shaderStages.data();
  m_createInfo.stageCount = m_shaderStages.size();
  return *this;
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::addDepthTestState(
    DepthTestStateCreateInfo depthTest) noexcept {
  auto dTest =
      depthTest.operator const VkPipelineDepthStencilStateCreateInfo &();
  m_depthStencilState.depthTestEnable = VK_TRUE;
  m_depthStencilState.depthWriteEnable = dTest.depthWriteEnable;
  m_depthStencilState.depthCompareOp = dTest.depthCompareOp;
  m_depthStencilState.minDepthBounds = dTest.minDepthBounds;
  m_depthStencilState.maxDepthBounds = dTest.maxDepthBounds;
  return *this;
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
      &(m_vertexInputStateCreateInfo.get().
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
      &(m_vertexInputStateCreateInfo.get().
        operator const VkPipelineVertexInputStateCreateInfo &());
  m_colorBlendState.pAttachments = m_blendStates.data();
  m_dynamicState.pDynamicStates = m_dynStates.data();
  m_createInfo.pStages = m_shaderStages.data();
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::addBlendState(
    VkPipelineColorBlendAttachmentState state, uint32_t attachment) noexcept {

  m_blendStates.at(attachment) = state;

  return *this;
}
GraphicsPipelineCreateInfo &
GraphicsPipelineCreateInfo::addDynamicState(VkDynamicState state) noexcept {
  if (std::find(m_dynStates.begin(), m_dynStates.end(), state) ==
      m_dynStates.end()) {
    m_dynStates.emplace_back(state);
    m_dynamicState.dynamicStateCount++;
    m_dynamicState.pDynamicStates = m_dynStates.data();
  }
  return *this;
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::enableMultisampling(
    bool alphaToCoverageEnable,
    bool alphaToOneEnable) noexcept(ExceptionsDisabled) {
  auto &subpassInfo = m_renderPass.get().subpassInfo(m_createInfo.subpass);
  if (subpassInfo.resolveAttachments.empty())
    postError(Error{"enableMultisampling() cannot be called if render pass "
                    "subpass have no resolve attachments"});
  auto attachmentIndex = subpassInfo.resolveAttachments.front().attachment;
  auto &attachmentDesc =
      m_renderPass.get().attachmentDescriptions().at(attachmentIndex).get();
  m_multisampleState.rasterizationSamples = attachmentDesc.samples();
  m_multisampleState.alphaToCoverageEnable = alphaToCoverageEnable;
  m_multisampleState.alphaToOneEnable = alphaToOneEnable;
  return *this;
};

GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::enableSampleRateShading(
    float minRate) noexcept(ExceptionsDisabled) {
  if (m_multisampleState.rasterizationSamples == VK_SAMPLE_COUNT_1_BIT) {
    postError(Error{"enableSampleRateShading() cannot be called if "
                    "multisampling is not enabled"});
  }
  if (pass().parent().physicalDevice().enabledFeatures().sampleRateShading ==
      VK_FALSE) {
    postError(Error{"enableSampleRateShading() cannot be called if "
                    "sampleRateShading feature is not enabled"});
  }

  m_multisampleState.sampleShadingEnable = VK_TRUE;
  m_multisampleState.minSampleShading = minRate;
  return *this;
}
GraphicsPipelineCreateInfo &GraphicsPipelineCreateInfo::setSampleMask(
    std::span<VkSampleMask> mask) noexcept(ExceptionsDisabled) {
  if (m_multisampleState.rasterizationSamples == VK_SAMPLE_COUNT_1_BIT) {
    postError(Error{"setSampleMask() cannot be called if "
                    "multisampling is not enabled"});
  }
  if (mask.size() !=
      getProperMaskLength(m_multisampleState.rasterizationSamples)) {
    postError(Error{"mask provided ro setSampleMask() has incorrect size"});
  }
  m_sampleMask.clear();
  std::copy(mask.begin(), mask.end(), std::back_inserter(m_sampleMask));
  return *this;
}
ComputePipelineCreateInfo::ComputePipelineCreateInfo(
    const PipelineLayout &layout, const ComputeShader &shader,
    SpecializationConstants constants) noexcept
    : m_layout(layout), m_constants(std::move(constants)) {
  m_createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.layout = layout;
  m_createInfo.flags = 0; // TODO
  m_createInfo.stage.module = shader;
  m_createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
  m_createInfo.stage.sType =
      VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  m_createInfo.stage.pNext = nullptr;
  m_createInfo.stage.pName = "main"; // TODO
  m_createInfo.stage.flags = 0;      // TODO
  if (!constants.empty())
    m_createInfo.stage.pSpecializationInfo =
        &(m_constants.operator const VkSpecializationInfo &());
  else
    m_createInfo.stage.pSpecializationInfo = nullptr;
}
SpecializationConstants::SpecializationConstants() { m_info.mapEntryCount = 0; }
void SpecializationConstants::m_addConstant(
    const void *constant, size_t size,
    uint32_t id) noexcept(ExceptionsDisabled) {
  if (std::find_if(m_entries.begin(), m_entries.end(),
                   [id](VkSpecializationMapEntry const &entry) {
                     return entry.constantID == id;
                   }) != m_entries.end())
    postError(
        Error("Tying to assign duplicate specialization constants. id = " +
              std::to_string(id)));

  auto offset = m_data.size();
  m_data.resize(offset + size);
  memcpy(m_data.data() + offset, constant, size);

  VkSpecializationMapEntry newEntry{};
  newEntry.constantID = id;
  newEntry.size = size;
  newEntry.offset = offset;
  m_entries.push_back(newEntry);

  m_info.mapEntryCount++;
  m_info.pData = m_data.data();
  m_info.pMapEntries = m_entries.data();
}

} // namespace vkw