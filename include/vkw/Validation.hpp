#ifndef VKWRAPPER_VALIDATION_HPP
#define VKWRAPPER_VALIDATION_HPP

#include <vkw/Extensions.hpp>
#include <vkw/Layers.hpp>
#include <vkw/ReferenceGuard.hpp>

#include <functional>

namespace vkw::debug {

enum class MsgSeverity : unsigned {
  Verbose = 0x1,
  Info = 0x2,
  Warning = 0x4,
  Error = 0x8,
};

inline unsigned operator|(MsgSeverity a, MsgSeverity b) {
  return static_cast<unsigned>(a) | static_cast<unsigned>(b);
}

inline unsigned operator|(unsigned a, MsgSeverity b) {
  return static_cast<unsigned>(a) | static_cast<unsigned>(b);
}

inline unsigned operator|(MsgSeverity a, unsigned b) {
  return static_cast<unsigned>(a) | static_cast<unsigned>(b);
}

inline unsigned operator&(MsgSeverity a, MsgSeverity b) {
  return static_cast<unsigned>(a) & static_cast<unsigned>(b);
}

inline unsigned operator&(unsigned a, MsgSeverity b) {
  return static_cast<unsigned>(a) & static_cast<unsigned>(b);
}

inline unsigned operator&(MsgSeverity a, unsigned b) {
  return static_cast<unsigned>(a) & static_cast<unsigned>(b);
}

class Validation : public ReferenceGuard {
public:
  unsigned messageSeverityFilter = MsgSeverity::Verbose | MsgSeverity::Info |
                                   MsgSeverity::Warning | MsgSeverity::Error;

  struct Message {
    // This corresponds to VkDebugUtilsMessengerCallbackDataEXT
    int id;
    std::string_view name;
    std::string_view what;
  };

  explicit Validation(Instance const &instance) noexcept(ExceptionsDisabled);

  Validation(Validation const &another) noexcept(ExceptionsDisabled);
  Validation(Validation &&another) noexcept;
  Validation &operator=(Validation const &another) noexcept(ExceptionsDisabled);
  Validation &operator=(Validation &&another) noexcept;

  virtual void onValidationMessage(MsgSeverity severity,
                                   Message const &message) {}

  virtual void onPerformanceMessage(MsgSeverity severity,
                                    Message const &message) {}

  virtual void onGeneralMessage(MsgSeverity severity, Message const &message) {}

  virtual ~Validation();
};

} // namespace vkw::debug
#endif // VKWRAPPER_VALIDATION_HPP
