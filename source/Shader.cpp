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

std::string_view shaderStageStr(VkShaderStageFlagBits stage) {
  switch (stage) {
#define CASE(X)                                                                \
  case (X):                                                                    \
    return #X;
    CASE(VK_SHADER_STAGE_VERTEX_BIT)
    CASE(VK_SHADER_STAGE_FRAGMENT_BIT)
    CASE(VK_SHADER_STAGE_COMPUTE_BIT)
  default:
    return "BAD_STAGE_ID";
  }
}

} // namespace

SPIRVModule const &ShaderBase::checkModule(
    SPIRVModule const &module,
    VkShaderStageFlagBits expectedStage) noexcept(ExceptionsDisabled) {
  auto &info = module.info();
  if (info.entryPoints().size() != 1)
    postError(
        vkw::Error{[&]() {
                     std::stringstream ss;
                     ss << "Bad shader module: unexpected entry point count("
                        << info.entryPoints().size() << "). Expected 1.";
                     return ss.str();
                   }(),
                   ErrorCode::BAD_SHADER_MODULE});

  auto moduleStage = (*info.entryPoints().begin()).stage();
  if (moduleStage != expectedStage)
    postError(vkw::Error{[&]() {
                           std::stringstream ss;
                           ss << "Bad shader module: shader stage mismatch.\n";
                           ss << "  Expected: " << shaderStageStr(expectedStage)
                              << "\n";
                           ss << "  Got: " << shaderStageStr(moduleStage)
                              << "\n";
                           return ss.str();
                         }(),
                         ErrorCode::BAD_SHADER_MODULE});
  return module;
}

ShaderBase::ShaderBase(
    Device const &device, SPIRVModule const &module,
    VkShaderStageFlagBits stage,
    VkShaderModuleCreateFlags flags) noexcept(ExceptionsDisabled)
    : UniqueVulkanObject<VkShaderModule>(device,
                                         fillCreateInfo(module, stage, flags)),
      m_stage(stage), m_info(module.takeInfo()) {}
} // namespace vkw