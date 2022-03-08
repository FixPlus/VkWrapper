#include "CommandBuffer.hpp"
#include "Buffer.hpp"
#include "CommandPool.hpp"
#include "DescriptorSet.hpp"
#include "Device.hpp"
#include "FrameBuffer.hpp"
#include "Image.hpp"
#include "Pipeline.hpp"
#include "RenderPass.hpp"
#include "Utils.hpp"

namespace vkw {

CommandBuffer::CommandBuffer(CommandPool &pool,
                             VkCommandBufferLevel bufferLevel)
    : m_pool(pool), m_device(pool.getParent()) {
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.pNext = nullptr;
  allocInfo.commandBufferCount = 1;
  allocInfo.commandPool = pool;

  VK_CHECK_RESULT(m_device.core<1, 0>().vkAllocateCommandBuffers(
      m_device, &allocInfo, &m_commandBuffer))
}

uint32_t CommandBuffer::queueFamily() const {
  return m_pool.queueFamilyIndex();
}

void CommandBuffer::m_begin(
    VkCommandBufferUsageFlags flags,
    const VkCommandBufferInheritanceInfo *inheritanceInfo) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = flags;
  beginInfo.pInheritanceInfo = inheritanceInfo;
  beginInfo.pNext = nullptr;

  VK_CHECK_RESULT(
      m_device.core<1, 0>().vkBeginCommandBuffer(m_commandBuffer, &beginInfo))

  m_recording = true;
}

void CommandBuffer::end() {
  VK_CHECK_RESULT(m_device.core<1, 0>().vkEndCommandBuffer(m_commandBuffer))
  m_recording = false;
  m_executable = true;
}

void CommandBuffer::copyBufferToBuffer(
    const BufferBase &src, const BufferBase &dst,
    const std::vector<VkBufferCopy> &regions) {
  m_device.core<1, 0>().vkCmdCopyBuffer(m_commandBuffer, src, dst,
                                        regions.size(), regions.data());
}

void CommandBuffer::copyBufferToImage(
    const BufferBase &src, const AllocatedImage &dst, VkImageLayout layout,
    const std::vector<VkBufferImageCopy> &regions) {
  m_device.core<1, 0>().vkCmdCopyBufferToImage(
      m_commandBuffer, src, dst, layout, regions.size(), regions.data());
}

void CommandBuffer::copyImageToImage(AllocatedImage const &src,
                                     VkImageLayout srcLayout,
                                     AllocatedImage const &dst,
                                     VkImageLayout dstLayout,
                                     std::vector<VkImageCopy> const &regions) {
  m_device.core<1, 0>().vkCmdCopyImage(m_commandBuffer, src, srcLayout, dst,
                                       dstLayout, regions.size(),
                                       regions.data());
}

void CommandBuffer::pipelineBarrier(
    VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
    const std::vector<VkMemoryBarrier> &memBarriers,
    const std::vector<VkImageMemoryBarrier> &imageMemoryBarrier,
    const std::vector<VkBufferMemoryBarrier> &bufferMemoryBarrier,
    VkDependencyFlags flags) {
  m_device.core<1, 0>().vkCmdPipelineBarrier(
      m_commandBuffer, srcStage, dstStage, flags, memBarriers.size(),
      memBarriers.data(), bufferMemoryBarrier.size(),
      bufferMemoryBarrier.data(), imageMemoryBarrier.size(),
      imageMemoryBarrier.data());
}

void CommandBuffer::m_bindVertexBuffer(VkBuffer buffer, uint32_t binding,
                                       VkDeviceSize offset) {
  m_device.core<1, 0>().vkCmdBindVertexBuffers(m_commandBuffer, binding, 1,
                                               &buffer, &offset);
}

