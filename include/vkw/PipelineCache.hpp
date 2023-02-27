#ifndef VKWRAPPER_PIPELINECACHE_HPP
#define VKWRAPPER_PIPELINECACHE_HPP

#include "vkw/Device.hpp"
#include <vkw/UniqueVulkanObject.hpp>

namespace vkw {

class PipelineCache : public UniqueVulkanObject<VkPipelineCache> {
public:
  explicit PipelineCache(Device const &device, size_t initDataSize = 0,
                         void *initData = nullptr,
                         VkPipelineCacheCreateFlags flags = 0);

  size_t dataSize() const;

  /** @return true if whole cache was copied, false otherwise. */
  bool getData(void *buffer, size_t bufferLength) const;
};

} // namespace vkw
#endif // VKWRAPPER_PIPELINECACHE_HPP
