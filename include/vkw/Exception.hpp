#ifndef VKRENDERER_EXCEPTION_HPP
#define VKRENDERER_EXCEPTION_HPP

#include <exception>
#include <functional>
#include <stdexcept>
#include <string>
#include <vulkan/vulkan.h>

namespace vkw {

class Exception : public std::runtime_error {
public:
  explicit Exception(std::string_view what) noexcept
      : std::runtime_error(std::string(what)) {}

private:
};

enum class ErrorCode {

  DYNAMIC_LIBRARY_NOT_FOUND,
  DYNAMIC_LIBRARY_SYMBOL_MISSING,

  VULKAN_ERROR,
  API_VERSION_UNSUPPORTED,
  VALIDATION_ERROR,
  EXTENSION_MISSING,
  EXTENSION_UNSUPPORTED,
  LAYER_MISSING,
  LAYER_UNSUPPORTED,
  FEATURE_UNSUPPORTED,
  SURFACE_OUTDATED,
  SPIRV_LINK_ERROR,
  SPIRV_REFLECT_ERROR,
  BAD_SHADER_MODULE,
  REFERENCE_GUARD_ERROR,
  UNKNOWN,
};

class Error : public Exception {
public:
  explicit Error(std::string_view what,
                 ErrorCode errorCode = ErrorCode::UNKNOWN) noexcept
      : Exception(what), m_errCode(errorCode) {}

  ErrorCode code() const noexcept { return m_errCode; }

  std::string codeString() const noexcept;

private:
  ErrorCode m_errCode;
};

class SurfaceOutdated : public Error {
public:
  SurfaceOutdated() noexcept
      : Error("Surface outdated", ErrorCode::SURFACE_OUTDATED) {}
};

enum class ext;
class ExtensionError : public Error {
public:
  const char *extName() const noexcept { return m_extName.c_str(); }

  ext id() const noexcept { return m_id; }

protected:
  ExtensionError(ext id, std::string_view extName, ErrorCode code,
                 std::string_view postfix) noexcept;

private:
  std::string m_extName;
  ext m_id;
};

enum class layer;
class LayerError : public Error {
public:
  const char *layerName() const noexcept { return m_layerName.c_str(); }

  layer id() const noexcept { return m_id; }

protected:
  LayerError(layer id, std::string_view layerName, ErrorCode code,
             std::string_view postfix) noexcept;

private:
  std::string m_layerName;
  layer m_id;
};

class ExtensionMissing : public ExtensionError {
public:
  ExtensionMissing(ext id, std::string_view extName) noexcept
      : ExtensionError(id, extName, ErrorCode::EXTENSION_MISSING, "missing") {}
};

class ExtensionUnsupported : public ExtensionError {
public:
  ExtensionUnsupported(ext id, std::string_view extName) noexcept
      : ExtensionError(id, extName, ErrorCode::EXTENSION_UNSUPPORTED,
                       "unsupported") {}
};

class LayerMissing : public LayerError {
public:
  LayerMissing(layer id, std::string_view layerName) noexcept
      : LayerError(id, layerName, ErrorCode::LAYER_MISSING, "missing") {}
};

class LayerUnsupported : public LayerError {
public:
  LayerUnsupported(layer id, std::string_view layerName) noexcept
      : LayerError(id, layerName, ErrorCode::LAYER_UNSUPPORTED, "unsupported") {
  }
};

class PositionalError : public Error {
public:
  PositionalError(std::string const &what, std::string const &filename,
                  uint32_t line, ErrorCode errCode) noexcept;

  std::string const &filename() const noexcept { return m_filename; };
  uint32_t line() const noexcept { return m_line; };

private:
  std::string m_filename;
  uint32_t m_line;
};

class VulkanError : public PositionalError {
public:
  VulkanError(VkResult error, std::string const &filename,
              uint32_t line) noexcept;

  VkResult result() const noexcept { return m_result; }

private:
  VkResult m_result;
};

class ValidationError : public Error {
public:
  explicit ValidationError(std::string_view what) noexcept
      : Error(what, ErrorCode::VALIDATION_ERROR){};
};

class ReferenceGuardError : public Error {
public:
  explicit ReferenceGuardError(unsigned strongReferenceLeft) noexcept
      : Error(std::string("Number of strong references left: ")
                  .append(std::to_string(strongReferenceLeft)),
              ErrorCode::REFERENCE_GUARD_ERROR){};
};

#ifdef VKW_ENABLE_EXCEPTIONS
static constexpr bool ExceptionsDisabled = false;
#else
static constexpr bool ExceptionsDisabled = true;
#endif
// postError may do two things in accordance to how project was configured
// If project was configured with VKW_ENABLE_EXCEPTIONS=On then it
// will throw instance of e. Otherwise, it will call irrecoverableError(e)
[[noreturn]] void postError(Error &&e);

// In some cases exception cannot be thrown(e.g. in destructor or move
// constructors). irrecoverableError() is called then which will eventually call
// std::terminate. Application might want to do something at that point(e.g.
// display diagnostics) For that it can add its own callbacks via
// addIrrecoverableErrorCallback()

[[noreturn]] void irrecoverableError(Error &&e) noexcept;
void addIrrecoverableErrorCallback(std::function<void(Error &)> callback);

} // namespace vkw
#endif // VKRENDERER_EXCEPTION_HPP
