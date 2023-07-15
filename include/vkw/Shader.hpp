#ifndef VKRENDERER_SHADER_HPP
#define VKRENDERER_SHADER_HPP

#include <vkw/Device.hpp>
#include <vkw/SPIRVModule.hpp>

#include <functional>

namespace vkw {

class Device;

class ShaderBase : public UniqueVulkanObject<VkShaderModule> {
public:
  ShaderBase(Device const &device, SPIRVModule const &module,
             VkShaderStageFlagBits stage,
             VkShaderModuleCreateFlags flags = 0) noexcept(ExceptionsDisabled);

  VkShaderStageFlagBits stage() const noexcept { return m_stage; }

  /// Checks that module has only one entry point and its stage
  /// is expectedStage.
  /// @throw BAD_SHADER_MODULE if conditions are unmet.
  static SPIRVModule const &
  checkModule(SPIRVModule const &module,
              VkShaderStageFlagBits expectedStage) noexcept(ExceptionsDisabled);

  auto &info() noexcept { return *m_info; }

private:
  VkShaderStageFlagBits m_stage;
  std::shared_ptr<SPIRVModuleInfo> m_info;
};

template <VkShaderStageFlagBits STAGE> class Shader : public ShaderBase {
public:
  Shader(Device const &device, SPIRVModule const &module,
         VkShaderModuleCreateFlags flags = 0) noexcept(ExceptionsDisabled)
      : ShaderBase(device, ShaderBase::checkModule(module, STAGE), STAGE,
                   flags){};
};

using FragmentShader = Shader<VK_SHADER_STAGE_FRAGMENT_BIT>;
using VertexShader = Shader<VK_SHADER_STAGE_VERTEX_BIT>;
using ComputeShader = Shader<VK_SHADER_STAGE_COMPUTE_BIT>;

} // namespace vkw
#endif // VKRENDERER_SHADER_HPP
