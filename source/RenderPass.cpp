#include "RenderPass.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

bool AttachmentDescription::isDepthStencil() const {
  switch (m_description.format) {
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
  case VK_FORMAT_D32_SFLOAT:
  case VK_FORMAT_D16_UNORM:
  case VK_FORMAT_D16_UNORM_S8_UINT:
  case VK_FORMAT_D24_UNORM_S8_UINT:
  case VK_FORMAT_X8_D24_UNORM_PACK32:
  case VK_FORMAT_S8_UINT:
    return true;
  default:
    return false;
  }
}
bool AttachmentDescription::formatHasDepthAspect() const {
  switch (m_description.format) {
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
  case VK_FORMAT_D32_SFLOAT:
  case VK_FORMAT_D16_UNORM:
  case VK_FORMAT_D16_UNORM_S8_UINT:
  case VK_FORMAT_D24_UNORM_S8_UINT:
  case VK_FORMAT_X8_D24_UNORM_PACK32:
    return true;
  default:
    return false;
  }
}
bool AttachmentDescription::formatHasStencilAspect() const {
  switch (m_description.format) {
  case VK_FORMAT_D32_SFLOAT_S8_UINT:
  case VK_FORMAT_D16_UNORM_S8_UINT:
  case VK_FORMAT_D24_UNORM_S8_UINT:
  case VK_FORMAT_S8_UINT:
    return true;
  default:
    return false;
  }
}
bool AttachmentDescription::isColor() const { return !isDepthStencil(); }

void SubpassDescription::addInputAttachment(
    AttachmentDescription const &attachment, VkImageLayout layout) {
  m_inputAttachments.emplace_back(attachment, layout);
}
void SubpassDescription::addColorAttachment(
    AttachmentDescription const &attachment, VkImageLayout layout) {
  m_colorAttachments.emplace_back(attachment, layout);
}
void SubpassDescription::addDepthAttachment(
    AttachmentDescription const &attachment, VkImageLayout layout) {
  m_depthAttachment = std::make_pair(attachment, layout);
}
void SubpassDescription::addPreserveAttachment(
    AttachmentDescription const &attachment) {
  m_preserveAttachments.emplace_back(attachment);
}
RenderPass::RenderPass(Device &device, RenderPassCreateInfo createInfo)
    : m_parent(device),
      m_createInfo(std::move(createInfo)){
          VK_CHECK_RESULT(m_parent.core_1_0().vkCreateRenderPass(
              m_parent, &m_createInfo.operator const VkRenderPassCreateInfo &(),
              nullptr, &m_renderPass))}

      RenderPass::~RenderPass() {
  if (m_renderPass == VK_NULL_HANDLE)
    return;

  m_parent.core_1_0().vkDestroyRenderPass(m_parent, m_renderPass, nullptr);
}

RenderPassCreateInfo::RenderPassCreateInfo(RenderPassCreateInfo &&another)
    : m_createInfo(another.m_createInfo),
      m_attachments(std::move(another.m_attachments)),
      m_subpasses(another.m_subpasses),
      m_subpassesDescs(another.m_subpassesDescs),
      m_dependencies(another.m_dependencies) {
  m_createInfo.pSubpasses = m_subpasses.data();
  m_createInfo.pAttachments = m_attachments;
  m_createInfo.pDependencies = m_dependencies.data();
  auto descIter = m_subpassesDescs.begin();
  for (auto &subpass : m_subpasses) {
    subpass.pInputAttachments = descIter->inputAttachments.data();
    subpass.pColorAttachments = descIter->colorAttachments.data();
    subpass.pDepthStencilAttachment = descIter->depthAttachment.has_value()
                                          ? &descIter->depthAttachment.value()
                                          : nullptr;
    subpass.pPreserveAttachments = descIter->preserveAttachments.data();
  }
}

