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
class ColorImageInterface;
class DepthStencilImageInterface;
class ImageArrayInterface;
class Image1DInterface;
class Image1DArrayInterface;
class Image2DInterface;
class Image2DArrayInterface;
class ImageCubeInterface;
class ImageCubeArrayInterface;
class Image3DInterface;

/** Predeclarations of templates for complete image interfaces.*/

template <typename T>
concept ImageAspectInterface = std::same_as<T, ColorImageInterface> ||
    std::same_as<T, DepthStencilImageInterface>;

template <ImageAspectInterface T> class T_Image1DInterface;

template <ImageAspectInterface T> class T_Image1DArrayInterface;

template <ImageAspectInterface T> class T_Image2DInterface;

template <ImageAspectInterface T> class T_Image2DArrayInterface;

template <ImageAspectInterface T> class T_Image3DInterface;

template <ImageAspectInterface T> class T_ImageCubeInterface;

template <ImageAspectInterface T> class T_ImageCubeArrayInterface;

/** Alias names for template specializations of complete image interfaces. */

using ColorImage1DInterface = T_Image1DInterface<ColorImageInterface>;
using DepthStencilImage1DInterface =
    T_Image1DInterface<DepthStencilImageInterface>;
using ColorImage2DInterface = T_Image2DInterface<ColorImageInterface>;
using DepthStencilImage2DInterface =
    T_Image2DInterface<DepthStencilImageInterface>;
using ColorImage3DInterface = T_Image3DInterface<ColorImageInterface>;
using DepthStencilImage3DInterface =
    T_Image3DInterface<DepthStencilImageInterface>;
using ColorImageCubeInterface = T_ImageCubeInterface<ColorImageInterface>;
using DepthStencilImageCubeInterface =
    T_ImageCubeInterface<DepthStencilImageInterface>;
using ColorImage1DArrayInterface = T_Image1DArrayInterface<ColorImageInterface>;
using DepthStencilImage1DArrayInterface =
    T_Image1DArrayInterface<DepthStencilImageInterface>;
using ColorImage2DArrayInterface = T_Image2DArrayInterface<ColorImageInterface>;
using DepthStencilImage2DArrayInterface =
    T_Image2DArrayInterface<DepthStencilImageInterface>;
using ColorImageCubeArrayInterface =
    T_ImageCubeArrayInterface<ColorImageInterface>;
using DepthStencilImageCubeArrayInterface =
    T_ImageCubeArrayInterface<DepthStencilImageInterface>;

class SwapChainImage;
class ImageView;
class Image1DView;
class Image2DView;
class Image3DView;
class Image1DArrayView;
class Image2DArrayView;
class ImageCubeView;
class ImageCubeArrayView;
class ColorImageView;
class DepthImageView;
class StencilImageView;
class DepthStencilImageView;
template <typename T>
concept ImageViewAspectInterface =
    std::same_as<T, ColorImageView> || std::same_as<T, DepthImageView> ||
    std::same_as<T, StencilImageView> || std::same_as<T, DepthStencilImageView>;

/** Predeclarations of templates for complete image view interfaces. */

template <ImageViewAspectInterface T> class T_Image1DView;
template <ImageViewAspectInterface T> class T_Image1DArrayView;
template <ImageViewAspectInterface T> class T_Image2DView;
template <ImageViewAspectInterface T> class T_Image2DArrayView;
template <ImageViewAspectInterface T> class T_Image3DView;
template <ImageViewAspectInterface T> class T_ImageCubeView;
template <ImageViewAspectInterface T> class T_ImageCubeArrayView;

/** Alias names for template specializations of complete image view
 * interfaces.*/

using ColorImage1DView = T_Image1DView<ColorImageView>;
using DepthStencilImage1DView = T_Image1DView<DepthStencilImageView>;
using DepthImage1DView = T_Image1DView<DepthImageView>;
using StencilImage1DView = T_Image1DView<StencilImageView>;

using ColorImage1DArrayView = T_Image1DArrayView<ColorImageView>;
using DepthStencilImage1DArrayView = T_Image1DArrayView<DepthStencilImageView>;
using DepthImage1DArrayView = T_Image1DArrayView<DepthImageView>;
using StencilImage1DArrayView = T_Image1DArrayView<StencilImageView>;

using ColorImage2DView = T_Image2DView<ColorImageView>;
using DepthStencilImage2DView = T_Image2DView<DepthStencilImageView>;
using DepthImage2DView = T_Image2DView<DepthImageView>;
using StencilImage2DView = T_Image2DView<StencilImageView>;

