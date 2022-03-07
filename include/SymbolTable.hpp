#ifndef VKWRAPPER_SYMBOLTABLE_HPP
#define VKWRAPPER_SYMBOLTABLE_HPP

#include <concepts>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace vkw {

template <typename Level>
requires std::same_as<Level, VkInstance> || std::same_as<Level, VkDevice>
class SymbolTableBase {
public:
  using PFN_getProcAddr = PFN_vkVoidFunction (*)(Level, const char *);

  SymbolTableBase(PFN_getProcAddr getProcAddr, Level level)
      : m_level(level), m_getProcAddr(getProcAddr){};

protected:
  template <typename FuncType>
  FuncType getProcAddrT(std::string const &funcName) {
    return reinterpret_cast<FuncType>(m_getProcAddr(m_level, funcName.c_str()));
  }

private:
  PFN_getProcAddr m_getProcAddr;
  Level m_level;
};

template <uint32_t major, uint32_t minor>
class InstanceCore : public SymbolTableBase<VkInstance> {};

template <uint32_t major, uint32_t minor>
class DeviceCore : public SymbolTableBase<VkInstance> {};

class InstanceExtensionBase : public SymbolTableBase<VkInstance> {
public:
  InstanceExtensionBase(PFN_getProcAddr getProcAddr, VkInstance instance,
                        std::string const &extName)
      : SymbolTableBase<VkInstance>(getProcAddr, instance), m_name(extName) {}

  std::string const &name() const { return m_name; }

private:
  std::string m_name;
};

class InstanceExtensionInitializerBase {
public:
  InstanceExtensionInitializerBase() = default;

  virtual InstanceExtensionBase *
  initialize(PFN_vkGetInstanceProcAddr getProcAddr,
             VkInstance instance) const = 0;
};

template <typename T>
class InstanceExtensionInitializerImpl
    : public InstanceExtensionInitializerBase {
public:
  InstanceExtensionBase *initialize(PFN_vkGetInstanceProcAddr getProcAddr,
                                    VkInstance instance) const override {
    return new T(getProcAddr, instance);
  };
};

class DeviceExtensionBase : public SymbolTableBase<VkDevice> {
public:
  DeviceExtensionBase(PFN_getProcAddr getProcAddr, VkDevice instance,
                      std::string const &extName)
      :

        SymbolTableBase<VkDevice>(getProcAddr, instance), m_name(extName) {}

  std::string const &name() const { return m_name; }

private:
  std::string m_name;
};

class DeviceExtensionInitializerBase {
public:
  DeviceExtensionInitializerBase() = default;

  virtual DeviceExtensionBase *initialize(PFN_vkGetDeviceProcAddr getProcAddr,
                                          VkDevice instance) const = 0;
};

template <typename T>
class DeviceExtensionInitializerImpl : public DeviceExtensionInitializerBase {
public:
  DeviceExtensionBase *initialize(PFN_vkGetDeviceProcAddr getProcAddr,
                                  VkDevice device) const override {
    return new T(getProcAddr, device);
  };
};

#include "SymbolTable.inc"

} // namespace vkw
#endif // VKWRAPPER_SYMBOLTABLE_HPP
