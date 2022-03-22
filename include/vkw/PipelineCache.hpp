#ifndef VKWRAPPER_PIPELINECACHE_HPP
#define VKWRAPPER_PIPELINECACHE_HPP

#include "vkw/Common.hpp"

namespace vkw {

class PipelineCache {
public:
  explicit PipelineCache(Device &device, size_t initDataSize = 0,
                         void *initData = nullptr,
                         VkPipelineCacheCreateFlags flags = 0);

  PipelineCache(PipelineCache const &another) = delete;
  PipelineCache(PipelineCache &&another) noexcept
      : m_device(another.m_device), m_cache(another.m_cache) {
    another.m_cache = VK_NULL_HANDLE;
  }

  PipelineCache &operator=(PipelineCache const &another) = delete;
  PipelineCache &operator=(PipelineCache &&another) noexcept {
    m_device = another.m_device;
    std::swap(m_cache, another.m_cache);
    return *this;
  }

  size_t dataSize() const;

  /** @return true if whole cache was copied, false otherwise. */
  bool getData(void *buffer, size_t bufferLength) const;

  virtual ~PipelineCache();

  operator VkPipelineCache() const { return m_cache; }

private:
  DeviceCRef m_device;
  VkPipelineCache m_cache;
};

} // namespace vkw
#endif // VKWRAPPER_PIPELINECACHE_HPP
