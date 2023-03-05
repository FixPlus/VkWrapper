#ifndef VKRENDERER_RENDERPASS_HPP
#define VKRENDERER_RENDERPASS_HPP

#include <vkw/Device.hpp>
#include <vkw/RangeConcepts.hpp>

#include <algorithm>
#include <optional>
#include <vector>

namespace vkw {

class AttachmentDescription final : public ReferenceGuard {
public:
  AttachmentDescription(VkFormat viewFormat, VkSampleCountFlagBits samples,
                        VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
                        VkAttachmentLoadOp stencilLoadOp,
                        VkAttachmentStoreOp stencilStoreOp,
                        VkImageLayout initialLayout, VkImageLayout finalLayout,
                        VkAttachmentDescriptionFlags flags = 0) noexcept
      : m_description({flags, viewFormat, samples, loadOp, storeOp,
                       stencilLoadOp, stencilStoreOp, initialLayout,
                       finalLayout}) {}

  VkFormat format() const noexcept { return m_description.format; }

  bool isDepthStencil() const noexcept;
  bool formatHasDepthAspect() const noexcept;
  bool formatHasStencilAspect() const noexcept;
  bool isColor() const noexcept;

  VkSampleCountFlagBits samples() const { return m_description.samples; }
  VkAttachmentLoadOp loadOp() const noexcept { return m_description.loadOp; }

  VkAttachmentLoadOp stencilLoadOp() const noexcept {
    return m_description.stencilLoadOp;
  }

  VkAttachmentStoreOp storeOp() const noexcept { return m_description.storeOp; }

  VkAttachmentStoreOp stencilStoreOp() const noexcept {
    return m_description.stencilStoreOp;
  }

  operator VkAttachmentDescription const &() const noexcept {
    return m_description;
  }

private:
  VkAttachmentDescription m_description;
};

class SubpassDescription : public ReferenceGuard {
public:
  SubpassDescription &
  addInputAttachment(AttachmentDescription const &,
                     VkImageLayout layout) noexcept(ExceptionsDisabled);
  SubpassDescription &
  addColorAttachment(AttachmentDescription const &,
                     VkImageLayout layout) noexcept(ExceptionsDisabled);
  SubpassDescription &
  addDepthAttachment(AttachmentDescription const &,
                     VkImageLayout layout) noexcept(ExceptionsDisabled);
  SubpassDescription &
  addResolveAttachment(AttachmentDescription const &,
                       VkImageLayout layout) noexcept(ExceptionsDisabled);
  SubpassDescription &addPreserveAttachment(
      AttachmentDescription const &) noexcept(ExceptionsDisabled);

  using SubpassAttachmentContainerT = boost::container::small_vector<
      std::pair<StrongReference<AttachmentDescription const>, VkImageLayout>,
      2>;
  using PreservedAttachmentContainerT = boost::container::small_vector<
      StrongReference<AttachmentDescription const>, 2>;
  auto &inputAttachments() const noexcept { return m_inputAttachments; }

  auto &colorAttachments() const noexcept { return m_colorAttachments; }

  auto &resolveAttachments() const noexcept { return m_resolveAttachments; }

  auto &depthAttachments() const noexcept { return m_depthAttachment; }
  auto &preserveAttachments() const noexcept { return m_preserveAttachments; }

  VkDependencyFlags flags;

private:
  SubpassAttachmentContainerT m_inputAttachments;
  SubpassAttachmentContainerT m_colorAttachments;
  SubpassAttachmentContainerT m_resolveAttachments;
  std::optional<
      std::pair<StrongReference<AttachmentDescription const>, VkImageLayout>>
      m_depthAttachment;
  PreservedAttachmentContainerT m_preserveAttachments;
};

class SubpassDependency {
public:
  void setSrcSubpass(SubpassDescription const &subpass) noexcept {
    m_srcSubpass = subpass;
  }

  void setDstSubpass(SubpassDescription const &subpass) noexcept {
    m_dstSubpass = subpass;
  }

