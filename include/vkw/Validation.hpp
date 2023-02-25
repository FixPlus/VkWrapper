#ifndef VKWRAPPER_VALIDATION_HPP
#define VKWRAPPER_VALIDATION_HPP

#include "vkw/Extensions.hpp"
#include "vkw/Layers.hpp"
#include "vkw/ReferenceGuard.hpp"
#include <functional>

namespace vkw::debug {
class Validation : public ReferenceGuard {
public:
  explicit Validation(Instance const &instance);

  Validation(Validation const &another);
  Validation(Validation &&another) noexcept;
  Validation &operator=(Validation const &another);
  Validation &operator=(Validation &&another);

  virtual bool
  messageCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData);

  virtual ~Validation();
};

} // namespace vkw::debug
#endif // VKWRAPPER_VALIDATION_HPP
