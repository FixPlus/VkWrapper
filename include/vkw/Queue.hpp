#ifndef VKRENDERER_QUEUE_HPP
#define VKRENDERER_QUEUE_HPP

#include <vkw/CommandBuffer.hpp>
#include <vkw/Semaphore.hpp>
#include <vkw/SwapChain.hpp>

#include <cassert>
#include <utility>

namespace vkw {

class Surface;

class PresentInfo {
public:
  template <forward_range_of<SwapChain> SWA, forward_range_of<Semaphore> SMA>
  PresentInfo(SWA const &swapChains, SMA const &waitFor)
      : m_swp_ext(decltype(ranges::make_subrange<SwapChain>(swapChains))::get(
                      *(ranges::make_subrange<SwapChain>(swapChains).begin()))
                      .ext()) {
    auto swapChainsSubrange = ranges::make_subrange<SwapChain>(swapChains);
    using swapChainsSubrangeT = decltype(swapChainsSubrange);
    auto waitForSub = ranges::make_subrange<Semaphore>(waitFor);
    using SMASubT = decltype(waitForSub);

    std::transform(swapChainsSubrange.begin(), swapChainsSubrange.end(),
                   std::back_inserter(m_swapChains),
                   [](auto const &swp) -> VkSwapchainKHR {
                     return swapChainsSubrangeT::get(swp);
                   });

    std::transform(swapChainsSubrange.begin(), swapChainsSubrange.end(),
                   std::back_inserter(m_pswapChains),
                   [](auto const &swp) -> SwapChain const * {
                     return swapChainsSubrangeT::get(swp);
                   });
    std::transform(
        waitForSub.begin(), waitForSub.end(),
        std::back_inserter(m_wait_semaphores),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });

    std::transform(swapChainsSubrange.begin(), swapChainsSubrange.end(),
                   std::back_inserter(m_images), [](auto const &swp) {
                     return swapChainsSubrangeT::get(swp).currentImage();
                   });

    m_fill_info();
  }

  template <forward_range_of<Semaphore> SMA =
                boost::container::small_vector<Semaphore, 2>>
  PresentInfo(SwapChain const &swapChain, SMA const &waitFor = {})
      : m_swp_ext(swapChain.extension()) {

    auto waitForSub = ranges::make_subrange<Semaphore>(waitFor);
    using SMASubT = decltype(waitForSub);

    m_swapChains.emplace_back(swapChain);
    m_pswapChains.emplace_back(&swapChain);
    m_images.emplace_back(swapChain.currentImage());

    std::transform(
        waitForSub.begin(), waitForSub.end(),
        std::back_inserter(m_wait_semaphores),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });

    m_fill_info();
  }

  PresentInfo(SwapChain const &swapChain, Semaphore const &waitFor)
      : m_swp_ext(swapChain.extension()) {

    m_swapChains.emplace_back(swapChain);
    m_pswapChains.emplace_back(swapChain);
    m_images.emplace_back(swapChain.currentImage());
    m_wait_semaphores.emplace_back(waitFor);

    m_fill_info();
  }

  PresentInfo(SwapChain const &swapChain) : m_swp_ext(swapChain.extension()) {

    m_swapChains.emplace_back(swapChain);
    m_pswapChains.emplace_back(swapChain);
    m_images.emplace_back(swapChain.currentImage());

    m_fill_info();
  }

  void updateImages() {
    auto swpIt = m_pswapChains.begin();
    auto swpIm = m_images.begin();
    for (; swpIt != m_pswapChains.end(); ++swpIt, ++swpIm) {
      *swpIm = (*swpIt).get().currentImage();
    }
  }

  operator VkPresentInfoKHR() const { return m_info; }

  Extension<ext::KHR_swapchain> const &swapChainExtension() const {
    return m_swp_ext.get();
  }

  PresentInfo(PresentInfo const &another)
      : m_wait_semaphores(another.m_wait_semaphores),
        m_swapChains(another.m_swapChains), m_images(another.m_images),
        m_swp_ext(another.m_swp_ext) {
    m_fill_info();
  }
  PresentInfo(PresentInfo &&another) noexcept = default;

  PresentInfo &operator=(PresentInfo const &another) {
    m_wait_semaphores = another.m_wait_semaphores;
    m_swapChains = another.m_swapChains;
    m_images = another.m_images;
    m_swp_ext = another.m_swp_ext;
    m_fill_info();
    return *this;
  }

  PresentInfo &operator=(PresentInfo &&another) noexcept = default;

  virtual ~PresentInfo() = default;

private:
  void m_fill_info() {
    m_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    m_info.pNext = nullptr;
    m_info.waitSemaphoreCount = m_wait_semaphores.size();
    m_info.pWaitSemaphores = m_wait_semaphores.data();
    m_info.swapchainCount = m_swapChains.size();
    m_info.pSwapchains = m_swapChains.data();
    m_info.pImageIndices = m_images.data();
    m_info.pResults = nullptr;
  }
  boost::container::small_vector<VkSemaphore, 2> m_wait_semaphores;
  boost::container::small_vector<StrongReference<SwapChain const>, 2>
      m_pswapChains;
  boost::container::small_vector<VkSwapchainKHR, 2> m_swapChains;
  boost::container::small_vector<uint32_t, 2> m_images;

  // TODO: rewrite to have a StrongReference or just copy it
  std::reference_wrapper<Extension<ext::KHR_swapchain> const> m_swp_ext;
  VkPresentInfoKHR m_info{};
};

