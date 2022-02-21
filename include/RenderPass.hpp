#ifndef VKRENDERER_RENDERPASS_HPP
#define VKRENDERER_RENDERPASS_HPP

#include <vulkan/vulkan.h>

namespace vkw {

class Device;

class RenderPass {
public:
  RenderPass(Device &device, VkRenderPassCreateInfo const &createInfo);

  RenderPass(RenderPass &&another) noexcept
      : m_parent(another.m_parent), m_createInfo(another.m_createInfo),
        m_renderPass(another.m_renderPass) {
    another.m_renderPass = VK_NULL_HANDLE;
  }

  virtual ~RenderPass();

  operator VkRenderPass() const { return m_renderPass; }

private:
  Device &m_parent;
  VkRenderPassCreateInfo m_createInfo;
  VkRenderPass m_renderPass = VK_NULL_HANDLE;
};

} // namespace vkr
#endif // VKRENDERER_RENDERPASS_HPP
