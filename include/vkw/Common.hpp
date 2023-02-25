#ifndef VKRENDERER_COMMON_HPP
#define VKRENDERER_COMMON_HPP

#include <ranges>
#include <vkw/ReferenceGuard.hpp>
#include <vulkan/vulkan.h>

namespace vkw {

/** Predeclarations. */

class AttachmentDescription;
class BufferBase;
class CommandBuffer;
class SecondaryCommandBuffer;
class PrimaryCommandBuffer;
class CommandPool;
class DescriptorPool;
class DescriptorSet;
class DescriptorSetLayout;
class DescriptorSetLayoutBinding;
class Device;
class Fence;
class FrameBuffer;
class AllocatedImage;
class ImageInterface;

class SwapChainImage;
class ImageViewBase;

class Instance;
class PhysicalDevice;
class Pipeline;
class PipelineCache;
class PipelineLayout;
class GraphicsPipeline;
class ComputePipeline;
class Queue;
class RenderPass;
class Semaphore;
class ShaderBase;
class VertexShader;
class FragmentShader;
class ComputeShader;
class Sampler;
class SubpassDependency;
class SubpassDescription;
class Surface;
class SwapChain;

template <typename T> struct TypeTraits {};

template <> struct TypeTraits<AttachmentDescription> {
  using VType = VkAttachmentDescription;
};

template <> struct TypeTraits<BufferBase> { using VType = VkBuffer; };

template <> struct TypeTraits<CommandBuffer> { using VType = VkCommandBuffer; };

template <> struct TypeTraits<SecondaryCommandBuffer> {
  using VType = VkCommandBuffer;
};

template <> struct TypeTraits<PrimaryCommandBuffer> {
  using VType = VkCommandBuffer;
};

template <> struct TypeTraits<CommandPool> { using VType = VkCommandPool; };

template <> struct TypeTraits<DescriptorPool> {
  using VType = VkDescriptorPool;
};

template <> struct TypeTraits<DescriptorSet> { using VType = VkDescriptorSet; };

template <> struct TypeTraits<DescriptorSetLayout> {
  using VType = VkDescriptorSetLayout;
};

template <> struct TypeTraits<DescriptorSetLayoutBinding> {
  using VType = VkDescriptorSetLayoutBinding;
};

template <> struct TypeTraits<Device> { using VType = VkDevice; };

template <> struct TypeTraits<Fence> { using VType = VkFence; };

template <> struct TypeTraits<FrameBuffer> { using VType = VkFramebuffer; };

template <> struct TypeTraits<AllocatedImage> { using VType = VkImage; };

template <> struct TypeTraits<ImageInterface> { using VType = VkImage; };

template <> struct TypeTraits<SwapChainImage> { using VType = VkImage; };

template <> struct TypeTraits<ImageViewBase> { using VType = VkImageView; };

template <> struct TypeTraits<Instance> { using VType = VkInstance; };

template <> struct TypeTraits<PhysicalDevice> {
  using VType = VkPhysicalDevice;
};

template <> struct TypeTraits<Pipeline> { using VType = VkPipeline; };

template <> struct TypeTraits<PipelineCache> { using VType = VkPipelineCache; };

template <> struct TypeTraits<PipelineLayout> {
  using VType = VkPipelineLayout;
};

template <> struct TypeTraits<GraphicsPipeline> { using VType = VkPipeline; };

template <> struct TypeTraits<ComputePipeline> { using VType = VkPipeline; };

template <> struct TypeTraits<Queue> { using VType = VkQueue; };

template <> struct TypeTraits<RenderPass> { using VType = VkRenderPass; };

template <> struct TypeTraits<Sampler> { using VType = VkSampler; };

template <> struct TypeTraits<Semaphore> { using VType = VkSemaphore; };

template <> struct TypeTraits<ShaderBase> { using VType = VkShaderModule; };

template <> struct TypeTraits<FragmentShader> { using VType = VkShaderModule; };

template <> struct TypeTraits<ComputeShader> { using VType = VkShaderModule; };

template <> struct TypeTraits<VertexShader> { using VType = VkShaderModule; };

template <> struct TypeTraits<Surface> { using VType = VkSurfaceKHR; };

template <> struct TypeTraits<SwapChain> { using VType = VkSwapchainKHR; };

template <typename T, typename U>
concept derived_or_reference_wrapped =
    std::same_as < typename std::remove_cv<T>::type,
typename std::remove_cv<U>::type > || std::derived_from<T, U> ||
    std::same_as<std::reference_wrapper<U>, T> ||
    std::same_as<std::reference_wrapper<const U>,
                 typename std::remove_cv<T>::type>;

template <typename T, typename U>
concept forward_range_of = std::ranges::forward_range<T> &&
    derived_or_reference_wrapped<std::ranges::range_value_t<T>, U>;

namespace ranges {

  template <typename T, typename U>
  requires derived_or_reference_wrapped<T, U>
  struct raw_ref {
    constexpr static U &get(T &a) { return a; }
  };

  template <typename U> struct raw_ref<std::reference_wrapper<U>, U> {
    constexpr static U &get(std::reference_wrapper<U> a) { return a.get(); }
  };

  template <typename U> struct raw_ref<std::reference_wrapper<const U>, U> {
    constexpr static U const &get(std::reference_wrapper<const U> a) {
      return a.get();
    }
  };

  // Disclaimer: I don't use std::ranges::subrange in order this code compile on
  // older compilers

  template <typename Base, forward_range_of<Base> R> class subrange {
  public:
    using iterator = decltype(std::ranges::begin(std::declval<R const &>()));
    using value_type = std::ranges::range_value_t<R>;

    subrange(R const &r)
        : m_begin(std::ranges::begin(r)), m_end(std::ranges::end(r)) {}

    auto begin() { return m_begin; }

    auto end() { return m_end; }

    static constexpr Base const &get(value_type const &v) {
      return raw_ref<value_type const, Base const>::get(v);
    }

  private:
    iterator m_begin, m_end;
  };

  template <typename U>
  subrange(U const &r) -> subrange<std::ranges::range_value_t<U>, U>;

  template <typename Base, typename R>
  subrange<Base, R> make_subrange(R const &r) {
    return subrange<Base, R>(r);
  }
} // namespace ranges

} // namespace vkw
#endif // VKRENDERER_COMMON_HPP