using ColorImage2DArrayView = T_Image2DArrayView<ColorImageView>;
using DepthStencilImage2DArrayView = T_Image2DArrayView<DepthStencilImageView>;
using DepthImage2DArrayView = T_Image2DArrayView<DepthImageView>;
using StencilImage2DArrayView = T_Image2DArrayView<StencilImageView>;

using ColorImage3DView = T_Image3DView<ColorImageView>;
using DepthStencilImage3DView = T_Image3DView<DepthStencilImageView>;
using DepthImage3DView = T_Image3DView<DepthImageView>;
using StencilImage3DView = T_Image3DView<StencilImageView>;

using ColorImageCubeView = T_ImageCubeView<ColorImageView>;
using DepthStencilImageCubeView = T_ImageCubeView<DepthStencilImageView>;
using DepthImageCubeView = T_ImageCubeView<DepthImageView>;
using StencilImageCubeView = T_ImageCubeView<StencilImageView>;

using ColorImageCubeArrayView = T_ImageCubeArrayView<ColorImageView>;
using DepthStencilImageCubeArrayView =
    T_ImageCubeArrayView<DepthStencilImageView>;
using DepthImageCubeArrayView = T_ImageCubeArrayView<DepthImageView>;
using StencilImageCubeArrayView = T_ImageCubeArrayView<StencilImageView>;

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

template <> struct TypeTraits<ColorImageInterface> { using VType = VkImage; };
template <> struct TypeTraits<DepthStencilImageInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<ImageArrayInterface> { using VType = VkImage; };
template <> struct TypeTraits<Image1DInterface> { using VType = VkImage; };
template <> struct TypeTraits<Image1DArrayInterface> { using VType = VkImage; };
template <> struct TypeTraits<Image2DInterface> { using VType = VkImage; };
template <> struct TypeTraits<Image2DArrayInterface> { using VType = VkImage; };
template <> struct TypeTraits<ImageCubeInterface> { using VType = VkImage; };
template <> struct TypeTraits<ImageCubeArrayInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<Image3DInterface> { using VType = VkImage; };

template <> struct TypeTraits<ColorImage1DInterface> { using VType = VkImage; };
template <> struct TypeTraits<DepthStencilImage1DInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<ColorImage2DInterface> { using VType = VkImage; };
template <> struct TypeTraits<DepthStencilImage2DInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<ColorImage3DInterface> { using VType = VkImage; };
template <> struct TypeTraits<DepthStencilImage3DInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<ColorImageCubeInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<DepthStencilImageCubeInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<ColorImage1DArrayInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<DepthStencilImage1DArrayInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<ColorImage2DArrayInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<DepthStencilImage2DArrayInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<ColorImageCubeArrayInterface> {
  using VType = VkImage;
};
template <> struct TypeTraits<DepthStencilImageCubeArrayInterface> {
  using VType = VkImage;
};

template <> struct TypeTraits<SwapChainImage> { using VType = VkImage; };

template <> struct TypeTraits<ImageView> { using VType = VkImageView; };

template <> struct TypeTraits<Image1DView> { using VType = VkImageView; };

template <> struct TypeTraits<Image2DView> { using VType = VkImageView; };

template <> struct TypeTraits<Image3DView> { using VType = VkImageView; };

template <> struct TypeTraits<Image1DArrayView> { using VType = VkImageView; };

template <> struct TypeTraits<Image2DArrayView> { using VType = VkImageView; };

template <> struct TypeTraits<ImageCubeView> { using VType = VkImageView; };

template <> struct TypeTraits<ImageCubeArrayView> {
  using VType = VkImageView;
};

template <> struct TypeTraits<ColorImage1DView> { using VType = VkImageView; };
template <> struct TypeTraits<DepthStencilImage1DView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthImage1DView> { using VType = VkImageView; };
template <> struct TypeTraits<StencilImage1DView> {
  using VType = VkImageView;
};

template <> struct TypeTraits<ColorImage1DArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthStencilImage1DArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthImage1DArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<StencilImage1DArrayView> {
  using VType = VkImageView;
};

template <> struct TypeTraits<ColorImage2DView> { using VType = VkImageView; };
template <> struct TypeTraits<DepthStencilImage2DView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthImage2DView> { using VType = VkImageView; };
template <> struct TypeTraits<StencilImage2DView> {
  using VType = VkImageView;
};

template <> struct TypeTraits<ColorImage2DArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthStencilImage2DArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthImage2DArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<StencilImage2DArrayView> {
  using VType = VkImageView;
};

