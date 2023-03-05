#ifndef VKWRAPPER_LIBRARY_HPP
#define VKWRAPPER_LIBRARY_HPP

#include <vkw/HostAllocator.hpp>
#include <vkw/SymbolTable.hpp>

#include <memory>
#include <sstream>
#include <vector>

namespace vkw {

#include <vkw/LibraryVersion.inc>

struct ApiVersion {
  uint32_t major;
  uint32_t minor;
  uint32_t revision;

  ApiVersion(uint32_t maj, uint32_t min, uint32_t rev)
      : major(maj), minor(min), revision(rev) {}
  ApiVersion(uint32_t encoded);
  ApiVersion() = default;

  operator std::string() const {
    std::stringstream ss;
    ss << std::to_string(major) << "." << std::to_string(minor) << "."
       << std::to_string(revision);
    return ss.str();
  }

  operator uint32_t() const {
    return VK_MAKE_API_VERSION(0, major, minor, revision);
  }
  auto operator<=>(ApiVersion const &another) const = default;
};

class ApiVersionUnsupported : public Error {
public:
  enum class CompatibilityFactor { EARLIER_VERSION, EXACT_VERSION };
  ApiVersionUnsupported(std::string_view details, ApiVersion supported,
                        ApiVersion unsupported,
                        CompatibilityFactor compatibility =
                            CompatibilityFactor::EARLIER_VERSION) noexcept;

  auto &supported() const noexcept { return m_lastSupported; }
  auto &unsupported() const noexcept { return m_unsupported; }
  auto compatibility() const noexcept { return m_compatibility; }

private:
  ApiVersion m_unsupported;
  ApiVersion m_lastSupported;
  CompatibilityFactor m_compatibility;
};

enum class ext;
enum class layer;

class VulkanLibraryLoader {
public:
  virtual PFN_vkGetInstanceProcAddr getInstanceProcAddr() = 0;

  virtual ~VulkanLibraryLoader() = default;
};

class Library final : public ReferenceGuard {
public:
  /**
   *    @param loader
   *    Application may create their own vulkan loader
   *    using interface VulkanLibraryLoader.
   *    Pass nullptr to use embedded loader.
   *
   *    @param allocator
   *    Application may want to pass their own instance of
   *    HostAllocator if it wants to override VkAllocationCallback structure.
   *    Pass nullptr to disable VkAllocationCallback override
   *
   *    @param vkwVersion
   *    This parameter should only be passed with default value here.
   *    Do not overwrite it. It used to check if loaded dll version
   *    matches header version
   * */
  Library(VulkanLibraryLoader *loader = nullptr,
          HostAllocator *allocator = nullptr,
          ApiVersion vkwVersion = ApiVersion{MajorVersion, MinorVersion,
                                             0}) noexcept(ExceptionsDisabled);

  ~Library();

  bool hasLayer(layer layerName) const noexcept(ExceptionsDisabled);

  VkLayerProperties layerProperties(layer layerName) const
      noexcept(ExceptionsDisabled);

  bool hasInstanceExtension(ext extensionId) const noexcept(ExceptionsDisabled);

  VkExtensionProperties instanceExtensionProperties(ext extensionId) const
      noexcept(ExceptionsDisabled);

  PFN_vkCreateInstance vkCreateInstance;
  PFN_vkEnumerateInstanceExtensionProperties
      vkEnumerateInstanceExtensionProperties;
  PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties;
  PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
  PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;

  ApiVersion instanceAPIVersion() const noexcept;

  static const char *ExtensionName(ext id) noexcept(ExceptionsDisabled);

  static ext
  ExtensionId(std::string_view extensionName) noexcept(ExceptionsDisabled);

  static bool ValidExtensionName(std::string_view extensionName) noexcept;

  static const char *LayerName(layer id) noexcept(ExceptionsDisabled);

  static layer LayerId(std::string_view layerName) noexcept(ExceptionsDisabled);

  static bool ValidLayerName(std::string_view layerName) noexcept;

  static ApiVersion version() noexcept;

  auto &hostAllocator() const noexcept { return m_allocator.get(); }

private:
  std::unique_ptr<VulkanLibraryLoader> m_embedded_loader;
  std::unique_ptr<HostAllocator> m_default_allocator;
  StrongReference<HostAllocator> m_allocator;
  std::vector<VkLayerProperties> m_layer_properties;
  std::vector<VkExtensionProperties> m_instance_extension_properties;
};
} // namespace vkw
#endif // VKWRAPPER_LIBRARY_HPP
