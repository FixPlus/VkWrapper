#ifndef VKWRAPPER_PIPELINECACHE_HPP
#define VKWRAPPER_PIPELINECACHE_HPP

#include <vkw/Device.hpp>

namespace vkw {

class PipelineCache : public UniqueVulkanObject<VkPipelineCache> {
public:
  explicit PipelineCache(
      Device const &device, size_t initDataSize = 0, void *initData = nullptr,
      VkPipelineCacheCreateFlags flags = 0) noexcept(ExceptionsDisabled);

  size_t dataSize() const noexcept(ExceptionsDisabled);

  /** @return true if whole cache was copied, false otherwise. */
  bool getData(void *buffer, size_t bufferLength) const
      noexcept(ExceptionsDisabled);
};

} // namespace vkw
#endif // VKWRAPPER_PIPELINECACHE_HPP
