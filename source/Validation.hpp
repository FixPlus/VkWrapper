#ifndef VKRENDERER_VALIDATION_H
#define VKRENDERER_VALIDATION_H

#include "vulkan/vulkan.h"

namespace vkw {

namespace debug {
// Default validation layers
extern int validationLayerCount;
extern const char *validationLayerNames[];

VKAPI_ATTR VkBool32 VKAPI_CALL
messageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
                uint64_t srcObject, size_t location, int32_t msgCode,
                const char *pLayerPrefix, const char *pMsg, void *pUserData);

void setupDebugging(VkInstance instance, VkDebugReportFlagsEXT flags,
                    VkDebugReportCallbackEXT callBack);
void freeDebugCallback(VkInstance instance);
} // namespace debug

} // namespace vkr
#endif // VKRENDERER_VALIDATION_H
