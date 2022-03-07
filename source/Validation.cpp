#include "Validation.hpp"

#include "Instance.hpp"
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>

namespace vkw {
namespace debug {
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
VkDebugUtilsMessengerEXT debugUtilsMessenger;

VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessengerCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  VkBool32 result = VK_FALSE;
  std::string prefix("");

  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    prefix = "VERBOSE: ";
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    prefix = "INFO: ";
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    prefix = "WARNING: ";
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    prefix = "ERROR: ";
    result = VK_TRUE;
  }

  // Display message to default output (console/logcat)
  std::stringstream debugMessage;
  debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "]["
               << pCallbackData->pMessageIdName
               << "] : " << pCallbackData->pMessage;

  if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    std::cerr << debugMessage.str() << "\n";
  } else {
    std::cout << debugMessage.str() << "\n";
  }
  fflush(stdout);

  return result;
}

void setupDebugging(Instance const &instance, VkDebugReportFlagsEXT flags,
                    VkDebugReportCallbackEXT callBack) {

  VkExtDebugUtils const *debugUtilsExt = static_cast<VkExtDebugUtils const *>(
      instance.getExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME));

  assert(debugUtilsExt && "VK_EXT_debug_utils must be present here");

  vkCreateDebugUtilsMessengerEXT =
      debugUtilsExt->vkCreateDebugUtilsMessengerEXT;
  vkDestroyDebugUtilsMessengerEXT =
      debugUtilsExt->vkDestroyDebugUtilsMessengerEXT;

  VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
  debugUtilsMessengerCI.sType =
      VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  debugUtilsMessengerCI.messageSeverity =
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  debugUtilsMessengerCI.messageType =
      VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
      VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
  debugUtilsMessengerCI.pfnUserCallback = debugUtilsMessengerCallback;
  VkResult result = vkCreateDebugUtilsMessengerEXT(
      instance, &debugUtilsMessengerCI, nullptr, &debugUtilsMessenger);
  assert(result == VK_SUCCESS);
}

void freeDebugCallback(VkInstance instance) {
  if (debugUtilsMessenger != VK_NULL_HANDLE) {
    vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
  }
}
} // namespace debug

} // namespace vkw