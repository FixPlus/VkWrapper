#include "vkw/Shader.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {
namespace {

VkShaderModuleCreateInfo
fillCreateInfo(SPIRVModule const &module, VkShaderStageFlagBits stage,
               VkShaderModuleCreateFlags flags) noexcept {
  VkShaderModuleCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = flags;
  createInfo.codeSize = module.code().size() * 4;
  createInfo.pCode = module.code().data();
  return createInfo;
}

} // namespace
ShaderBase::ShaderBase(
    Device const &device, SPIRVModule const &module,
    VkShaderStageFlagBits stage,
    VkShaderModuleCreateFlags flags) noexcept(ExceptionsDisabled)
    : UniqueVulkanObject<VkShaderModule>(device,
                                         fillCreateInfo(module, stage, flags)),
      m_stage(stage) {}

} // namespace vkw