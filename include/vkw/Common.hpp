#ifndef VKRENDERER_COMMON_HPP
#define VKRENDERER_COMMON_HPP

#include <functional>
#include <vector>
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

/*
 *  class RefArray
 *
 *  Used to pass arbitrary amount of T objects by reference.
 *
 */
template <typename T> class RefArray {
public:
  using TRef = std::reference_wrapper<T>;
  using VType = typename TypeTraits<typename std::remove_const<T>::type>::VType;
  using IterT = typename std::vector<TRef>::const_iterator;

  // default constructor for empty arrays
  RefArray() : m_container({}), m_raw({}){};

  RefArray &operator=(RefArray &&another) noexcept {
    m_container = std::move(another.m_container);
    m_raw = std::move(another.m_raw);
    return *this;
  }

  template <typename U>
  requires std::derived_from<U, T> && std::same_as<
      VType, typename TypeTraits<typename std::remove_const<U>::type>::VType>
  RefArray(std::vector<std::reference_wrapper<U>> array)
      : m_container(std::move(array)) {
    m_raw.reserve(m_container.size());
    for (auto &elem : m_container)
      m_raw.template emplace_back(elem.get());
  }

  template <typename... Args, typename Extra, typename Extra1>
  RefArray(Extra const &extra, Extra1 const &extra1, Args const &...args)
      : m_container{TRef{static_cast<T const &>(args)}...,
                    TRef{static_cast<T const &>(extra)},
                    TRef{static_cast<T const &>(extra1)}} {
    m_raw.reserve(m_container.size());
    for (auto &elem : m_container)
      m_raw.template emplace_back(elem.get());
  }

  template <typename... Args, typename Extra, typename Extra1>
  RefArray(Extra &extra, Extra1 &extra1, Args &...args)
      : m_container{TRef{static_cast<T &>(args)}...,
                    TRef{static_cast<T &>(extra)},
                    TRef{static_cast<T &>(extra1)}} {
    m_raw.reserve(m_container.size());
    for (auto &elem : m_container)
      m_raw.template emplace_back(elem.get());
  }

  template <typename U>
  requires std::derived_from<U, T> && std::same_as<
      VType, typename TypeTraits<typename std::remove_const<U>::type>::VType>
  RefArray(RefArray<U> const &array)
      : m_container(m_convert<>(array.begin(), array.end())) {
    m_raw.reserve(m_container.size());
    for (auto &elem : m_container)
      m_raw.template emplace_back(elem.get());
  }

  RefArray(RefArray &&another)
      : m_container(std::move(another.m_container)),
        m_raw(std::move(another.m_raw)) {}
  // should be implicit to allow passing single object by reference
  template <typename U>
  requires std::derived_from<U, T> && std::same_as<
      VType, typename TypeTraits<typename std::remove_const<U>::type>::VType>
  RefArray(U const &single) : m_container{single} {
    m_raw.template emplace_back(single);
  }

  RefArray(TRef const &single) : m_container{single} {
    m_raw.template emplace_back(single.get());
  }

  template <typename U>
  requires std::derived_from<U, T> && std::same_as<
      VType, typename TypeTraits<typename std::remove_const<U>::type>::VType>
  RefArray(std::vector<U> const &array)
      : m_container(m_convert<>(array.begin(), array.end())) {
    m_raw.reserve(m_container.size());
    for (auto &elem : m_container)
      m_raw.template emplace_back(elem.get());
  }

  // Range-based for operators

  IterT begin() const { return m_container.cbegin(); }

  IterT end() const { return m_container.cend(); }

  // Implicit cast to array of UnderlyingT

  operator VType const *() const {
    if (empty())
      return nullptr;
    return m_raw.data();
  }
  operator VType *() {
    if (empty())
      return nullptr;
    return m_raw.data();
  }

  size_t size() const { return m_container.size(); }

  bool empty() const { return m_container.empty(); }

private:
  template <typename Iter> std::vector<TRef> m_convert(Iter begin, Iter end) {
    std::vector<TRef> ret;
    for (auto it = begin; it != end; ++it) {
      ret.template emplace_back(*it);
    }
    return ret;
  }
  std::vector<TRef> m_container;
  std::vector<VType> m_raw;
};

/** Reference Types declaration. */

#define VKR_DECLARE_ARRAY_TYPES(T)                                             \
  using T##RefArray = RefArray<T>;                                             \
  using T##ConstRefArray = RefArray<T const>;

