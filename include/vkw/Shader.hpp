#ifndef VKRENDERER_SHADER_HPP
#define VKRENDERER_SHADER_HPP

#include <vulkan/vulkan.h>

namespace vkw {

class Device;

class ShaderBase {
public:
  ShaderBase(Device &device, size_t codeSize, uint32_t *pCode,
             VkShaderStageFlagBits stage, VkShaderModuleCreateFlags flags = 0);
  ShaderBase(ShaderBase &&another)
      : m_device(another.m_device), m_shader(another.m_shader),
        m_stage(another.m_stage) {
    another.m_shader = VK_NULL_HANDLE;
  }

  VkShaderStageFlagBits stage() const { return m_stage; }

  virtual ~ShaderBase();

  operator VkShaderModule() const { return m_shader; }

private:
  Device &m_device;
  VkShaderStageFlagBits m_stage;
  VkShaderModule m_shader = VK_NULL_HANDLE;
};

class FragmentShader : public ShaderBase {
public:
  FragmentShader(Device &device, size_t codeSize, uint32_t *pCode,
                 VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, codeSize, pCode, VK_SHADER_STAGE_FRAGMENT_BIT,
                   flags) {}
};

class VertexShader : public ShaderBase {
public:
  VertexShader(Device &device, size_t codeSize, uint32_t *pCode,
               VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, codeSize, pCode, VK_SHADER_STAGE_VERTEX_BIT, flags) {
  }
};

class ComputeShader : public ShaderBase {
public:
  ComputeShader(Device &device, size_t codeSize, uint32_t *pCode,
                VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, codeSize, pCode, VK_SHADER_STAGE_COMPUTE_BIT,
                   flags) {}
};

} // namespace vkw
#endif // VKRENDERER_SHADER_HPP
