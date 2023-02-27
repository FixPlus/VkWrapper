#ifndef VKRENDERER_RENDERPASS_HPP
#define VKRENDERER_RENDERPASS_HPP

#include "vkw/Device.hpp"
#include "vkw/RangeConcepts.hpp"
#include <vkw/UniqueVulkanObject.hpp>

#include <algorithm>
#include <boost/container/small_vector.hpp>
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

class SubpassDescription : public ReferenceGuard {
public:
  void addInputAttachment(AttachmentDescription const &, VkImageLayout layout);
  void addColorAttachment(AttachmentDescription const &, VkImageLayout layout);
  void addDepthAttachment(AttachmentDescription const &, VkImageLayout layout);
  void addPreserveAttachment(AttachmentDescription const &);

  using SubpassAttachmentContainerT = boost::container::small_vector<
      std::pair<StrongReference<AttachmentDescription const>, VkImageLayout>,
      2>;
  using PreservedAttachmentContainerT = boost::container::small_vector<
      StrongReference<AttachmentDescription const>, 2>;
  SubpassAttachmentContainerT const &inputAttachments() const {
    return m_inputAttachments;
  }

  SubpassAttachmentContainerT const &colorAttachments() const {
    return m_colorAttachments;
  }
  std::optional<std::pair<StrongReference<AttachmentDescription const>,
                          VkImageLayout>> const &
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
  std::optional<
      std::pair<StrongReference<AttachmentDescription const>, VkImageLayout>>
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

  auto srcSubpass() const { return m_srcSubpass; }

  auto dstSubpass() const { return m_dstSubpass; }

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
  RenderPassCreateInfo(RenderPassCreateInfo &&another);
  RenderPassCreateInfo(RenderPassCreateInfo const &another) = delete;
  template <forward_range_of<AttachmentDescription const> T>
  RenderPassCreateInfo(
      T const &attachments,
      std::vector<StrongReference<SubpassDescription const>> const &subpasses,
      std::vector<SubpassDependency> const &dependencies,
      VkRenderPassCreateFlags flags = 0) {
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
      VkRenderPassCreateFlags flags = 0) {
    m_attachments.emplace_back(attachment);
    m_init(subpasses, dependencies, flags);
  }

  RenderPassCreateInfo &
  operator=(RenderPassCreateInfo &&another) noexcept = default;

  auto &info() const { return m_createInfo; }

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

  auto const &attachmentDescriptions() const { return m_attachments; }

private:
  void m_init(
      std::vector<StrongReference<SubpassDescription const>> const &subpasses,
      std::vector<SubpassDependency> const &dependencies,
      VkRenderPassCreateFlags flags = 0);

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
  RenderPass(Device const &device, RenderPassCreateInfo createInfo)
      : RenderPassCreateInfo(std::move(createInfo)),
        UniqueVulkanObject<VkRenderPass>(device, info()) {}
};

} // namespace vkw
#endif // VKRENDERER_RENDERPASS_HPP
