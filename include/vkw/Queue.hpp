#ifndef VKRENDERER_QUEUE_HPP
#define VKRENDERER_QUEUE_HPP

#include "Common.hpp"
#include <vulkan/vulkan.h>

namespace vkw {

class Queue {
public:
  bool present(SwapChainConstRefArray const &swapChains,
               SemaphoreConstRefArray const &waitFor = {});

  bool present(SwapChain const &swapChain,
               SemaphoreConstRefArray const &waitFor = {});

  void submit(PrimaryCommandBufferConstRefArray const &commandBuffer,
              SemaphoreConstRefArray const &waitFor = {},
              std::vector<VkPipelineStageFlags> const &waitTill = {},
              SemaphoreConstRefArray const &signalTo = {},
              Fence const *fence = nullptr);

  uint32_t familyIndex() const { return m_familyIndex; }

  bool supportsGraphics() const { return m_flags & VK_QUEUE_GRAPHICS_BIT; }
  bool supportsTransfer() const { return m_flags & VK_QUEUE_TRANSFER_BIT; }
  bool supportsPresenting(Surface const &surface) const;

  operator VkQueue() const { return m_queue; }

  void waitIdle();

private:
  Queue(Device &parent, uint32_t queueFamilyIndex, uint32_t queueIndex);

  friend class Device;

  DeviceRef m_parent;
  VkQueue m_queue;
  VkQueueFlags m_flags;
  uint32_t m_familyIndex;
};
} // namespace vkw
#endif // VKRENDERER_QUEUE_HPP
