#ifndef VKRENDERER_SHADER_HPP
#define VKRENDERER_SHADER_HPP

#include "vkw/Device.hpp"
#include "vkw/SPIRVModule.hpp"
#include <functional>
#include <vulkan/vulkan.h>

namespace vkw {

class Device;

class ShaderBase : public ReferenceGuard {
public:
  ShaderBase(Device &device, SPIRVModule const &module,
             VkShaderStageFlagBits stage, VkShaderModuleCreateFlags flags = 0);
  ShaderBase(ShaderBase &&another)
      : m_device(another.m_device), m_shader(another.m_shader),
        m_stage(another.m_stage) {
    another.m_shader = VK_NULL_HANDLE;
  }

  ShaderBase &operator=(ShaderBase &&another) noexcept {
    m_device = another.m_device;
    m_stage = another.m_stage;
    std::swap(m_shader, another.m_shader);

    return *this;
  }

  VkShaderStageFlagBits stage() const { return m_stage; }

  virtual ~ShaderBase();

  operator VkShaderModule() const { return m_shader; }

private:
  StrongReference<Device> m_device;
  VkShaderStageFlagBits m_stage;
  VkShaderModule m_shader = VK_NULL_HANDLE;
};

class FragmentShader : public ShaderBase {
public:
  FragmentShader(Device &device, SPIRVModule const &module,
                 VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, module, VK_SHADER_STAGE_FRAGMENT_BIT, flags) {}
};

class VertexShader : public ShaderBase {
public:
  VertexShader(Device &device, SPIRVModule const &module,
               VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, module, VK_SHADER_STAGE_VERTEX_BIT, flags) {}
};

class ComputeShader : public ShaderBase {
public:
  ComputeShader(Device &device, SPIRVModule const &module,
                VkShaderModuleCreateFlags flags = 0)
      : ShaderBase(device, module, VK_SHADER_STAGE_COMPUTE_BIT, flags) {}
};

} // namespace vkw
#endif // VKRENDERER_SHADER_HPP
