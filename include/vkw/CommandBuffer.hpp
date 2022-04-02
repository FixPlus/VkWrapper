#ifndef VKRENDERER_COMMANDBUFFER_HPP
#define VKRENDERER_COMMANDBUFFER_HPP

#include "Common.hpp"
#include "VertexBuffer.hpp"
#include <optional>
#include <vulkan/vulkan.h>

namespace vkw {

class CommandBuffer {
public:
  CommandBuffer(CommandBuffer &&another) noexcept
      : m_device(another.m_device), m_pool(another.m_pool),
        m_executable(another.m_executable), m_recording(another.m_recording) {
    another.m_commandBuffer = VK_NULL_HANDLE;
  };

  CommandBuffer &operator=(CommandBuffer &&another) noexcept {
    m_device = another.m_device;
    m_pool = another.m_pool;
    m_executable = another.m_executable;
    m_recording = another.m_recording;
    std::swap(m_commandBuffer, another.m_commandBuffer);
    return *this;
  }

  uint32_t queueFamily() const;

  virtual ~CommandBuffer() = default;

  operator VkCommandBuffer() const { return m_commandBuffer; }

  bool isInRecordState() const { return m_recording; }

  bool isInExecutableState() const { return m_executable; }

  /** Transfer */

  void copyBufferToBuffer(BufferBase const &src, BufferBase const &dst,
                          std::vector<VkBufferCopy> const &regions);
  void copyBufferToImage(BufferBase const &src, AllocatedImage const &dst,
                         VkImageLayout layout,
                         std::vector<VkBufferImageCopy> const &regions);
  void copyImageToBuffer(AllocatedImage const &src, VkImageLayout layout,
                         BufferBase const &dst,
                         std::vector<VkBufferImageCopy> const &regions);

  void copyImageToImage(AllocatedImage const &src, VkImageLayout srcLayout,
                        AllocatedImage const &dst, VkImageLayout dstLayout,
                        std::vector<VkImageCopy> const &regions);

  /** Synchronization */
  void
  pipelineBarrier(VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
                  std::vector<VkMemoryBarrier> const &memBarriers,
                  std::vector<VkImageMemoryBarrier> const &imageMemoryBarrier,
                  std::vector<VkBufferMemoryBarrier> const &bufferMemoryBarrier,
                  VkDependencyFlags flags = 0);

  void imageMemoryBarrier(
      VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
      std::vector<VkImageMemoryBarrier> const &imageMemoryBarrier,
      VkDependencyFlags flags = 0) {
    pipelineBarrier(srcStage, dstStage, {}, imageMemoryBarrier, {}, flags);
  }
  void bufferMemoryBarrier(
      VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage,
      std::vector<VkBufferMemoryBarrier> const &bufferMemoryBarrier,
      VkDependencyFlags flags = 0) {
    pipelineBarrier(srcStage, dstStage, {}, {}, bufferMemoryBarrier, flags);
  }
  void memoryBarrier(VkPipelineStageFlags srcStage,
                     VkPipelineStageFlags dstStage,
                     std::vector<VkMemoryBarrier> const &memBarriers,
                     VkDependencyFlags flags = 0) {
    pipelineBarrier(srcStage, dstStage, memBarriers, {}, {}, flags);
  }

  /** Binding operations */

  template <typename T>
  void bindVertexBuffer(VertexBuffer<T> const &vbuf, uint32_t binding,
                        VkDeviceSize offset) {
    m_bindVertexBuffer(static_cast<BufferBase const &>(vbuf), binding, offset);
  }

  template <VkIndexType type>
  void bindIndexBuffer(IndexBuffer<type> const &ibuf, VkDeviceSize offset) {
    m_bindIndexBuffer(static_cast<BufferBase const &>(ibuf), type, offset);
  }

  void bindGraphicsPipeline(GraphicsPipeline const &pipeline);
  void bindComputePipeline(ComputePipeline const &pipeline);

  void bindDescriptorSets(PipelineLayout const &layout,
                          VkPipelineBindPoint bindPoint,
                          DescriptorSetConstRefArray sets, uint32_t firstSet);

  template <typename T>
  void pushConstants(PipelineLayout const &layout,
                     VkShaderStageFlagBits shaderStage, uint32_t offset,
                     T const &constant) {
    m_pushConstants(layout, shaderStage, offset, sizeof(constant), &constant);
  }
  /** Draw commands */

  void draw(uint32_t vertexCount, uint32_t instanceCount = 0,
            uint32_t firstVertex = 0, uint32_t firstInstance = 0);
  void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 0,
                   uint32_t firstIndex = 0, int32_t vertexOffset = 0,
                   uint32_t firstInstance = 0);

  /** Dispatch commands */

  void dispatch(uint32_t groupCountX, uint32_t groupCountY,
                uint32_t groupCountZ);

  /** Pipeline dynamic state sets */

  void setScissors(std::vector<VkRect2D> const &scissors,
                   uint32_t firstScissor = 0);
  void setViewports(std::vector<VkViewport> const &viewports,
                    uint32_t firstViewport = 0);

  /** Finishing */

  void end();

protected:
  CommandBuffer(CommandPool &pool, VkCommandBufferLevel bufferLevel);
  DeviceRef m_device;
  std::reference_wrapper<CommandPool> m_pool;
  VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
  void m_begin(VkCommandBufferUsageFlags flags,
               VkCommandBufferInheritanceInfo const *inheritanceInfo);

private:
  void m_pushConstants(PipelineLayout const &layout,
                       VkShaderStageFlagBits shaderStage, uint32_t offset,
                       uint32_t size, const void *data);
  void m_bindVertexBuffer(VkBuffer buffer, uint32_t binding,
                          VkDeviceSize offset);
  void m_bindIndexBuffer(VkBuffer buffer, VkIndexType type,
                         VkDeviceSize offset);
  bool m_recording = false;
  bool m_executable = false;
};

class SecondaryCommandBuffer : public CommandBuffer {
public:
  explicit SecondaryCommandBuffer(CommandPool &pool)
      : CommandBuffer(pool, VK_COMMAND_BUFFER_LEVEL_SECONDARY){};

  void begin(VkCommandBufferUsageFlags flags,
             VkCommandBufferInheritanceInfo const *inheritanceInfo = nullptr) {
    m_begin(flags, inheritanceInfo);
  }
};

class PrimaryCommandBuffer : public CommandBuffer {
public:
  explicit PrimaryCommandBuffer(CommandPool &pool)
      : CommandBuffer(pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY){};

  void beginRenderPass(RenderPass const &renderPass,
                       FrameBuffer const &frameBuffer, VkRect2D renderArea,
                       bool useSecondary = false, uint32_t clearValuesCount = 0,
                       VkClearValue *pClearValues = nullptr);

  void nextSubpass(bool useSecondary = false);

  void executeCommands(SecondaryCommandBufferConstRefArray const &commands);

  void endRenderPass();

  void begin(VkCommandBufferUsageFlags flags) {

    // According to vulkan specification primary command buffers don't use
    // VkCommandBufferInheritanceInfo

    m_begin(flags, nullptr);
  }

private:
  std::optional<RenderPassCRef> m_currentPass;
  uint32_t m_currentSubpass;
};

} // namespace vkw
#endif // VKRENDERER_COMMANDBUFFER_HPP