VKR_DECLARE_ARRAY_TYPES(AttachmentDescription)
VKR_DECLARE_ARRAY_TYPES(BufferBase)
VKR_DECLARE_ARRAY_TYPES(CommandBuffer)
VKR_DECLARE_ARRAY_TYPES(SecondaryCommandBuffer)
VKR_DECLARE_ARRAY_TYPES(PrimaryCommandBuffer)
VKR_DECLARE_ARRAY_TYPES(DescriptorSet)
VKR_DECLARE_ARRAY_TYPES(DescriptorSetLayout)
VKR_DECLARE_ARRAY_TYPES(DescriptorSetLayoutBinding)
VKR_DECLARE_ARRAY_TYPES(Device)
VKR_DECLARE_ARRAY_TYPES(Fence)
VKR_DECLARE_ARRAY_TYPES(FrameBuffer)
VKR_DECLARE_ARRAY_TYPES(AllocatedImage)
VKR_DECLARE_ARRAY_TYPES(ImageInterface)
VKR_DECLARE_ARRAY_TYPES(SwapChainImage)
VKR_DECLARE_ARRAY_TYPES(ImageViewBase)
VKR_DECLARE_ARRAY_TYPES(Instance)
VKR_DECLARE_ARRAY_TYPES(PhysicalDevice)
VKR_DECLARE_ARRAY_TYPES(Pipeline)
VKR_DECLARE_ARRAY_TYPES(PipelineCache)
VKR_DECLARE_ARRAY_TYPES(PipelineLayout)
VKR_DECLARE_ARRAY_TYPES(GraphicsPipeline)
VKR_DECLARE_ARRAY_TYPES(ComputePipeline)
VKR_DECLARE_ARRAY_TYPES(Queue)
VKR_DECLARE_ARRAY_TYPES(RenderPass)
VKR_DECLARE_ARRAY_TYPES(Sampler)
VKR_DECLARE_ARRAY_TYPES(Semaphore)
VKR_DECLARE_ARRAY_TYPES(ShaderBase)
VKR_DECLARE_ARRAY_TYPES(VertexShader)
VKR_DECLARE_ARRAY_TYPES(FragmentShader)
VKR_DECLARE_ARRAY_TYPES(ComputeShader)
VKR_DECLARE_ARRAY_TYPES(Surface)
VKR_DECLARE_ARRAY_TYPES(SwapChain)

#undef VKR_DECLARE_ARRAY_TYPES

#define VKR_DECLARE_REF_TYPES(T)                                               \
  using T##Ref = std::reference_wrapper<T>;                                    \
  using T##CRef = std::reference_wrapper<T const>;

VKR_DECLARE_REF_TYPES(AttachmentDescription)
VKR_DECLARE_REF_TYPES(BufferBase)
VKR_DECLARE_REF_TYPES(CommandBuffer)
VKR_DECLARE_REF_TYPES(SecondaryCommandBuffer)
VKR_DECLARE_REF_TYPES(PrimaryCommandBuffer)
VKR_DECLARE_REF_TYPES(DescriptorPool)
VKR_DECLARE_REF_TYPES(DescriptorSet)
VKR_DECLARE_REF_TYPES(DescriptorSetLayout)
VKR_DECLARE_REF_TYPES(DescriptorSetLayoutBinding)
VKR_DECLARE_REF_TYPES(Device)
VKR_DECLARE_REF_TYPES(Fence)
VKR_DECLARE_REF_TYPES(FrameBuffer)
VKR_DECLARE_REF_TYPES(AllocatedImage)
VKR_DECLARE_REF_TYPES(ImageInterface)
VKR_DECLARE_REF_TYPES(SwapChainImage)
VKR_DECLARE_REF_TYPES(ImageViewBase)
VKR_DECLARE_REF_TYPES(Instance)
VKR_DECLARE_REF_TYPES(PhysicalDevice)
VKR_DECLARE_REF_TYPES(Pipeline)
VKR_DECLARE_REF_TYPES(PipelineCache)
VKR_DECLARE_REF_TYPES(PipelineLayout)
VKR_DECLARE_REF_TYPES(GraphicsPipeline)
VKR_DECLARE_REF_TYPES(ComputePipeline)
VKR_DECLARE_REF_TYPES(Queue)
VKR_DECLARE_REF_TYPES(RenderPass)
VKR_DECLARE_REF_TYPES(Sampler)
VKR_DECLARE_REF_TYPES(Semaphore)
VKR_DECLARE_REF_TYPES(ShaderBase)
VKR_DECLARE_REF_TYPES(VertexShader)
VKR_DECLARE_REF_TYPES(FragmentShader)
VKR_DECLARE_REF_TYPES(ComputeShader)
VKR_DECLARE_REF_TYPES(SubpassDependency)
VKR_DECLARE_REF_TYPES(SubpassDescription)
VKR_DECLARE_REF_TYPES(Surface)
VKR_DECLARE_REF_TYPES(SwapChain)

#undef VKR_DECLARE_REF_TYPES
} // namespace vkw
#endif // VKRENDERER_COMMON_HPP
