#ifndef VKWRAPPER_VALIDATION_HPP
#define VKWRAPPER_VALIDATION_HPP

#include "Extensions.hpp"
#include "Layers.hpp"
#include <functional>

namespace vkw::debug {
class Validation {
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
                  void *pUserData) const;

  virtual ~Validation();
};

} // namespace vkw::debug
#endif // VKWRAPPER_VALIDATION_HPP
