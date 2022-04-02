#include "vkw/Shader.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

ShaderBase::ShaderBase(Device &device, size_t codeSize, uint32_t *pCode,
                       VkShaderStageFlagBits stage,
                       VkShaderModuleCreateFlags flags)
    : m_device(device), m_stage(stage) {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = flags;
  createInfo.codeSize = codeSize;
  createInfo.pCode = pCode;
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkCreateShaderModule(
      m_device.get(), &createInfo, nullptr, &m_shader))
}

ShaderBase::~ShaderBase() {
  if (m_shader == VK_NULL_HANDLE)
    return;
  m_device.get().core<1, 0>().vkDestroyShaderModule(m_device.get(), m_shader,
                                                    nullptr);
}
} // namespace vkw