#include "vkw/PipelineCache.hpp"
#include "Utils.hpp"
#include "vkw/Device.hpp"

namespace vkw {

namespace {

VkPipelineCacheCreateInfo fillCreateInfo(size_t initDataSize, void *initData,
                                         VkPipelineCacheCreateFlags flags) {
  VkPipelineCacheCreateInfo createInfo{};
  createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  createInfo.pNext = nullptr;
  createInfo.flags = flags;
  createInfo.initialDataSize = initDataSize;
  createInfo.pInitialData = initData;
  return createInfo;
}
} // namespace

PipelineCache::PipelineCache(
    vkw::Device const &device, size_t initDataSize, void *initData,
    VkPipelineCacheCreateFlags flags) noexcept(ExceptionsDisabled)
    : UniqueVulkanObject<VkPipelineCache>(
          device, fillCreateInfo(initDataSize, initData, flags)) {}

size_t PipelineCache::dataSize() const noexcept(ExceptionsDisabled) {
  size_t ret{};
  VK_CHECK_RESULT(parent().core<1, 0>().vkGetPipelineCacheData(
      parent(), handle(), &ret, nullptr))

  return ret;
}

bool PipelineCache::getData(void *buffer, size_t bufferLength) const
    noexcept(ExceptionsDisabled) {

  auto result = parent().core<1, 0>().vkGetPipelineCacheData(
      parent(), handle(), &bufferLength, buffer);
  if (result == VK_INCOMPLETE) {
    return false;
  } else if (result == VK_SUCCESS) {
    return true;
  }

  VK_CHECK_RESULT(result)

  return false;
}

} // namespace vkw