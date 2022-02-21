#include "CommandBuffer.hpp"
#include "Buffer.hpp"
#include "CommandPool.hpp"
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

  VK_CHECK_RESULT(
      vkAllocateCommandBuffers(m_device, &allocInfo, &m_commandBuffer))
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

  VK_CHECK_RESULT(vkBeginCommandBuffer(m_commandBuffer, &beginInfo))

  m_recording = true;
}

void CommandBuffer::end() {
  VK_CHECK_RESULT(vkEndCommandBuffer(m_commandBuffer))
  m_recording = false;
  m_executable = true;
}

void CommandBuffer::copyBufferToBuffer(
    const BufferBase &src, const BufferBase &dst,
    const std::vector<VkBufferCopy> &regions) {
  vkCmdCopyBuffer(m_commandBuffer, src, dst, regions.size(), regions.data());
}

void CommandBuffer::copyBufferToImage(
    const BufferBase &src, const ImageBase &dst, VkImageLayout layout,
    const std::vector<VkBufferImageCopy> &regions) {
  vkCmdCopyBufferToImage(m_commandBuffer, src, dst, layout, regions.size(),
                         regions.data());
}

void CommandBuffer::pipelineBarrier(
    VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
    const std::vector<VkMemoryBarrier> &memBarriers,
    const std::vector<VkImageMemoryBarrier> &imageMemoryBarrier,
    const std::vector<VkBufferMemoryBarrier> &bufferMemoryBarrier,
    VkDependencyFlags flags) {
  vkCmdPipelineBarrier(m_commandBuffer, srcStage, dstStage, flags,
                       memBarriers.size(), memBarriers.data(),
                       bufferMemoryBarrier.size(), bufferMemoryBarrier.data(),
                       imageMemoryBarrier.size(), imageMemoryBarrier.data());
}

void CommandBuffer::m_bindVertexBuffer(VkBuffer buffer, uint32_t binding,
                                       VkDeviceSize offset) {
  vkCmdBindVertexBuffers(m_commandBuffer, binding, 1, &buffer, &offset);
}

void CommandBuffer::m_bindIndexBuffer(VkBuffer buffer, VkIndexType type,
                                      VkDeviceSize offset) {
  vkCmdBindIndexBuffer(m_commandBuffer, buffer, offset, type);
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount,
                         uint32_t firstVertex, uint32_t firstInstance) {
  vkCmdDraw(m_commandBuffer, vertexCount, instanceCount, firstVertex,
            firstInstance);
}

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                uint32_t firstIndex, int32_t vertexOffset,
                                uint32_t firstInstance) {
  vkCmdDrawIndexed(m_commandBuffer, indexCount, instanceCount, firstIndex,
                   vertexOffset, firstInstance);
}

void CommandBuffer::bindGraphicsPipeline(GraphicsPipeline const &pipeline) {
  vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::bindComputePipeline(ComputePipeline const &pipeline) {
  vkCmdBindPipeline(m_commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}
void CommandBuffer::copyImageToBuffer(
    ImageBase const &src, VkImageLayout layout, BufferBase const &dst,
    std::vector<VkBufferImageCopy> const &regions) {
  vkCmdCopyImageToBuffer(m_commandBuffer, src, layout, dst, regions.size(),
                         regions.data());
}
void CommandBuffer::setScissors(std::vector<VkRect2D> const &scissors,
                                uint32_t firstScissor) {
  vkCmdSetScissor(m_commandBuffer, firstScissor, scissors.size(),
                  scissors.data());
}
void CommandBuffer::setViewports(std::vector<VkViewport> const &viewports,
                                 uint32_t firstViewport) {
  vkCmdSetViewport(m_commandBuffer, firstViewport, viewports.size(),
                   viewports.data());
}

void PrimaryCommandBuffer::beginRenderPass(const RenderPass &renderPass,
                                           const FrameBuffer &frameBuffer,
                                           VkRect2D renderArea,
                                           bool useSecondary,
                                           uint32_t clearValuesCount,
                                           VkClearValue *pClearValues) {
  VkRenderPassBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  beginInfo.pNext = nullptr;
  beginInfo.renderPass = renderPass;
  beginInfo.framebuffer = frameBuffer;
  beginInfo.renderArea = renderArea;
  beginInfo.clearValueCount = clearValuesCount;
  beginInfo.pClearValues = pClearValues;

  vkCmdBeginRenderPass(m_commandBuffer, &beginInfo,
                       useSecondary
                           ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                           : VK_SUBPASS_CONTENTS_INLINE);
}

void PrimaryCommandBuffer::nextSubpass(bool useSecondary) {
  vkCmdNextSubpass(m_commandBuffer,
                   useSecondary ? VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
                                : VK_SUBPASS_CONTENTS_INLINE);
}

void PrimaryCommandBuffer::endRenderPass() {
  vkCmdEndRenderPass(m_commandBuffer);
}

void PrimaryCommandBuffer::executeCommands(
    SecondaryCommandBufferConstRefArray const &commands) {
  vkCmdExecuteCommands(m_commandBuffer, commands.size(), commands);
}
} // namespace vkw