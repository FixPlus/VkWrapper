#include "vkw/PipelineCache.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

vkw::PipelineCache::PipelineCache(vkw::Device &device, size_t initDataSize,
                                  void *initData,
                                  VkPipelineCacheCreateFlags flags)
    : m_device(device) {
  VkPipelineCacheCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = flags;
  createInfo.initialDataSize = initDataSize;
  createInfo.pInitialData = initData;

  VK_CHECK_RESULT(device.core<1, 0>().vkCreatePipelineCache(device, &createInfo,
                                                            nullptr, &m_cache))
}
vkw::PipelineCache::~PipelineCache() {
  if (!m_cache)
    return;

  m_device.get().core<1, 0>().vkDestroyPipelineCache(m_device.get(), m_cache,
                                                     nullptr);
}

size_t vkw::PipelineCache::dataSize() const {
  size_t ret{};
  VK_CHECK_RESULT(m_device.get().core<1, 0>().vkGetPipelineCacheData(
      m_device.get(), m_cache, &ret, nullptr))

  return ret;
}

bool vkw::PipelineCache::getData(void *buffer, size_t bufferLength) const {

  auto result = m_device.get().core<1, 0>().vkGetPipelineCacheData(
      m_device.get(), m_cache, &bufferLength, buffer);
  if (result == VK_INCOMPLETE) {
    return false;
  } else if (result == VK_SUCCESS) {
    return true;
  }

  VK_CHECK_RESULT(result)

  return false;
}
