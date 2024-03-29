#include "vkw/Validation.hpp"

#include "Utils.hpp"
#include "vkw/Extensions.hpp"
#include "vkw/Instance.hpp"
#include <algorithm>
#include <cassert>
#include <memory>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>

namespace vkw::debug {

namespace {

MsgSeverity convert(VkDebugUtilsMessageSeverityFlagBitsEXT severity) {
  if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
    return MsgSeverity::Verbose;
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
    return MsgSeverity::Info;
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    return MsgSeverity::Warning;
  } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    return MsgSeverity::Error;
  } else if (severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    return MsgSeverity::Error;
  } else {
    return MsgSeverity::Verbose;
  }
}
class ValidationImpl final : private Layer<layer::KHRONOS_validation>,
                             private Extension<ext::EXT_debug_utils> {
public:
  ValidationImpl(Instance const &instance) noexcept(ExceptionsDisabled);

  void
  register_validation(Validation &validation) noexcept(ExceptionsDisabled) {
    auto register_guard = std::lock_guard{m_mutex};
    assert(!m_validations.contains(&validation) && "Double registration");

    if (m_validations.empty())
      m_setup();

    m_validations.emplace(&validation);
  }

  void
  unregister_validation(Validation &validation) noexcept(ExceptionsDisabled) {
    auto register_guard = std::lock_guard{m_mutex};
    assert(m_validations.contains(&validation) && "Unknown validation");

    m_validations.erase(&validation);

    if (m_validations.empty())
      m_free();
  }

  static ValidationImpl *handle;

private:
  std::mutex m_mutex;
  std::set<Validation *> m_validations;

  static VKAPI_ATTR VkBool32 VKAPI_CALL
  m_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
             VkDebugUtilsMessageTypeFlagsEXT messageType,
             const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
             void *pUserData) noexcept(ExceptionsDisabled) {
    return handle->m_self_callback(messageSeverity, messageType, pCallbackData,
                                   pUserData);
  }

  VkBool32
  m_self_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                  void *pUserData) noexcept(ExceptionsDisabled) {
    auto callbackGuard = std::lock_guard{m_mutex};
    Validation::Message message{pCallbackData->messageIdNumber,
                                pCallbackData->pMessageIdName,
                                pCallbackData->pMessage};
    auto severity = convert(messageSeverity);
    std::for_each(m_validations.begin(), m_validations.end(),
                  [&](Validation *val) {
                    if (!(val->messageSeverityFilter & severity))
                      return;
                    switch (messageType) {
                    default:
                    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
                      val->onGeneralMessage(severity, message);
                      break;
                    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
                      val->onValidationMessage(severity, message);
                      break;
                    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
                      val->onPerformanceMessage(severity, message);
                      break;
                    }
                  });
    return severity == MsgSeverity::Error;
  }

  void m_setup() noexcept(ExceptionsDisabled) {
    VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
    debugUtilsMessengerCI.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugUtilsMessengerCI.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugUtilsMessengerCI.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
    debugUtilsMessengerCI.pfnUserCallback = m_callback;
    VK_CHECK_RESULT(vkCreateDebugUtilsMessengerEXT(
        m_instance.get(), &debugUtilsMessengerCI,
        m_instance.get().hostAllocator().allocator(), &m_debugUtilsMessenger))
  }

  void m_free() noexcept(ExceptionsDisabled) {
    if (m_debugUtilsMessenger != VK_NULL_HANDLE) {
      vkDestroyDebugUtilsMessengerEXT(
          m_instance.get(), m_debugUtilsMessenger,
          m_instance.get().hostAllocator().allocator());
    }

    m_debugUtilsMessenger = VK_NULL_HANDLE;
  }
  VkDebugUtilsMessengerEXT m_debugUtilsMessenger = VK_NULL_HANDLE;
  // FIXME: Cannot use StrongReference here because ValidationImpl has static
  // storage duration
  std::reference_wrapper<Instance const> m_instance;
};

ValidationImpl *ValidationImpl::handle = nullptr;

ValidationImpl::ValidationImpl(const Instance &instance) noexcept(
    ExceptionsDisabled)
    : Extension(instance), Layer(instance), m_instance(instance) {
  handle = this;
}

void register_callback(Validation &validation,
                       Instance const &instance) noexcept(ExceptionsDisabled) {
  static ValidationImpl impl{instance};
  impl.register_validation(validation);
}

void register_callback_secondary(Validation &validation) noexcept(
    ExceptionsDisabled) {
  assert(ValidationImpl::handle && "handle must be valid here");
  ValidationImpl::handle->register_validation(validation);
}

void unregister_callback(Validation &validation) noexcept(ExceptionsDisabled) {
  assert(ValidationImpl::handle && "handle must be valid here");
  ValidationImpl::handle->unregister_validation(validation);
}
} // namespace

Validation::Validation(const Instance &instance) noexcept(ExceptionsDisabled) {
  register_callback(*this, instance);
}
Validation::~Validation() { unregister_callback(*this); }

Validation::Validation(const Validation &another) noexcept(ExceptionsDisabled) {
  register_callback_secondary(*this);
}
Validation::Validation(Validation &&another) noexcept {
  register_callback_secondary(*this);
}
Validation &
Validation::operator=(const Validation &another) noexcept(ExceptionsDisabled) {
  register_callback_secondary(*this);
  return *this;
}
Validation &Validation::operator=(Validation &&another) noexcept {
  register_callback_secondary(*this);
  return *this;
}
} // namespace vkw::debug