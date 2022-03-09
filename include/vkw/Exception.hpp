#ifndef VKRENDERER_EXCEPTION_HPP
#define VKRENDERER_EXCEPTION_HPP

#include <exception>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace vkw {

class Exception : public std::runtime_error {
public:
  Exception(std::string const &what) : std::runtime_error(what) {}

private:
};

enum class ErrorCode {

  DYNAMIC_LIBRARY_NOT_FOUND,
  DYNAMIC_LIBRARY_SYMBOL_MISSING,

  VULKAN_ERROR,
  EXTENSION_MISSING,
  SURFACE_OUTDATED,
  UNKNOWN,
};

class Error : public Exception {
public:
  Error(std::string const &what, ErrorCode errorCode = ErrorCode::UNKNOWN)
      : Exception(what), m_errCode(errorCode) {}

  ErrorCode code() const { return m_errCode; }

private:
  ErrorCode m_errCode;
};

class SurfaceOutdated : public Error {
public:
  SurfaceOutdated() : Error("Surface outdated", ErrorCode::SURFACE_OUTDATED) {}
};

inline std::string errorString(VkResult errorCode) {
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

class ExtensionMissing : public Error {
public:
  explicit ExtensionMissing(std::string const &extName)
      : Error("Extension " + extName + " is missing",
              ErrorCode::EXTENSION_MISSING),
        m_extName(extName) {}

  std::string const &extName() const { return m_extName; }

private:
  std::string m_extName;
};

class PositionalError : public Error {
public:
  PositionalError(std::string const &what, std::string const &filename,
                  uint32_t line, ErrorCode errCode)
      : Error(what + "\n in file " + filename + " on line " +
                  std::to_string(line),
              errCode),
        m_filename(filename), m_line(line) {}

  std::string const &filename() const { return m_filename; };
  uint32_t line() const { return m_line; };

private:
  std::string m_filename;
  uint32_t m_line;
};

class VulkanError : public PositionalError {
public:
  explicit VulkanError(VkResult error, std::string const &filename,
                       uint32_t line)
      : PositionalError("Vulkan function call returned VkResult: " +
                            errorString(error),
                        filename, line, ErrorCode::VULKAN_ERROR),
        m_result(error) {}

  VkResult result() const { return m_result; }

private:
  VkResult m_result;
};

} // namespace vkw
#endif // VKRENDERER_EXCEPTION_HPP
