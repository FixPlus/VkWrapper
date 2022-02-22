#ifndef VKRENDERER_COMMON_HPP
#define VKRENDERER_COMMON_HPP

#include <functional>
#include <vector>
#include <vulkan/vulkan.h>

namespace vkw {

/** Predeclarations. */

class BufferBase;
class CommandBuffer;
class SecondaryCommandBuffer;
class PrimaryCommandBuffer;
class CommandPool;
class Device;
class Fence;
class FrameBuffer;
class ImageBase;
class ImageInterface;
class ImageView;
class Instance;
class Pipeline;
class PipelineLayout;
class GraphicsPipeline;
class ComputePipeline;
class Queue;
class RenderPass;
class Semaphore;
class ShaderBase;
class VertexShader;
class FragmentShader;
class Surface;
class SwapChain;

template <typename T> struct TypeTraits {};

template <> struct TypeTraits<BufferBase> { using VType = VkBuffer; };

template <> struct TypeTraits<CommandBuffer> { using VType = VkCommandBuffer; };

template <> struct TypeTraits<SecondaryCommandBuffer> {
  using VType = VkCommandBuffer;
};

template <> struct TypeTraits<PrimaryCommandBuffer> {
  using VType = VkCommandBuffer;
};

template <> struct TypeTraits<CommandPool> { using VType = VkCommandPool; };

template <> struct TypeTraits<Device> { using VType = VkDevice; };

template <> struct TypeTraits<Fence> { using VType = VkFence; };

template <> struct TypeTraits<FrameBuffer> { using VType = VkFramebuffer; };

template <> struct TypeTraits<ImageBase> { using VType = VkImage; };

template <> struct TypeTraits<ImageInterface> { using VType = VkImage; };

template <> struct TypeTraits<ImageView> { using VType = VkImageView; };

template <> struct TypeTraits<Instance> { using VType = VkInstance; };

template <> struct TypeTraits<Pipeline> { using VType = VkPipeline; };

template <> struct TypeTraits<PipelineLayout> {
  using VType = VkPipelineLayout;
};

template <> struct TypeTraits<GraphicsPipeline> { using VType = VkPipeline; };

template <> struct TypeTraits<ComputePipeline> { using VType = VkPipeline; };

template <> struct TypeTraits<Queue> { using VType = VkQueue; };

template <> struct TypeTraits<RenderPass> { using VType = VkRenderPass; };

template <> struct TypeTraits<Semaphore> { using VType = VkSemaphore; };

template <> struct TypeTraits<ShaderBase> { using VType = VkShaderModule; };

template <> struct TypeTraits<FragmentShader> { using VType = VkShaderModule; };

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

  RefArray(std::initializer_list<TRef> array) : m_container(array) {
    m_raw.reserve(m_container.size());
    for (auto &elem : m_container)
      m_raw.template emplace_back(elem.get());
  }
  // should be implicit to allow passing single object by reference
  RefArray(T &single) : RefArray({single}) {}

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
  std::vector<TRef> const m_container;
  std::vector<VType> m_raw;
};

/** Reference Types declaration. */

#define VKR_DECLARE_ARRAY_TYPES(T)                                             \
  using T##RefArray = RefArray<T>;                                             \
  using T##ConstRefArray = RefArray<T const>;

VKR_DECLARE_ARRAY_TYPES(BufferBase)
VKR_DECLARE_ARRAY_TYPES(CommandBuffer)
VKR_DECLARE_ARRAY_TYPES(SecondaryCommandBuffer)
VKR_DECLARE_ARRAY_TYPES(PrimaryCommandBuffer)
VKR_DECLARE_ARRAY_TYPES(Device)
VKR_DECLARE_ARRAY_TYPES(Fence)
VKR_DECLARE_ARRAY_TYPES(FrameBuffer)
VKR_DECLARE_ARRAY_TYPES(ImageBase)
VKR_DECLARE_ARRAY_TYPES(ImageInterface)
VKR_DECLARE_ARRAY_TYPES(ImageView)
VKR_DECLARE_ARRAY_TYPES(Instance)
VKR_DECLARE_ARRAY_TYPES(Pipeline)
VKR_DECLARE_ARRAY_TYPES(PipelineLayout)
VKR_DECLARE_ARRAY_TYPES(GraphicsPipeline)
VKR_DECLARE_ARRAY_TYPES(ComputePipeline)
VKR_DECLARE_ARRAY_TYPES(Queue)
VKR_DECLARE_ARRAY_TYPES(RenderPass)
VKR_DECLARE_ARRAY_TYPES(Semaphore)
VKR_DECLARE_ARRAY_TYPES(ShaderBase)
VKR_DECLARE_ARRAY_TYPES(VertexShader)
VKR_DECLARE_ARRAY_TYPES(FragmentShader)
VKR_DECLARE_ARRAY_TYPES(Surface)
VKR_DECLARE_ARRAY_TYPES(SwapChain)

#undef VKR_DECLARE_ARRAY_TYPES

#define VKR_DECLARE_REF_TYPES(T)                                               \
  using T##Ref = std::reference_wrapper<T>;                                    \
  using T##CRef = std::reference_wrapper<T const>;

VKR_DECLARE_REF_TYPES(BufferBase)
VKR_DECLARE_REF_TYPES(CommandBuffer)
VKR_DECLARE_REF_TYPES(SecondaryCommandBuffer)
VKR_DECLARE_REF_TYPES(PrimaryCommandBuffer)
VKR_DECLARE_REF_TYPES(Device)
VKR_DECLARE_REF_TYPES(Fence)
VKR_DECLARE_REF_TYPES(FrameBuffer)
VKR_DECLARE_REF_TYPES(ImageBase)
VKR_DECLARE_REF_TYPES(ImageInterface)
VKR_DECLARE_REF_TYPES(ImageView)
VKR_DECLARE_REF_TYPES(Instance)
VKR_DECLARE_REF_TYPES(Pipeline)
VKR_DECLARE_REF_TYPES(PipelineLayout)
VKR_DECLARE_REF_TYPES(GraphicsPipeline)
VKR_DECLARE_REF_TYPES(ComputePipeline)
VKR_DECLARE_REF_TYPES(Queue)
VKR_DECLARE_REF_TYPES(RenderPass)
VKR_DECLARE_REF_TYPES(Semaphore)
VKR_DECLARE_REF_TYPES(ShaderBase)
VKR_DECLARE_REF_TYPES(VertexShader)
VKR_DECLARE_REF_TYPES(FragmentShader)
VKR_DECLARE_REF_TYPES(Surface)
VKR_DECLARE_REF_TYPES(SwapChain)

#undef VKR_DECLARE_REF_TYPES
} // namespace vkw
#endif // VKRENDERER_COMMON_HPP
