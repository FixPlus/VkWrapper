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

class InstanceCore_1_0 : public SymbolTableBase<VkInstance> {
public:
  InstanceCore_1_0(PFN_getProcAddr getProcAddr, VkInstance instance)
      :
#define STRINGIFY(X) #X

#define VKW_VK_INSTANCE_FUNCTION_1_0(X)                                        \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        SymbolTableBase<VkInstance>(getProcAddr, instance){};

#define VKW_VK_INSTANCE_FUNCTION_1_0(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"
};

class InstanceCore_1_1 : public InstanceCore_1_0 {
public:
  InstanceCore_1_1(PFN_getProcAddr getProcAddr, VkInstance instance)
      :
#define STRINGIFY(X) #X

#define VKW_VK_INSTANCE_FUNCTION_1_1(X)                                        \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        InstanceCore_1_0(getProcAddr, instance){};

#define VKW_VK_INSTANCE_FUNCTION_1_1(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"
};

class DeviceCore_1_0 : public SymbolTableBase<VkDevice> {
public:
  DeviceCore_1_0(PFN_getProcAddr getProcAddr, VkDevice device)
      :
#define STRINGIFY(X) #X

#define VKW_VK_DEVICE_FUNCTION_1_0(X)                                          \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        SymbolTableBase<VkDevice>(getProcAddr, device){};

#define VKW_VK_DEVICE_FUNCTION_1_0(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"
};

class DeviceCore_1_1 : public DeviceCore_1_0 {
public:
  DeviceCore_1_1(PFN_getProcAddr getProcAddr, VkDevice device)
      :
#define STRINGIFY(X) #X

#define VKW_VK_DEVICE_FUNCTION_1_1(X)                                          \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        DeviceCore_1_0(getProcAddr, device){};

#define VKW_VK_DEVICE_FUNCTION_1_1(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"
};

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

class VkSurfaceKHRExtension : public InstanceExtensionBase {
public:
  VkSurfaceKHRExtension(PFN_getProcAddr getProcAddr, VkInstance instance)
      :
#define STRINGIFY(X) #X

#define VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION(X)                             \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        InstanceExtensionBase(getProcAddr, instance,
                              VK_KHR_SURFACE_EXTENSION_NAME) {
  }

#define VKW_VK_INSTANCE_VK_KHR_surface_FUNCTION(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"

  constexpr static const InstanceExtensionInitializerImpl<VkSurfaceKHRExtension>
      m_initializer{};
};

#ifdef _WIN32
class VkSurfaceWin32KHRExtension : public InstanceExtensionBase {
public:
  VkSurfaceWin32KHRExtension(PFN_getProcAddr getProcAddr, VkInstance instance)
      :
#define STRINGIFY(X) #X

#define VKW_VK_INSTANCE_VK_KHR_win32_surface_FUNCTION(X)                       \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        InstanceExtensionBase(getProcAddr, instance,
                              VK_KHR_WIN32_SURFACE_EXTENSION_NAME) {
  }

#define VKW_VK_INSTANCE_VK_KHR_win32_surface_FUNCTION(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"

  constexpr static const InstanceExtensionInitializerImpl<
      VkSurfaceWin32KHRExtension>
      m_initializer{};
};

#endif

class VkDebugUtilsEXTExtension : public InstanceExtensionBase {
public:
  VkDebugUtilsEXTExtension(PFN_getProcAddr getProcAddr, VkInstance instance)
      :
#define STRINGIFY(X) #X

#define VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION(X)                         \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        InstanceExtensionBase(getProcAddr, instance,
                              VK_EXT_DEBUG_UTILS_EXTENSION_NAME) {
  }

#define VKW_VK_INSTANCE_VK_EXT_debug_utils_FUNCTION(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"

  constexpr static const InstanceExtensionInitializerImpl<
      VkDebugUtilsEXTExtension>
      m_initializer{};
};

class VkSwapchainKHRExtension : public DeviceExtensionBase {
public:
  VkSwapchainKHRExtension(PFN_getProcAddr getProcAddr, VkDevice device)
      :
#define STRINGIFY(X) #X

#define VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(X)                             \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        DeviceExtensionBase(getProcAddr, device,
                            VK_KHR_SWAPCHAIN_EXTENSION_NAME) {
  }

#define VKW_VK_DEVICE_VK_KHR_swapchain_FUNCTION(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"

  constexpr static const DeviceExtensionInitializerImpl<VkSwapchainKHRExtension>
      m_initializer{};
};

class VkDebugMarkerEXTExtension : public DeviceExtensionBase {
public:
  VkDebugMarkerEXTExtension(PFN_getProcAddr getProcAddr, VkDevice device)
      :
#define STRINGIFY(X) #X

#define VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(X)                          \
  vk##X(getProcAddrT<PFN_vk##X>(STRINGIFY(vk##X))),
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
        DeviceExtensionBase(getProcAddr, device,
                            VK_EXT_DEBUG_MARKER_EXTENSION_NAME) {
  }

#define VKW_VK_DEVICE_VK_EXT_debug_marker_FUNCTION(X) PFN_vk##X vk##X;
#include "VulkanFunctionList.hpp"

  constexpr static const DeviceExtensionInitializerImpl<
      VkDebugMarkerEXTExtension>
      m_initializer{};
};

const std::unordered_map<std::string, InstanceExtensionInitializerBase const *>
    m_instanceExtInitializers{
#define VKW_MAP_ENTRY(X, Y) {X, &Y::m_initializer},
        VKW_MAP_ENTRY(VK_KHR_SURFACE_EXTENSION_NAME, VkSurfaceKHRExtension)
#ifdef _WIN32
            VKW_MAP_ENTRY(VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
                          VkSurfaceWin32KHRExtension)
#endif
                VKW_MAP_ENTRY(VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                              VkDebugUtilsEXTExtension)
#undef VKW_MAP_ENTRY
    };

const std::unordered_map<std::string, DeviceExtensionInitializerBase const *>
    m_deviceExtInitializers{
#define VKW_MAP_ENTRY(X, Y) {X, &Y::m_initializer},
        //#define VKW_MAP_ENTRY(X, Y) {X,
        //static_cast<DeviceExtensionInitializerBase const*>(new
        //DeviceExtensionInitializerImpl<Y>)},
        VKW_MAP_ENTRY(VK_KHR_SWAPCHAIN_EXTENSION_NAME, VkSwapchainKHRExtension)
            VKW_MAP_ENTRY(VK_EXT_DEBUG_MARKER_EXTENSION_NAME,
                          VkDebugMarkerEXTExtension)
#undef VKW_MAP_ENTRY
    };

} // namespace vkw
#endif // VKWRAPPER_SYMBOLTABLE_HPP
