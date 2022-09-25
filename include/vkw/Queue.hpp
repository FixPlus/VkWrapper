#ifndef VKRENDERER_QUEUE_HPP
#define VKRENDERER_QUEUE_HPP

#include "CommandBuffer.hpp"
#include "Common.hpp"
#include "Semaphore.hpp"
#include "SwapChain.hpp"
#include <cassert>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

class Queue {
public:
  template <forward_range_of<SwapChain> SWA, forward_range_of<Semaphore> SMA>
  bool present(SWA const &swapChains, SMA const &waitFor = {}) {
    VkPresentInfoKHR presentInfo{};
    auto swapChainsSubrange = ranges::make_subrange<SwapChain>(swapChains);
    using swapChainsSubrangeT = decltype(swapChainsSubrange);
    auto waitForSub = ranges::make_subrange<Semaphore>(waitFor);
    using SMASubT = decltype(waitForSub);

    std::vector<VkSemaphore> sems; // TODO: make this small vectors
    std::vector<VkSwapchainKHR> swps;

    std::transform(swapChainsSubrange.begin(), swapChainsSubrange.end(),
                   std::back_inserter(swps),
                   [](auto const &swp) -> VkSwapchainKHR {
                     return swapChainsSubrangeT::get(swp);
                   });
    std::transform(
        waitForSub.begin(), waitForSub.end(), std::back_inserter(sems),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = sems.size();
    presentInfo.pWaitSemaphores = sems.data();
    presentInfo.swapchainCount = swps.size();
    presentInfo.pSwapchains = swps.data();

    std::vector<uint32_t> images;

    images.reserve(swapChains.size());

    for (auto &swapChain : swapChains) {
      images.push_back(swapChain.currentImage());
    }
    presentInfo.pImageIndices = images.data();
    presentInfo.pResults = nullptr;

    return m_present(&presentInfo, swapChains.begin()->ext());
  }

  template <forward_range_of<Semaphore> SMA = std::vector<Semaphore>>
  bool present(SwapChain const &swapChain, SMA const &waitFor = {}) {

    auto waitForSub = ranges::make_subrange<Semaphore>(waitFor);
    using SMASubT = decltype(waitForSub);

    VkPresentInfoKHR presentInfo{};
    std::vector<VkSemaphore> sems; // TODO: make this small vectors
    VkSwapchainKHR swp = swapChain;

    std::transform(
        waitForSub.begin(), waitForSub.end(), std::back_inserter(sems),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = sems.size();
    presentInfo.pWaitSemaphores = sems.data();
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swp;

    uint32_t image = swapChain.currentImage();

    presentInfo.pImageIndices = &image;
    presentInfo.pResults = nullptr;

    return m_present(&presentInfo, swapChain.ext());
  }

  bool present(SwapChain const &swapChain, Semaphore const &waitFor) {
    VkPresentInfoKHR presentInfo{};
    VkSemaphore sem = waitFor;
    VkSwapchainKHR swp = swapChain;

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &sem;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swp;

    uint32_t image = swapChain.currentImage();

    presentInfo.pImageIndices = &image;
    presentInfo.pResults = nullptr;

    return m_present(&presentInfo, swapChain.ext());
  }

  bool present(SwapChain const &swapChain) {
    VkPresentInfoKHR presentInfo{};
    VkSwapchainKHR swp = swapChain;

    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.waitSemaphoreCount = 0;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swp;

    uint32_t image = swapChain.currentImage();

    presentInfo.pImageIndices = &image;
    presentInfo.pResults = nullptr;

    return m_present(&presentInfo, swapChain.ext());
  }

  template <forward_range_of<PrimaryCommandBuffer> PCMDA,
            forward_range_of<Semaphore> SMA>
  void submit(PCMDA const &commandBuffer, SMA const &waitFor = {},
              std::vector<VkPipelineStageFlags> const &waitTill = {},
              SMA const &signalTo = {}, Fence const *fence = nullptr) {
    auto commandBufferSub =
        ranges::make_subrange<PrimaryCommandBuffer>(commandBuffer);
    using commandBufferSubT = decltype(commandBufferSub);

    auto waitForSub = ranges::make_subrange<Semaphore>(waitFor);
    auto signalToSub = ranges::make_subrange<Semaphore>(signalTo);
    using semaphoreSubT = decltype(waitForSub);

    assert(
        waitTill.size() == waitFor.size() &&
        "Count of dst stage masks must be equal to count of wait semaphores");

    std::vector<VkSemaphore> rawWaitFor,
        rawSignalTo; // TODO: make this small vectors
    std::vector<VkCommandBuffer> rawCmdBufs;

    std::transform(commandBufferSub.begin(), commandBufferSub.end(),
                   [](auto const &cmd) -> VkCommandBuffer {
                     return commandBufferSubT::get(cmd);
                   });
    std::transform(
        waitForSub.begin(), waitForSub.end(), std::back_inserter(rawWaitFor),
        [](auto const &smr) -> VkSemaphore { return semaphoreSubT::get(smr); });
    std::transform(
        signalToSub.begin(), signalToSub.end(), std::back_inserter(rawSignalTo),
        [](auto const &smr) -> VkSemaphore { return semaphoreSubT::get(smr); });

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = rawCmdBufs.size();
    submitInfo.pCommandBuffers = rawCmdBufs.data();
    submitInfo.signalSemaphoreCount = rawSignalTo.size();
    submitInfo.pSignalSemaphores = rawSignalTo.data();
    submitInfo.waitSemaphoreCount = rawWaitFor.size();
    submitInfo.pWaitSemaphores = rawWaitFor.data();
    submitInfo.pWaitDstStageMask = waitTill.data();

    m_submit(&submitInfo, fence);
  }

  template <forward_range_of<Semaphore> SMA = std::vector<Semaphore>>
  void submit(const PrimaryCommandBuffer &commandBuffer,
              SMA const &waitFor = {},
              std::vector<VkPipelineStageFlags> const &waitTill = {},
              SMA const &signalTo = {}, Fence const *fence = nullptr) {
    auto waitForSub = ranges::make_subrange<Semaphore>(waitFor);
    auto signalToSub = ranges::make_subrange<Semaphore>(signalTo);
    using SMASubT = decltype(waitForSub);

    assert(
        waitTill.size() == waitFor.size() &&
        "Count of dst stage masks must be equal to count of wait semaphores");

    std::vector<VkSemaphore> rawWaitFor,
        rawSignalTo; // TODO: make this small vectors
    VkCommandBuffer rawCmdBuf = commandBuffer;
    std::transform(
        waitForSub.begin(), waitForSub.end(), std::back_inserter(rawWaitFor),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });
    std::transform(
        signalTo.begin(), signalTo.end(), std::back_inserter(rawSignalTo),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &rawCmdBuf;
    submitInfo.signalSemaphoreCount = rawSignalTo.size();
    submitInfo.pSignalSemaphores = rawSignalTo.data();
    submitInfo.waitSemaphoreCount = rawWaitFor.size();
    submitInfo.pWaitSemaphores = rawWaitFor.data();
    submitInfo.pWaitDstStageMask = waitTill.data();

    m_submit(&submitInfo, fence);
  }

  void submit(const PrimaryCommandBuffer &commandBuffer,
              Semaphore const &waitFor, VkPipelineStageFlags waitTill,
              Semaphore const &signalTo, Fence const *fence = nullptr) {
    assert(
        waitTill.size() == waitFor.size() &&
        "Count of dst stage masks must be equal to count of wait semaphores");

    VkSemaphore rawWaitFor, rawSignalTo;
    rawWaitFor = waitFor;
    rawSignalTo = signalTo;
    VkCommandBuffer rawCmdBuf = commandBuffer;

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &rawCmdBuf;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &rawSignalTo;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &rawWaitFor;
    submitInfo.pWaitDstStageMask = &waitTill;

    m_submit(&submitInfo, fence);
  }

  template <forward_range_of<Semaphore> SMA = std::vector<Semaphore>>
  void submit(SMA const &waitFor = {},
              std::vector<VkPipelineStageFlags> const &waitTill = {},
              SMA const &signalTo = {}, Fence const *fence = nullptr) {
    auto waitForSub = ranges::make_subrange<Semaphore>(waitFor);
    auto signalToSub = ranges::make_subrange<Semaphore>(signalTo);
    using SMASubT = decltype(waitForSub);

    assert(
        waitTill.size() == waitFor.size() &&
        "Count of dst stage masks must be equal to count of wait semaphores");

    std::vector<VkSemaphore> rawWaitFor,
        rawSignalTo; // TODO: make this small vectors
    std::vector<VkCommandBuffer> rawCmdBufs;

    std::transform(
        waitForSub.begin(), waitForSub.end(), std::back_inserter(rawWaitFor),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });
    std::transform(
        signalToSub.begin(), signalToSub.end(), std::back_inserter(rawSignalTo),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 0;
    submitInfo.signalSemaphoreCount = rawSignalTo.size();
    submitInfo.pSignalSemaphores = rawSignalTo.data();
    submitInfo.waitSemaphoreCount = rawWaitFor.size();
    submitInfo.pWaitSemaphores = rawWaitFor.data();
    submitInfo.pWaitDstStageMask = waitTill.data();
    m_submit(&submitInfo, fence);
  }
  uint32_t familyIndex() const { return m_familyIndex; }

  bool supportsGraphics() const { return m_flags & VK_QUEUE_GRAPHICS_BIT; }
  bool supportsTransfer() const { return m_flags & VK_QUEUE_TRANSFER_BIT; }
  bool supportsPresenting(Surface const &surface) const;

  operator VkQueue() const { return m_queue; }

  void waitIdle();

private:
  Queue(Device &parent, uint32_t queueFamilyIndex, uint32_t queueIndex);

  friend class Device;

  bool m_present(VkPresentInfoKHR const *presentInfo,
                 Extension<ext::KHR_swapchain> const &swpExtension);
  void m_submit(VkSubmitInfo const *info, Fence const *fence);
  DeviceRef m_parent;
  VkQueue m_queue;
  VkQueueFlags m_flags;
  uint32_t m_familyIndex;
};
} // namespace vkw
#endif // VKRENDERER_QUEUE_HPP
