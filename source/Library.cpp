#include "Library.hpp"
#include "loader/DynamicLoader.hpp"

namespace vkw {

Library::Library() {
  static constexpr const char *libName =
#ifdef _WIN32
      "vulkan-1.dll";
#endif
  m_loader = std::make_unique<DynamicLoader>(libName);

#define STRINGIFY(X) #X
#define VKW_VK_GLOBAL_FUNCTION_1_0(X)                                          \
  vk##X = reinterpret_cast<PFN_vk##X>(m_loader->getSymbol(STRINGIFY(vk##X)));
#define VKW_VK_GLOBAL_FUNCTION_1_1(X)                                          \
  vk##X = reinterpret_cast<PFN_vk##X>(m_loader->getSymbol(STRINGIFY(vk##X)));
#include "VulkanFunctionList.hpp"
#undef STRINGIFY
}

ApiVersion::ApiVersion(uint32_t encoded)
    : major(VK_API_VERSION_MAJOR(encoded)),
      minor(VK_API_VERSION_MINOR(encoded)),
      revision(VK_API_VERSION_PATCH(encoded)) {}
ApiVersion Library::instanceAPIVersion() const {
  if (this->vkEnumerateInstanceVersion == nullptr)
    return {1, 0, 0};

  uint32_t encoded;
  this->vkEnumerateInstanceVersion(&encoded);
  return ApiVersion{encoded};
}
Library::~Library() {}

} // namespace vkw