#include "RenderPass.hpp"
#include "Device.hpp"
#include "Utils.hpp"

namespace vkw {

RenderPass::RenderPass(Device &device, VkRenderPassCreateInfo const &createInfo)
    : m_parent(device),
      m_createInfo(createInfo){VK_CHECK_RESULT(
          vkCreateRenderPass(m_parent, &createInfo, nullptr, &m_renderPass))}

      RenderPass::~RenderPass() {
  if (m_renderPass == VK_NULL_HANDLE)
    return;

  vkDestroyRenderPass(m_parent, m_renderPass, nullptr);
}
} // namespace vkr