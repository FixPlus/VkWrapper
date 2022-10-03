#ifndef VKRENDERER_RENDERPASS_HPP
#define VKRENDERER_RENDERPASS_HPP

#include "Common.hpp"
#include <boost/container/small_vector.hpp>
#include <optional>
#include <vector>

namespace vkw {

class AttachmentDescription final {
public:
  AttachmentDescription(VkFormat viewFormat, VkSampleCountFlagBits samples,
                        VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                        VkAttachmentLoadOp stencilLoadOp,
                        VkAttachmentStoreOp stencilStoreOp,
                        VkImageLayout initialLayout, VkImageLayout finalLayout,
                        VkAttachmentDescriptionFlags flags = 0)
      : m_description({flags, viewFormat, samples, loadOp, storeOp,
                       stencilLoadOp, stencilStoreOp, initialLayout,
                       finalLayout}) {}

  VkFormat format() const { return m_description.format; }

  bool isDepthStencil() const;
  bool formatHasDepthAspect() const;
  bool formatHasStencilAspect() const;
  bool isColor() const;

  VkAttachmentLoadOp loadOp() const { return m_description.loadOp; }

  VkAttachmentLoadOp stencilLoadOp() const {
    return m_description.stencilLoadOp;
  }

  VkAttachmentStoreOp storeOp() const { return m_description.storeOp; }

  VkAttachmentStoreOp stencilStoreOp() const {
    return m_description.stencilStoreOp;
  }

  operator VkAttachmentDescription const &() const { return m_description; }

private:
  VkAttachmentDescription m_description;
};

class SubpassDescription {
public:
  void addInputAttachment(AttachmentDescription const &, VkImageLayout layout);
  void addColorAttachment(AttachmentDescription const &, VkImageLayout layout);
  void addDepthAttachment(AttachmentDescription const &, VkImageLayout layout);
  void addPreserveAttachment(AttachmentDescription const &);

  using SubpassAttachmentContainerT = boost::container::small_vector<
      std::pair<AttachmentDescriptionCRef, VkImageLayout>, 2>;
  using PreservedAttachmentContainerT =
      boost::container::small_vector<AttachmentDescriptionCRef, 2>;
  SubpassAttachmentContainerT const &inputAttachments() const {
    return m_inputAttachments;
  }

  SubpassAttachmentContainerT const &colorAttachments() const {
    return m_colorAttachments;
  }
  std::optional<std::pair<AttachmentDescriptionCRef, VkImageLayout>> const &
  depthAttachments() const {
    return m_depthAttachment;
  }
  PreservedAttachmentContainerT const &preserveAttachments() const {
    return m_preserveAttachments;
  }

  VkDependencyFlags flags;

private:
  SubpassAttachmentContainerT m_inputAttachments;
  SubpassAttachmentContainerT m_colorAttachments;
  std::optional<std::pair<AttachmentDescriptionCRef, VkImageLayout>>
      m_depthAttachment;
  PreservedAttachmentContainerT m_preserveAttachments;
};

class SubpassDependency {
public:
  void setSrcSubpass(SubpassDescription const &subpass) {
    m_srcSubpass = subpass;
  }

  void setDstSubpass(SubpassDescription const &subpass) {
    m_dstSubpass = subpass;
  }

  std::optional<SubpassDescriptionCRef> srcSubpass() const {
    return m_srcSubpass;
  }

  std::optional<SubpassDescriptionCRef> dstSubpass() const {
    return m_dstSubpass;
  }

  VkPipelineStageFlags srcStageMask{};
  VkPipelineStageFlags dstStageMask{};
  VkAccessFlags srcAccessMask{};
  VkAccessFlags dstAccessMask{};
  VkDependencyFlags dependencyFlags{};

private:
  std::optional<SubpassDescriptionCRef> m_srcSubpass{};
  std::optional<SubpassDescriptionCRef> m_dstSubpass{};
};

class RenderPassCreateInfo {
public:
  RenderPassCreateInfo(RenderPassCreateInfo &&another);
  RenderPassCreateInfo(RenderPassCreateInfo const &another) = delete;
  template <forward_range_of<AttachmentDescription> T>
  RenderPassCreateInfo(T const &attachments,
                       std::vector<SubpassDescriptionCRef> const &subpasses,
                       std::vector<SubpassDependency> const &dependencies,
                       VkRenderPassCreateFlags flags = 0) {
    auto attachmentsSubrange =
        ranges::make_subrange<AttachmentDescription>(attachments);
    using attachmentsSubrangeT = decltype(attachmentsSubrange);
    std::transform(attachmentsSubrange.begin(), attachmentsSubrange.end(),
                   std::back_inserter(m_attachments),
                   [](auto const &attachment) {
                     return std::reference_wrapper<const AttachmentDescription>{
                         attachmentsSubrangeT::get(attachment)};
                   });
    m_init(subpasses, dependencies, flags);
  }

  RenderPassCreateInfo(AttachmentDescription const &attachment,
                       std::vector<SubpassDescriptionCRef> const &subpasses,
                       std::vector<SubpassDependency> const &dependencies,
                       VkRenderPassCreateFlags flags = 0) {
    m_attachments.emplace_back(attachment);
    m_init(subpasses, dependencies, flags);
  }

  RenderPassCreateInfo &
  operator=(RenderPassCreateInfo &&another) noexcept = default;

  operator VkRenderPassCreateInfo const &() const { return m_createInfo; }

  uint32_t subpassCount() const { return m_subpasses.size(); }

  struct M_subpassDesc {
    std::vector<VkAttachmentReference> inputAttachments;
    std::vector<VkAttachmentReference> colorAttachments;
    std::optional<VkAttachmentReference> depthAttachment;
    std::vector<uint32_t> preserveAttachments;
  };

  M_subpassDesc const &subpassInfo(uint32_t subpass) const {
    return m_subpassesDescs.at(subpass);
  }

  std::vector<AttachmentDescriptionCRef> const &attachmentDescriptions() const {
    return m_attachments;
  }

private:
  void m_init(std::vector<SubpassDescriptionCRef> const &subpasses,
              std::vector<SubpassDependency> const &dependencies,
              VkRenderPassCreateFlags flags = 0);

  std::vector<AttachmentDescriptionCRef> m_attachments;
  std::vector<VkAttachmentDescription> m_attachments_raw;
  std::vector<M_subpassDesc> m_subpassesDescs;
  std::vector<VkSubpassDescription> m_subpasses;
  std::vector<VkSubpassDependency> m_dependencies;

  VkRenderPassCreateInfo m_createInfo{};
};

class RenderPass {
public:
  RenderPass(Device &device, RenderPassCreateInfo createInfo);

  RenderPass(RenderPass &&another) noexcept
      : m_parent(another.m_parent),
        m_createInfo(std::move(another.m_createInfo)),
        m_renderPass(another.m_renderPass) {
    another.m_renderPass = VK_NULL_HANDLE;
  }

  RenderPass &operator=(RenderPass &&another) noexcept {
    m_parent = another.m_parent;
    m_createInfo = std::move(another.m_createInfo);
    std::swap(m_renderPass, another.m_renderPass);
    return *this;
  }

  virtual ~RenderPass();

  RenderPassCreateInfo const &info() const { return m_createInfo; }

  operator VkRenderPass() const { return m_renderPass; }

private:
  DeviceRef m_parent;
  RenderPassCreateInfo m_createInfo;
  VkRenderPass m_renderPass = VK_NULL_HANDLE;
};

} // namespace vkw
#endif // VKRENDERER_RENDERPASS_HPP