  auto srcSubpass() const noexcept { return m_srcSubpass; }

  auto dstSubpass() const noexcept { return m_dstSubpass; }

  VkPipelineStageFlags srcStageMask{};
  VkPipelineStageFlags dstStageMask{};
  VkAccessFlags srcAccessMask{};
  VkAccessFlags dstAccessMask{};
  VkDependencyFlags dependencyFlags{};

private:
  std::optional<StrongReference<SubpassDescription const>> m_srcSubpass{};
  std::optional<StrongReference<SubpassDescription const>> m_dstSubpass{};
};

class RenderPassCreateInfo {
public:
  RenderPassCreateInfo(RenderPassCreateInfo &&another) noexcept;
  RenderPassCreateInfo(RenderPassCreateInfo const &another) = delete;
  template <forward_range_of<AttachmentDescription const> T>
  RenderPassCreateInfo(
      T const &attachments,
      std::vector<StrongReference<SubpassDescription const>> const &subpasses,
      std::vector<SubpassDependency> const &dependencies,
      VkRenderPassCreateFlags flags = 0) noexcept(ExceptionsDisabled) {
    auto attachmentsSubrange =
        ranges::make_subrange<AttachmentDescription const>(attachments);
    using attachmentsSubrangeT = decltype(attachmentsSubrange);
    std::transform(attachmentsSubrange.begin(), attachmentsSubrange.end(),
                   std::back_inserter(m_attachments),
                   [](auto const &attachment) {
                     return StrongReference<const AttachmentDescription>{
                         attachmentsSubrangeT::get(attachment)};
                   });
    m_init(subpasses, dependencies, flags);
  }

  RenderPassCreateInfo(
      AttachmentDescription const &attachment,
      std::vector<StrongReference<SubpassDescription const>> const &subpasses,
      std::vector<SubpassDependency> const &dependencies,
      VkRenderPassCreateFlags flags = 0) noexcept(ExceptionsDisabled) {
    m_attachments.emplace_back(attachment);
    m_init(subpasses, dependencies, flags);
  }

  RenderPassCreateInfo &
  operator=(RenderPassCreateInfo &&another) noexcept = default;

  auto &info() const noexcept { return m_createInfo; }

  uint32_t subpassCount() const noexcept { return m_subpasses.size(); }

  struct M_subpassDesc {
    std::vector<VkAttachmentReference> inputAttachments;
    std::vector<VkAttachmentReference> colorAttachments;
    std::vector<VkAttachmentReference> resolveAttachments;
    std::optional<VkAttachmentReference> depthAttachment;
    std::vector<uint32_t> preserveAttachments;
  };

  M_subpassDesc const &subpassInfo(uint32_t subpass) const noexcept {
    return m_subpassesDescs.at(subpass);
  }

  auto const &attachmentDescriptions() const noexcept { return m_attachments; }

private:
  void m_init(
      std::vector<StrongReference<SubpassDescription const>> const &subpasses,
      std::vector<SubpassDependency> const &dependencies,
      VkRenderPassCreateFlags flags = 0) noexcept(ExceptionsDisabled);

  std::vector<StrongReference<AttachmentDescription const>> m_attachments;
  std::vector<VkAttachmentDescription> m_attachments_raw;
  std::vector<M_subpassDesc> m_subpassesDescs;
  std::vector<VkSubpassDescription> m_subpasses;
  std::vector<VkSubpassDependency> m_dependencies;

  VkRenderPassCreateInfo m_createInfo{};
};

class RenderPass : public RenderPassCreateInfo,
                   public UniqueVulkanObject<VkRenderPass> {
public:
  RenderPass(Device const &device,
             RenderPassCreateInfo createInfo) noexcept(ExceptionsDisabled)
      : RenderPassCreateInfo(std::move(createInfo)),
        UniqueVulkanObject<VkRenderPass>(device, info()) {}
};

} // namespace vkw
#endif // VKRENDERER_RENDERPASS_HPP
