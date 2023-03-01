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
             VkShaderStageFlagBits stage, VkShaderModuleCreateFlags flags = 0);

  VkShaderStageFlagBits stage() const { return m_stage; }

private:
  VkShaderStageFlagBits m_stage;
};

class FragmentShader : public ShaderBase {
public:
  FragmentShader(Device const &device, SPIRVModule const &module,
                 VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, module, VK_SHADER_STAGE_FRAGMENT_BIT, flags) {}
};

class VertexShader : public ShaderBase {
public:
  VertexShader(Device const &device, SPIRVModule const &module,
               VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, module, VK_SHADER_STAGE_VERTEX_BIT, flags) {}
};

class ComputeShader : public ShaderBase {
public:
  ComputeShader(Device const &device, SPIRVModule const &module,
                VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, module, VK_SHADER_STAGE_COMPUTE_BIT, flags) {}
};

} // namespace vkw
#endif // VKRENDERER_SHADER_HPP
