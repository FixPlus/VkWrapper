#ifndef VKRENDERER_RENDERPASS_HPP
#define VKRENDERER_RENDERPASS_HPP

#include <Common.hpp>
#include <optional>
#include <vulkan/vulkan.h>

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

  std::vector<std::pair<AttachmentDescriptionCRef, VkImageLayout>> const &
  inputAttachments() const {
    return m_inputAttachments;
  }

  std::vector<std::pair<AttachmentDescriptionCRef, VkImageLayout>> const &
  colorAttachments() const {
    return m_colorAttachments;
  }
  std::optional<std::pair<AttachmentDescriptionCRef, VkImageLayout>> const &
  depthAttachments() const {
    return m_depthAttachment;
  }
  std::vector<AttachmentDescriptionCRef> const &preserveAttachments() const {
    return m_preserveAttachments;
  }

  VkDependencyFlags flags;

private:
  std::vector<std::pair<AttachmentDescriptionCRef, VkImageLayout>>
      m_inputAttachments;
  std::vector<std::pair<AttachmentDescriptionCRef, VkImageLayout>>
      m_colorAttachments;
  std::optional<std::pair<AttachmentDescriptionCRef, VkImageLayout>>
      m_depthAttachment;
  std::vector<AttachmentDescriptionCRef> m_preserveAttachments;
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
  RenderPassCreateInfo(AttachmentDescriptionConstRefArray attachments,
                       std::vector<SubpassDescriptionCRef> const &subpasses,
                       std::vector<SubpassDependency> const &dependencies,
                       VkRenderPassCreateFlags flags = 0);

  operator VkRenderPassCreateInfo const &() const { return m_createInfo; }

  uint32_t subpassCount() const { return m_subpasses.size(); }

  AttachmentDescriptionConstRefArray const &attachmentDescriptions() const {
    return m_attachments;
  }

private:
  struct M_subpassDesc {
    std::vector<VkAttachmentReference> inputAttachments;
    std::vector<VkAttachmentReference> colorAttachments;
    std::optional<VkAttachmentReference> depthAttachment;
    std::vector<uint32_t> preserveAttachments;
  };
  AttachmentDescriptionConstRefArray m_attachments;
  std::vector<M_subpassDesc> m_subpassesDescs;
  std::vector<VkSubpassDescription> m_subpasses;
  std::vector<VkSubpassDependency> m_dependencies;

  VkRenderPassCreateInfo m_createInfo;
};

class RenderPass {
public:
  RenderPass(Device &device, RenderPassCreateInfo createInfo);

  RenderPass(RenderPass &&another) noexcept
      : m_parent(another.m_parent), m_createInfo(another.m_createInfo),
        m_renderPass(another.m_renderPass) {
    another.m_renderPass = VK_NULL_HANDLE;
  }

  virtual ~RenderPass();

  RenderPassCreateInfo const &info() const { return m_createInfo; }

  operator VkRenderPass() const { return m_renderPass; }

private:
  Device &m_parent;
  RenderPassCreateInfo m_createInfo;
  VkRenderPass m_renderPass = VK_NULL_HANDLE;
};

} // namespace vkw
#endif // VKRENDERER_RENDERPASS_HPP