RenderPassCreateInfo::RenderPassCreateInfo(
    AttachmentDescriptionConstRefArray attachments,
    const std::vector<SubpassDescriptionCRef> &subpasses,
    const std::vector<SubpassDependency> &dependencies,
    VkRenderPassCreateFlags flags)
    : m_attachments(std::move(attachments)) {

  for (auto &subpass : subpasses) {
    M_subpassDesc subpassDesc{};
    VkSubpassDescription desc{};
    desc.flags = subpass.get().flags;
    desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    for (auto &inputAttachment : subpass.get().inputAttachments()) {
      auto found = std::find_if(
          attachments.begin(), attachments.end(),
          [&inputAttachment](AttachmentDescriptionCRef const &attachment) {
            return &attachment.get() == &inputAttachment.first.get();
          });
      if (found == attachments.end()) {
        throw Error("Subpass referenced unknown input attachment");
      }
      subpassDesc.inputAttachments.emplace_back(VkAttachmentReference{
          static_cast<uint32_t>(found - attachments.begin()),
          inputAttachment.second});
    }

    for (auto &colorAttachment : subpass.get().colorAttachments()) {
      auto found = std::find_if(
          attachments.begin(), attachments.end(),
          [&colorAttachment](AttachmentDescriptionCRef const &attachment) {
            return &attachment.get() == &colorAttachment.first.get();
          });
      if (found == attachments.end()) {
        throw Error("Subpass referenced unknown color attachment");
      }
      if (!found->get().isColor()) {
        throw Error("Subpass referenced color attachment with bad format");
      }
      subpassDesc.colorAttachments.emplace_back(VkAttachmentReference{
          static_cast<uint32_t>(found - attachments.begin()),
          colorAttachment.second});
    }

    for (auto &preserveAttachment : subpass.get().preserveAttachments()) {
      auto found = std::find_if(
          attachments.begin(), attachments.end(),
          [&preserveAttachment](AttachmentDescriptionCRef const &attachment) {
            return &attachment.get() == &preserveAttachment.get();
          });
      if (found == attachments.end()) {
        throw Error("Subpass referenced unknown attachment to preserve");
      }
      subpassDesc.preserveAttachments.emplace_back(
          static_cast<uint32_t>(found - attachments.begin()));
    }
    if (subpass.get().depthAttachments().has_value()) {
      auto &depthAttachment = subpass.get().depthAttachments().value();

      auto found = std::find_if(
          attachments.begin(), attachments.end(),
          [&depthAttachment](AttachmentDescriptionCRef const &attachment) {
            return &attachment.get() == &depthAttachment.first.get();
          });
      if (found == attachments.end()) {
        throw Error("Subpass referenced unknown depth/stencil attachment");
      }
      if (!found->get().isDepthStencil()) {
        throw Error(
            "Subpass referenced depth/stencil attachment with bad format");
      }
      subpassDesc.depthAttachment.emplace(VkAttachmentReference{
          static_cast<uint32_t>(found - attachments.begin()),
          depthAttachment.second});
    }
    desc.inputAttachmentCount = subpassDesc.inputAttachments.size();
    desc.pInputAttachments = subpassDesc.inputAttachments.data();
    desc.colorAttachmentCount = subpassDesc.colorAttachments.size();
    desc.pColorAttachments = subpassDesc.colorAttachments.data();
    desc.preserveAttachmentCount = subpassDesc.preserveAttachments.size();
    desc.pPreserveAttachments = subpassDesc.preserveAttachments.data();
    desc.pDepthStencilAttachment = subpassDesc.depthAttachment.has_value()
                                       ? &subpassDesc.depthAttachment.value()
                                       : nullptr;
    desc.pResolveAttachments = nullptr; // TODO: support MSAA

    m_subpassesDescs.emplace_back(std::move(subpassDesc));
    m_subpasses.emplace_back(desc);
  }

  for (auto &dependency : dependencies) {
    VkSubpassDependency dep{};

    if (dependency.srcSubpass().has_value()) {
      auto srcSubpassRef = dependency.srcSubpass().value();
      auto found_src =
          std::find_if(subpasses.begin(), subpasses.end(),
                       [&srcSubpassRef](SubpassDescriptionCRef const &subpass) {
                         return &subpass.get() == &srcSubpassRef.get();
                       });
      if (found_src == subpasses.end()) {
        throw Error("Subpass Dependency referenced bad subpass");
      } else {
        dep.srcSubpass = found_src - subpasses.begin();
      }
    } else {
      dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    }

    if (dependency.dstSubpass().has_value()) {
      auto dstSubpassRef = dependency.dstSubpass().value();
      auto found_dst =
          std::find_if(subpasses.begin(), subpasses.end(),
                       [&dstSubpassRef](SubpassDescriptionCRef const &subpass) {
                         return &subpass.get() == &dstSubpassRef.get();
                       });
      if (found_dst == subpasses.end()) {
        throw Error("Subpass Dependency referenced bad subpass");
      } else {
        dep.dstSubpass = found_dst - subpasses.begin();
      }
    } else {
      dep.dstSubpass = VK_SUBPASS_EXTERNAL;
    }

    if (dep.srcSubpass == VK_SUBPASS_EXTERNAL &&
        dep.dstSubpass == VK_SUBPASS_EXTERNAL)
      throw Error(
          "Dependency can't have both SRC and DST subpasses be EXTERNAL");

    dep.srcAccessMask = dependency.srcAccessMask;
    dep.dstAccessMask = dependency.dstAccessMask;
    dep.srcStageMask = dependency.srcStageMask;
    dep.dstStageMask = dependency.dstStageMask;
    dep.dependencyFlags = dependency.dependencyFlags;
    m_dependencies.push_back(dep);
  }

  m_createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  m_createInfo.pNext = nullptr;
  m_createInfo.attachmentCount = m_attachments.size();
  m_createInfo.pAttachments = m_attachments;
  m_createInfo.subpassCount = m_subpasses.size();
  m_createInfo.pSubpasses = m_subpasses.data();
  m_createInfo.dependencyCount = m_dependencies.size();
  m_createInfo.pDependencies = m_dependencies.data();
  m_createInfo.flags = flags;
}
} // namespace vkw