template <> struct TypeTraits<ColorImage3DView> { using VType = VkImageView; };
template <> struct TypeTraits<DepthStencilImage3DView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthImage3DView> { using VType = VkImageView; };
template <> struct TypeTraits<StencilImage3DView> {
  using VType = VkImageView;
};

template <> struct TypeTraits<ColorImageCubeView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthStencilImageCubeView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthImageCubeView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<StencilImageCubeView> {
  using VType = VkImageView;
};

template <> struct TypeTraits<ColorImageCubeArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthStencilImageCubeArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<DepthImageCubeArrayView> {
  using VType = VkImageView;
};
template <> struct TypeTraits<StencilImageCubeArrayView> {
  using VType = VkImageView;
};

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

  template<typename U>
  requires std::derived_from<U, T> && std::same_as<
      VType, typename TypeTraits<typename std::remove_const<U>::type>::VType>
  RefArray(std::vector<U> const& array): m_container(m_convert<>(array.begin(), array.end())){
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
  std::vector<TRef> const m_container;
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
VKR_DECLARE_ARRAY_TYPES(ColorImageInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImageInterface)
VKR_DECLARE_ARRAY_TYPES(ImageArrayInterface)
VKR_DECLARE_ARRAY_TYPES(Image1DInterface)
VKR_DECLARE_ARRAY_TYPES(Image1DArrayInterface)
VKR_DECLARE_ARRAY_TYPES(Image2DInterface)
VKR_DECLARE_ARRAY_TYPES(Image2DArrayInterface)
VKR_DECLARE_ARRAY_TYPES(ImageCubeInterface)
VKR_DECLARE_ARRAY_TYPES(ImageCubeArrayInterface)
VKR_DECLARE_ARRAY_TYPES(Image3DInterface)
VKR_DECLARE_ARRAY_TYPES(ColorImage1DInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage1DInterface)
VKR_DECLARE_ARRAY_TYPES(ColorImage2DInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage2DInterface)
VKR_DECLARE_ARRAY_TYPES(ColorImage3DInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage3DInterface)
VKR_DECLARE_ARRAY_TYPES(ColorImageCubeInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImageCubeInterface)
VKR_DECLARE_ARRAY_TYPES(ColorImage1DArrayInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage1DArrayInterface)
VKR_DECLARE_ARRAY_TYPES(ColorImage2DArrayInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage2DArrayInterface)
VKR_DECLARE_ARRAY_TYPES(ColorImageCubeArrayInterface)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImageCubeArrayInterface)
VKR_DECLARE_ARRAY_TYPES(SwapChainImage)
VKR_DECLARE_ARRAY_TYPES(ImageView)
VKR_DECLARE_ARRAY_TYPES(Image1DView)
VKR_DECLARE_ARRAY_TYPES(Image2DView)
VKR_DECLARE_ARRAY_TYPES(Image3DView)
VKR_DECLARE_ARRAY_TYPES(Image1DArrayView)
VKR_DECLARE_ARRAY_TYPES(Image2DArrayView)
VKR_DECLARE_ARRAY_TYPES(ImageCubeView)
VKR_DECLARE_ARRAY_TYPES(ImageCubeArrayView)
VKR_DECLARE_ARRAY_TYPES(ColorImage1DView)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage1DView)
VKR_DECLARE_ARRAY_TYPES(DepthImage1DView)
VKR_DECLARE_ARRAY_TYPES(StencilImage1DView)

VKR_DECLARE_ARRAY_TYPES(ColorImage1DArrayView)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage1DArrayView)
VKR_DECLARE_ARRAY_TYPES(DepthImage1DArrayView)
VKR_DECLARE_ARRAY_TYPES(StencilImage1DArrayView)

VKR_DECLARE_ARRAY_TYPES(ColorImage2DView)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage2DView)
VKR_DECLARE_ARRAY_TYPES(DepthImage2DView)
VKR_DECLARE_ARRAY_TYPES(StencilImage2DView)

VKR_DECLARE_ARRAY_TYPES(ColorImage2DArrayView)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage2DArrayView)
VKR_DECLARE_ARRAY_TYPES(DepthImage2DArrayView)
VKR_DECLARE_ARRAY_TYPES(StencilImage2DArrayView)

VKR_DECLARE_ARRAY_TYPES(ColorImage3DView)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImage3DView)
VKR_DECLARE_ARRAY_TYPES(DepthImage3DView)
VKR_DECLARE_ARRAY_TYPES(StencilImage3DView)

VKR_DECLARE_ARRAY_TYPES(ColorImageCubeView)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImageCubeView)
VKR_DECLARE_ARRAY_TYPES(DepthImageCubeView)
VKR_DECLARE_ARRAY_TYPES(StencilImageCubeView)