class SubmitInfo {
public:
  template <forward_range_of<PrimaryCommandBuffer const> PCMDA,
            forward_range_of<Semaphore const> SMA,
            forward_range_of<VkPipelineStageFlags const> PSFA =
                boost::container::small_vector<VkPipelineStageFlags, 2>>
  SubmitInfo(PCMDA const &commandBuffer, SMA const &waitFor = {},
             PSFA const &waitTill = {}, SMA const &signalTo = {}) {
    auto commandBufferSub =
        ranges::make_subrange<PrimaryCommandBuffer const>(commandBuffer);
    using commandBufferSubT = decltype(commandBufferSub);

    auto waitForSub = ranges::make_subrange<Semaphore const>(waitFor);
    auto signalToSub = ranges::make_subrange<Semaphore const>(signalTo);
    using semaphoreSubT = decltype(waitForSub);

    auto waitTillSub = ranges::make_subrange<VkPipelineStageFlags>(waitTill);
    using FSFASubT = decltype(waitTillSub);

    std::transform(commandBufferSub.begin(), commandBufferSub.end(),
                   std::back_inserter(m_cmd_buffers),
                   [](auto const &cmd) -> VkCommandBuffer {
                     return commandBufferSubT::get(cmd);
                   });
    std::transform(
        waitForSub.begin(), waitForSub.end(),
        std::back_inserter(m_wait_semaphores),
        [](auto const &smr) -> VkSemaphore { return semaphoreSubT::get(smr); });
    std::transform(
        signalToSub.begin(), signalToSub.end(),
        std::back_inserter(m_signal_semaphores),
        [](auto const &smr) -> VkSemaphore { return semaphoreSubT::get(smr); });

    std::transform(waitTillSub.begin(), waitTillSub.end(),
                   std::back_inserter(m_wait_stage),
                   [](auto const &stage) -> VkPipelineStageFlags {
                     return FSFASubT::get(stage);
                   });

    m_fill_info();
  }

  operator VkSubmitInfo() const { return m_info; }

  SubmitInfo(const PrimaryCommandBuffer &commandBuffer,
             Semaphore const &waitFor, VkPipelineStageFlags waitTill,
             Semaphore const &signalTo)
      : m_wait_stage(1, waitTill), m_cmd_buffers(1, commandBuffer),
        m_wait_semaphores(1, waitFor), m_signal_semaphores(1, signalTo) {
    m_fill_info();
  }

  template <forward_range_of<Semaphore const> SMA =
                boost::container::small_vector<Semaphore, 2>,
            forward_range_of<VkPipelineStageFlags const> PSFA =
                boost::container::small_vector<VkPipelineStageFlags, 2>>
  SubmitInfo(SMA const &waitFor = {}, PSFA const &waitTill = {},
             SMA const &signalTo = {}) {
    auto waitForSub = ranges::make_subrange<Semaphore const>(waitFor);
    auto signalToSub = ranges::make_subrange<Semaphore const>(signalTo);
    auto waitTillSub =
        ranges::make_subrange<VkPipelineStageFlags const>(waitTill);
    using FSFASubT = decltype(waitTillSub);
    using SMASubT = decltype(waitForSub);

    std::transform(
        waitForSub.begin(), waitForSub.end(),
        std::back_inserter(m_wait_semaphores),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });
    std::transform(
        signalToSub.begin(), signalToSub.end(),
        std::back_inserter(m_signal_semaphores),
        [](auto const &smr) -> VkSemaphore { return SMASubT::get(smr); });

    std::transform(waitTillSub.begin(), waitTillSub.end(),
                   std::back_inserter(m_wait_stage),
                   [](auto const &stage) -> VkPipelineStageFlags {
                     return FSFASubT::get(stage);
                   });

