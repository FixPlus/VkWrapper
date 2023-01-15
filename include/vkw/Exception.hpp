#ifndef VKRENDERER_EXCEPTION_HPP
#define VKRENDERER_EXCEPTION_HPP

#include <exception>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace vkw {

class Exception : public std::runtime_error {
public:
  explicit Exception(std::string_view what)
      : std::runtime_error(std::string(what)) {}

private:
};

enum class ErrorCode {

  DYNAMIC_LIBRARY_NOT_FOUND,
  DYNAMIC_LIBRARY_SYMBOL_MISSING,

  VULKAN_ERROR,
  VALIDATION_ERROR,
  EXTENSION_MISSING,
  EXTENSION_UNSUPPORTED,
  LAYER_MISSING,
  LAYER_UNSUPPORTED,
  FEATURE_UNSUPPORTED,
  SURFACE_OUTDATED,
  SPIRV_LINK_ERROR,
  UNKNOWN,
};

class Error : public Exception {
public:
  explicit Error(std::string_view what,
                 ErrorCode errorCode = ErrorCode::UNKNOWN)
      : Exception(what), m_errCode(errorCode) {}

  ErrorCode code() const { return m_errCode; }

private:
  ErrorCode m_errCode;
};

class SurfaceOutdated : public Error {
public:
  SurfaceOutdated() : Error("Surface outdated", ErrorCode::SURFACE_OUTDATED) {}
};

inline const char *errorString(VkResult errorCode) {
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
enum class ext;
class ExtensionError : public Error {
public:
  const char *extName() const { return m_extName.c_str(); }

  ext id() const { return m_id; }

protected:
  explicit ExtensionError(ext id, std::string_view extName, ErrorCode code,
                          std::string_view postfix)
      : Error(std::string("Extension ")
                  .append(extName)
                  .append(" is ")
                  .append(postfix),
              code),
        m_extName(extName), m_id(id) {}

private:
  std::string m_extName;
  ext m_id;
};

enum class layer;
class LayerError : public Error {
public:
  const char *layerName() const { return m_layerName.c_str(); }

  layer id() const { return m_id; }

protected:
  explicit LayerError(layer id, std::string_view layerName, ErrorCode code,
                      std::string_view postfix)
      : Error(std::string("Layer ").append(layerName).append(" is ").append(
                  postfix),
              code),
        m_layerName(layerName), m_id(id) {}

private:
  std::string m_layerName;
  layer m_id;
};

class ExtensionMissing : public ExtensionError {
public:
  ExtensionMissing(ext id, std::string_view extName)
      : ExtensionError(id, extName, ErrorCode::EXTENSION_MISSING, "missing") {}
};

class ExtensionUnsupported : public ExtensionError {
public:
  ExtensionUnsupported(ext id, std::string_view extName)
      : ExtensionError(id, extName, ErrorCode::EXTENSION_UNSUPPORTED,
                       "unsupported") {}
};

class LayerMissing : public LayerError {
public:
  LayerMissing(layer id, std::string_view layerName)
      : LayerError(id, layerName, ErrorCode::LAYER_MISSING, "missing") {}
};

class LayerUnsupported : public LayerError {
public:
  LayerUnsupported(layer id, std::string_view layerName)
      : LayerError(id, layerName, ErrorCode::LAYER_UNSUPPORTED, "unsupported") {
  }
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
      : PositionalError(std::string("Vulkan function call returned VkResult: ")
                            .append(errorString(error)),
                        filename, line, ErrorCode::VULKAN_ERROR),
        m_result(error) {}

  VkResult result() const { return m_result; }

private:
  VkResult m_result;
};

class ValidationError : public Error {
public:
  explicit ValidationError(std::string_view what)
      : Error(what, ErrorCode::VALIDATION_ERROR){};
};

} // namespace vkw
#endif // VKRENDERER_EXCEPTION_HPP
