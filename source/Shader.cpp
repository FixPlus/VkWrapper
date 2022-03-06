#include "Shader.hpp"
#include "Device.hpp"
#include "Utils.hpp"

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
  VK_CHECK_RESULT(m_device.core_1_0().vkCreateShaderModule(
      m_device, &createInfo, nullptr, &m_shader))
}

ShaderBase::~ShaderBase() {
  if (m_shader == VK_NULL_HANDLE)
    return;
  m_device.core_1_0().vkDestroyShaderModule(m_device, m_shader, nullptr);
}
} // namespace vkw