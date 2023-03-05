#include <vkw/Exception.hpp>

namespace vkw {
namespace {

std::vector<std::function<void(Error &)>> errorCallbacks;

inline const char *errorString(VkResult errorCode) noexcept {
  switch (errorCode) {
#define STR(r)                                                                 \
  case VK_##r:                                                                 \
    return #r
    STR(NOT_READY);
    STR(TIMEOUT);
    STR(EVENT_SET);
    STR(EVENT_RESET);
    STR(INCOMPLETE);
    STR(ERROR_OUT_OF_HOST_MEMORY);
    STR(ERROR_OUT_OF_DEVICE_MEMORY);
    STR(ERROR_OUT_OF_POOL_MEMORY);
    STR(ERROR_INITIALIZATION_FAILED);
    STR(ERROR_DEVICE_LOST);
    STR(ERROR_MEMORY_MAP_FAILED);
    STR(ERROR_LAYER_NOT_PRESENT);
    STR(ERROR_EXTENSION_NOT_PRESENT);
    STR(ERROR_FEATURE_NOT_PRESENT);
    STR(ERROR_INCOMPATIBLE_DRIVER);
    STR(ERROR_TOO_MANY_OBJECTS);
    STR(ERROR_FORMAT_NOT_SUPPORTED);
    STR(ERROR_SURFACE_LOST_KHR);
    STR(ERROR_NATIVE_WINDOW_IN_USE_KHR);
    STR(SUBOPTIMAL_KHR);
    STR(ERROR_OUT_OF_DATE_KHR);
    STR(ERROR_INCOMPATIBLE_DISPLAY_KHR);
    STR(ERROR_VALIDATION_FAILED_EXT);
    STR(ERROR_INVALID_SHADER_NV);
#undef STR
  default:
    return "UNKNOWN_ERROR";
  }
}

} // namespace

std::string Error::codeString() const noexcept {
  switch (m_errCode) {
#define STR(r)                                                                 \
  case ErrorCode ::r:                                                          \
    return #r;
    STR(DYNAMIC_LIBRARY_NOT_FOUND)
    STR(DYNAMIC_LIBRARY_SYMBOL_MISSING)
    STR(VULKAN_ERROR)
    STR(API_VERSION_UNSUPPORTED)
    STR(VALIDATION_ERROR)
    STR(EXTENSION_MISSING)
    STR(EXTENSION_UNSUPPORTED)
    STR(LAYER_MISSING)
    STR(LAYER_UNSUPPORTED)
    STR(FEATURE_UNSUPPORTED)
    STR(SURFACE_OUTDATED)
    STR(SPIRV_LINK_ERROR)
    STR(REFERENCE_GUARD_ERROR)
    STR(UNKNOWN)
#undef STR
  }
  return "";
}

PositionalError::PositionalError(std::string const &what,
                                 std::string const &filename, uint32_t line,
                                 ErrorCode errCode) noexcept
    : Error(what + "\n in file " + filename + " on line " +
                std::to_string(line),
            errCode),
      m_filename(filename), m_line(line) {}

VulkanError::VulkanError(VkResult error, std::string const &filename,
                         uint32_t line) noexcept
    : PositionalError(std::string("Vulkan function call returned VkResult: ")
                          .append(errorString(error)),
                      filename, line, ErrorCode::VULKAN_ERROR),
      m_result(error) {}

ExtensionError::ExtensionError(ext id, std::string_view extName, ErrorCode code,
                               std::string_view postfix) noexcept
    : Error(std::string("Extension ")
                .append(extName)
                .append(" is ")
                .append(postfix),
            code),
      m_extName(extName), m_id(id) {}

LayerError::LayerError(layer id, std::string_view layerName, ErrorCode code,
                       std::string_view postfix) noexcept
    : Error(std::string("Layer ").append(layerName).append(" is ").append(
                postfix),
            code),
      m_layerName(layerName), m_id(id) {}

[[noreturn]] void postError(Error &&e) {
#ifdef VKW_ENABLE_EXCEPTIONS
  throw e;
#else
  irrecoverableError(std::move(e));
#endif
}

[[noreturn]] void irrecoverableError(Error &&e) noexcept {
  for (auto &callback : errorCallbacks)
    callback(e);

  std::terminate();
}

void addIrrecoverableErrorCallback(std::function<void(Error &)> callback) {
  errorCallbacks.emplace_back(std::move(callback));
}

} // namespace vkw