    m_fill_info();
  }

  template <forward_range_of<Semaphore const> SMA =
                boost::container::small_vector<Semaphore, 2>,
            forward_range_of<VkPipelineStageFlags const> PSFA =
                boost::container::small_vector<VkPipelineStageFlags, 2>>
  SubmitInfo(const PrimaryCommandBuffer &commandBuffer, SMA const &waitFor = {},
             PSFA const &waitTill = {}, SMA const &signalTo = {})
      : SubmitInfo(waitFor, waitTill, signalTo) {

    m_cmd_buffers.emplace_back(commandBuffer);

    m_info.commandBufferCount = m_cmd_buffers.size();
    m_info.pCommandBuffers = m_cmd_buffers.data();
  }

  SubmitInfo &operator=(SubmitInfo const &another) {
    m_cmd_buffers = another.m_cmd_buffers;
    m_signal_semaphores = another.m_signal_semaphores;
    m_wait_semaphores = another.m_wait_semaphores;
    m_wait_stage = another.m_wait_stage;
    m_fill_info();
    return *this;
  }
  SubmitInfo &operator=(SubmitInfo &&another) noexcept {
    m_cmd_buffers = std::move(another.m_cmd_buffers);
    m_signal_semaphores = std::move(another.m_signal_semaphores);
    m_wait_semaphores = std::move(another.m_wait_semaphores);
    m_wait_stage = std::move(another.m_wait_stage);
    m_fill_info();
    return *this;
  }

  SubmitInfo(SubmitInfo const &another)
      : m_cmd_buffers(another.m_cmd_buffers),
        m_signal_semaphores(another.m_signal_semaphores),
        m_wait_semaphores(another.m_wait_semaphores),
        m_wait_stage(another.m_wait_stage) {
    m_fill_info();
  }
  SubmitInfo(SubmitInfo &&another) noexcept
      : m_cmd_buffers(std::move(another.m_cmd_buffers)),
        m_signal_semaphores(std::move(another.m_signal_semaphores)),
        m_wait_semaphores(std::move(another.m_wait_semaphores)),
        m_wait_stage(std::move(another.m_wait_stage)) {
    m_cmd_buffers = another.m_cmd_buffers;
    m_signal_semaphores = another.m_signal_semaphores;
    m_wait_semaphores = another.m_wait_semaphores;
    m_wait_stage = another.m_wait_stage;
    m_fill_info();
  }

private:
  boost::container::small_vector<VkCommandBuffer, 2> m_cmd_buffers;
  boost::container::small_vector<VkSemaphore, 2> m_signal_semaphores;
  boost::container::small_vector<VkSemaphore, 2> m_wait_semaphores;
  boost::container::small_vector<VkPipelineStageFlags, 2> m_wait_stage;

  void m_fill_info() {
    assert(
        m_wait_stage.size() == m_wait_semaphores.size() &&
        "Count of dst stage masks must be equal to count of wait semaphores");
    m_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    m_info.pNext = nullptr;
    m_info.commandBufferCount = m_cmd_buffers.size();
    m_info.pCommandBuffers = m_cmd_buffers.data();
    m_info.signalSemaphoreCount = m_signal_semaphores.size();
    m_info.pSignalSemaphores = m_signal_semaphores.data();
    m_info.waitSemaphoreCount = m_wait_semaphores.size();
    m_info.pWaitSemaphores = m_wait_semaphores.data();
    m_info.pWaitDstStageMask = m_wait_stage.data();
  }
  VkSubmitInfo m_info{};
};
class Queue {
public:
  bool present(PresentInfo const &presentInfo) const;

  void submit(SubmitInfo const &info) const {
    VkSubmitInfo rawInfo = info;
    m_submit(&rawInfo, 1, nullptr);
  }

  void submit(SubmitInfo const &info, Fence const &fence) const {
    VkSubmitInfo rawInfo = info;
    m_submit(&rawInfo, 1, &fence);
  }

  template <forward_range_of<SubmitInfo const> SubmitRange>
  void submit(SubmitRange const &info, Fence const &fence) const {
    auto infoSubrange = ranges::make_subrange<SubmitInfo const>(info);
    using infoSubrangeT = decltype(infoSubrange);
    boost::container::small_vector<VkSubmitInfo, 3> m_infos;
    std::transform(infoSubrange.begin(), infoSubrange.end(),
                   std::back_inserter(m_infos),
                   [](auto const &info) -> VkSubmitInfo {
                     return infoSubrangeT::get(info);
                   });
    m_submit(m_infos.data(), m_infos.size(), &fence);
  }

  template <forward_range_of<SubmitInfo const> SubmitRange>
  void submit(SubmitRange const &info) const {
    auto infoSubrange = ranges::make_subrange<SubmitInfo const>(info);
    using infoSubrangeT = decltype(infoSubrange);
    boost::container::small_vector<VkSubmitInfo, 3> m_infos;
    std::transform(infoSubrange.begin(), infoSubrange.end(),
                   std::back_inserter(m_infos),
                   [](auto const &info) -> VkSubmitInfo {
                     return infoSubrangeT::get(info);
                   });
    m_submit(m_infos.data(), m_infos.size(), nullptr);
  }

  QueueFamily const &family() const;

  unsigned index() const { return m_queueIndex; }

  bool supportsPresenting(Surface const &surface) const;

  operator VkQueue() const { return m_queue; }

  void waitIdle() const;

private:
  Queue(Device &parent, uint32_t queueFamilyIndex, uint32_t queueIndex);

  friend class Device;

  void m_submit(VkSubmitInfo const *info, size_t infoCount,
                Fence const *fence) const;
  StrongReference<Device> m_parent;
  VkQueue m_queue = VK_NULL_HANDLE;
  uint32_t m_familyIndex;
  uint32_t m_queueIndex;
};
} // namespace vkw
#endif // VKRENDERER_QUEUE_HPP
