#include "vkw/Query.hpp"

namespace vkw {
QueryPool::QueryPool(
    vkw::Device &device,
    const VkQueryPoolCreateInfo &createInfo) noexcept(ExceptionsDisabled)
    : UniqueVulkanObject<VkQueryPool>{device, createInfo},
      m_size(createInfo.queryCount) {}
void QueryPool::m_getResultsImpl(
    void *pData, uint32_t firstQuery, uint32_t queryCount, size_t dataSize,
    size_t stride, VkQueryResultFlags flags) noexcept(ExceptionsDisabled) {
  parent().core<1, 0>().vkGetQueryPoolResults(
      parent(), *this, firstQuery, queryCount, dataSize, pData, stride, flags);
}

} // namespace vkw