void CommandBuffer::m_bindIndexBuffer(VkBuffer buffer, VkIndexType type,
                                      VkDeviceSize offset) {
  m_device.core<1, 0>().vkCmdBindIndexBuffer(m_commandBuffer, buffer, offset,
                                             type);
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount,
                         uint32_t firstVertex, uint32_t firstInstance) {
  m_device.core<1, 0>().vkCmdDraw(m_commandBuffer, vertexCount, instanceCount,
                                  firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                uint32_t firstIndex, int32_t vertexOffset,
                                uint32_t firstInstance) {
  m_device.core<1, 0>().vkCmdDrawIndexed(m_commandBuffer, indexCount,
                                         instanceCount, firstIndex,
                                         vertexOffset, firstInstance);
}

void CommandBuffer::bindGraphicsPipeline(GraphicsPipeline const &pipeline) {
  m_device.core<1, 0>().vkCmdBindPipeline(
      m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::bindComputePipeline(ComputePipeline const &pipeline) {
  m_device.core<1, 0>().vkCmdBindPipeline(
      m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}
void CommandBuffer::copyImageToBuffer(
    AllocatedImage const &src, VkImageLayout layout, BufferBase const &dst,
    std::vector<VkBufferImageCopy> const &regions) {
  m_device.core<1, 0>().vkCmdCopyImageToBuffer(
      m_commandBuffer, src, layout, dst, regions.size(), regions.data());
}
void CommandBuffer::setScissors(std::vector<VkRect2D> const &scissors,
                                uint32_t firstScissor) {
  m_device.core<1, 0>().vkCmdSetScissor(m_commandBuffer, firstScissor,
                                        scissors.size(), scissors.data());
}
void CommandBuffer::setViewports(std::vector<VkViewport> const &viewports,
                                 uint32_t firstViewport) {
  m_device.core<1, 0>().vkCmdSetViewport(m_commandBuffer, firstViewport,
                                         viewports.size(), viewports.data());
}
void CommandBuffer::bindDescriptorSets(const PipelineLayout &layout,
                                       VkPipelineBindPoint bindPoint,
                                       DescriptorSetConstRefArray sets,
                                       uint32_t firstSet) {
  std::vector<uint32_t> dynamicOffsets{};
  for (auto const &set : sets) {
    auto offsetCount = set.get().dynamicOffsetsCount();
    if (offsetCount == 0)
      continue;
    auto cachedSize = dynamicOffsets.size();
    dynamicOffsets.resize(cachedSize + offsetCount);
    set.get().copyOffsets(dynamicOffsets.data() + cachedSize);
  }
  m_device.core<1, 0>().vkCmdBindDescriptorSets(
      m_commandBuffer, bindPoint, layout, firstSet, sets.size(), sets,
      dynamicOffsets.size(), dynamicOffsets.data());
}

void PrimaryCommandBuffer::beginRenderPass(const RenderPass &renderPass,
                                           const FrameBuffer &frameBuffer,
                                           VkRect2D renderArea,
                                           bool useSecondary,
                                           uint32_t clearValuesCount,
                                           VkClearValue *pClearValues) {
  if (m_currentPass.has_value())
    throw Error("CommandBuffer record failed: called beginRenderPass() while "
                "having another RenderPass active");

  VkRenderPassBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.pNext = nullptr;
  beginInfo.renderPass = renderPass;
  beginInfo.framebuffer = frameBuffer;
  beginInfo.renderArea = renderArea;
  beginInfo.clearValueCount = clearValuesCount;
  beginInfo.pClearValues = pClearValues;

  m_device.core<1, 0>().vkCmdBeginRenderPass(
      m_commandBuffer, &beginInfo,
      useSecondary ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                   : VK_SUBPASS_CONTENTS_INLINE);
  m_currentPass = renderPass;
  m_currentSubpass = 0;
}

void PrimaryCommandBuffer::nextSubpass(bool useSecondary) {
  if (!m_currentPass.has_value())
    throw Error("CommandBuffer record failed: called nextSubpass() while "
                "having no RenderPass active");
  if (m_currentPass.value().get().info().subpassCount() == m_currentSubpass)
    throw Error("CommandBuffer record failed: call to nextSubpass() caused "
                "subpass overflow");

  m_device.core<1, 0>().vkCmdNextSubpass(
      m_commandBuffer, useSecondary
                           ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                           : VK_SUBPASS_CONTENTS_INLINE);
  m_currentSubpass++;
}

void PrimaryCommandBuffer::endRenderPass() {
  if (!m_currentPass.has_value())
    throw Error("CommandBuffer record failed: called endRenderPass() while "
                "having no RenderPass active");
  m_device.core<1, 0>().vkCmdEndRenderPass(m_commandBuffer);
  m_currentPass.reset();
}

void PrimaryCommandBuffer::executeCommands(
    SecondaryCommandBufferConstRefArray const &commands) {
  m_device.core<1, 0>().vkCmdExecuteCommands(m_commandBuffer, commands.size(),
                                             commands);
}
} // namespace vkw