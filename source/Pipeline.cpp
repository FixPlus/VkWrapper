#include "Pipeline.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

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
} // namespace vkr