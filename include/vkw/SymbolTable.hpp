#ifndef VKWRAPPER_SYMBOLTABLE_HPP
#define VKWRAPPER_SYMBOLTABLE_HPP

#include <concepts>
#include <string>
#include <unordered_map>
#include <vulkan/vulkan.h>

namespace vkw {

class Instance;
class Device;

template <typename Level>
requires std::same_as<Level, VkInstance> || std::same_as<Level, VkDevice>
class SymbolTableBase {
public:
  using PFN_getProcAddr = PFN_vkVoidFunction (*)(Level, const char *);

  SymbolTableBase(PFN_getProcAddr getProcAddr, Level level)
      : m_level(level), m_getProcAddr(getProcAddr){};

  virtual ~SymbolTableBase() = default;
protected:
  template <typename FuncType>
  FuncType getProcAddrT(std::string const &funcName) {
    return reinterpret_cast<FuncType>(m_getProcAddr(m_level, funcName.c_str()));
  }

private:
  PFN_getProcAddr m_getProcAddr;
  Level m_level;
};

template <uint32_t major, uint32_t minor> class InstanceCore {};

template <uint32_t major, uint32_t minor> class DeviceCore {};

#define VKW_DUMP_CORE_CLASSES
#include "SymbolTable.inc"
#undef VKW_DUMP_CORE_CLASSES

} // namespace vkw
#endif // VKWRAPPER_SYMBOLTABLE_HPP