VKR_DECLARE_ARRAY_TYPES(ColorImageCubeArrayView)
VKR_DECLARE_ARRAY_TYPES(DepthStencilImageCubeArrayView)
VKR_DECLARE_ARRAY_TYPES(DepthImageCubeArrayView)
VKR_DECLARE_ARRAY_TYPES(StencilImageCubeArrayView)

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
VKR_DECLARE_REF_TYPES(ColorImageInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImageInterface)
VKR_DECLARE_REF_TYPES(ImageArrayInterface)
VKR_DECLARE_REF_TYPES(Image1DInterface)
VKR_DECLARE_REF_TYPES(Image1DArrayInterface)
VKR_DECLARE_REF_TYPES(Image2DInterface)
VKR_DECLARE_REF_TYPES(Image2DArrayInterface)
VKR_DECLARE_REF_TYPES(ImageCubeInterface)
VKR_DECLARE_REF_TYPES(ImageCubeArrayInterface)
VKR_DECLARE_REF_TYPES(Image3DInterface)
VKR_DECLARE_REF_TYPES(ColorImage1DInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImage1DInterface)
VKR_DECLARE_REF_TYPES(ColorImage2DInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImage2DInterface)
VKR_DECLARE_REF_TYPES(ColorImage3DInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImage3DInterface)
VKR_DECLARE_REF_TYPES(ColorImageCubeInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImageCubeInterface)
VKR_DECLARE_REF_TYPES(ColorImage1DArrayInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImage1DArrayInterface)
VKR_DECLARE_REF_TYPES(ColorImage2DArrayInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImage2DArrayInterface)
VKR_DECLARE_REF_TYPES(ColorImageCubeArrayInterface)
VKR_DECLARE_REF_TYPES(DepthStencilImageCubeArrayInterface)
VKR_DECLARE_REF_TYPES(SwapChainImage)
VKR_DECLARE_REF_TYPES(ImageView)
VKR_DECLARE_REF_TYPES(Image1DView)
VKR_DECLARE_REF_TYPES(Image2DView)
VKR_DECLARE_REF_TYPES(Image3DView)
VKR_DECLARE_REF_TYPES(Image1DArrayView)
VKR_DECLARE_REF_TYPES(Image2DArrayView)
VKR_DECLARE_REF_TYPES(ImageCubeView)
VKR_DECLARE_REF_TYPES(ImageCubeArrayView)
VKR_DECLARE_REF_TYPES(ColorImage1DArrayView)
VKR_DECLARE_REF_TYPES(DepthStencilImage1DArrayView)
VKR_DECLARE_REF_TYPES(DepthImage1DArrayView)
VKR_DECLARE_REF_TYPES(StencilImage1DArrayView)

VKR_DECLARE_REF_TYPES(ColorImage1DView)
VKR_DECLARE_REF_TYPES(DepthStencilImage1DView)
VKR_DECLARE_REF_TYPES(DepthImage1DView)
VKR_DECLARE_REF_TYPES(StencilImage1DView)

VKR_DECLARE_REF_TYPES(ColorImage2DView)
VKR_DECLARE_REF_TYPES(DepthStencilImage2DView)
VKR_DECLARE_REF_TYPES(DepthImage2DView)
VKR_DECLARE_REF_TYPES(StencilImage2DView)

VKR_DECLARE_REF_TYPES(ColorImage2DArrayView)
VKR_DECLARE_REF_TYPES(DepthStencilImage2DArrayView)
VKR_DECLARE_REF_TYPES(DepthImage2DArrayView)
VKR_DECLARE_REF_TYPES(StencilImage2DArrayView)

VKR_DECLARE_REF_TYPES(ColorImage3DView)
VKR_DECLARE_REF_TYPES(DepthStencilImage3DView)
VKR_DECLARE_REF_TYPES(DepthImage3DView)
VKR_DECLARE_REF_TYPES(StencilImage3DView)

VKR_DECLARE_REF_TYPES(ColorImageCubeView)
VKR_DECLARE_REF_TYPES(DepthStencilImageCubeView)
VKR_DECLARE_REF_TYPES(DepthImageCubeView)
VKR_DECLARE_REF_TYPES(StencilImageCubeView)

VKR_DECLARE_REF_TYPES(ColorImageCubeArrayView)
VKR_DECLARE_REF_TYPES(DepthStencilImageCubeArrayView)
VKR_DECLARE_REF_TYPES(DepthImageCubeArrayView)
VKR_DECLARE_REF_TYPES(StencilImageCubeArrayView)

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
