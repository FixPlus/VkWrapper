#include "vkw/Device.hpp"
#include "Utils.hpp"
#include "vkw/Buffer.hpp"
#include "vkw/Instance.hpp"
#include "vkw/Queue.hpp"
#include "vkw/SymbolTable.hpp"
#include <cassert>
#include <iostream>
#include <vector>

namespace vkw {

Device::Device(Device &&another)
    : m_parent(another.m_parent), m_ph_device(std::move(another.m_ph_device)) {
  m_device = another.m_device;

  queueFamilyIndices = another.queueFamilyIndices;

  m_allocator = another.m_allocator;

  m_available_queues = another.m_available_queues;

  m_coreDeviceSymbols = std::move(another.m_coreDeviceSymbols);

  m_extensions = std::move(another.m_extensions);

  another.m_device = VK_NULL_HANDLE;
}

uint32_t getQueueFamilyIndex(
    std::vector<VkQueueFamilyProperties> const &queueFamilyProperties,
    VkQueueFlagBits queueFlags) {
  // Dedicated queue for compute
  // Try to find a queue family index that supports compute but not graphics
  if (queueFlags & VK_QUEUE_COMPUTE_BIT) {
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
      if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
          ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
           0)) {
        return i;
      }
    }
  }

  // Dedicated queue for transfer
  // Try to find a queue family index that supports transfer but not graphics
  // and compute
  if (queueFlags & VK_QUEUE_TRANSFER_BIT) {
    for (uint32_t i = 0;
         i < static_cast<uint32_t>(queueFamilyProperties.size()); i++) {
      if ((queueFamilyProperties[i].queueFlags & queueFlags) &&
          ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ==
           0) &&
          ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)) {
        return i;
      }
    }
  }

  // For other queue types or if no separate compute queue is present, return
  // the first one to support the requested flags
  for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size());
       i++) {
    if (queueFamilyProperties[i].queueFlags & queueFlags) {
      return i;
    }
  }

  throw Error("Could not find a matching queue family index");
}

Device::Device(Instance &parent, PhysicalDevice phDevice)
    : m_parent(parent), m_ph_device(std::move(phDevice)) {

  // TODO: move this logic to DeviceCreateInfo level

  auto requestedQueueTypes = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

  const float defaultQueuePriority(0.0f);

  // Graphics queue
  if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
    queueFamilyIndices.graphics = getQueueFamilyIndex(
        m_ph_device.queueProperties(), VK_QUEUE_GRAPHICS_BIT);
    VkDeviceQueueCreateInfo queueInfo{};
    queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
    queueInfo.queueCount = 1;
    queueInfo.pQueuePriorities = &defaultQueuePriority;
    queueCreateInfos.push_back(queueInfo);
    m_available_queues.emplace_back(queueFamilyIndices.graphics, 1);
  } else {
    queueFamilyIndices.graphics = 0;
  }

  // Dedicated compute queue
  if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT) {
    queueFamilyIndices.compute = getQueueFamilyIndex(
        m_ph_device.queueProperties(), VK_QUEUE_COMPUTE_BIT);
    if (queueFamilyIndices.compute != queueFamilyIndices.graphics) {
      // If compute family index differs, we need an additional queue create
      // info for the compute queue
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
      m_available_queues.emplace_back(queueFamilyIndices.compute, 1);
    }
  } else {
    // Else we use the same queue
    queueFamilyIndices.compute = queueFamilyIndices.graphics;
  }

  // Dedicated transfer queue
  if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT) {
    queueFamilyIndices.transfer = getQueueFamilyIndex(
        m_ph_device.queueProperties(), VK_QUEUE_TRANSFER_BIT);
    if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) &&
        (queueFamilyIndices.transfer != queueFamilyIndices.compute)) {
      // If compute family index differs, we need an additional queue create
      // info for the compute queue
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
      queueInfo.queueCount = 1;
      queueInfo.pQueuePriorities = &defaultQueuePriority;
      queueCreateInfos.push_back(queueInfo);
      m_available_queues.emplace_back(queueFamilyIndices.transfer, 1);
    }
  } else {
    // Else we use the same queue
    queueFamilyIndices.transfer = queueFamilyIndices.graphics;
  }

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &m_ph_device.enabledFeatures();

  std::vector<const char *> extensionsRaw{};
  extensionsRaw.reserve(m_ph_device.enabledExtensions().size());
  std::transform(m_ph_device.enabledExtensions().begin(),
                 m_ph_device.enabledExtensions().end(),
                 std::back_inserter(extensionsRaw),
                 [](std::string const &ext) { return ext.c_str(); });

  deviceCreateInfo.enabledExtensionCount = extensionsRaw.size();
  deviceCreateInfo.ppEnabledExtensionNames = extensionsRaw.data();

  VK_CHECK_RESULT(m_parent.core<1, 0>().vkCreateDevice(
      m_ph_device, &deviceCreateInfo, nullptr, &m_device))

  m_coreDeviceSymbols = std::make_unique<DeviceCore<1, 0>>(
      m_parent.core<1, 0>().vkGetDeviceProcAddr, m_device);

  for (auto &extName : extensionsRaw) {
    std::unique_ptr<DeviceExtensionBase> ext;
    auto initializer = m_deviceExtInitializers.find(extName);
    assert(initializer != m_deviceExtInitializers.end() &&
           "Bad extension name");
    m_extensions.emplace(
        extName,
        std::unique_ptr<DeviceExtensionBase>(initializer->second->initialize(
            m_parent.core<1, 0>().vkGetDeviceProcAddr, m_device)));
  }

  m_apiVer = {1, 0, 0};

  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
  allocatorInfo.physicalDevice = m_ph_device;
  allocatorInfo.device = m_device;
  allocatorInfo.instance = parent;

  VmaVulkanFunctions vmaVulkanFunctions;
  vmaVulkanFunctions.vkCmdCopyBuffer = core<1, 0>().vkCmdCopyBuffer;
  vmaVulkanFunctions.vkAllocateMemory = core<1, 0>().vkAllocateMemory;
  vmaVulkanFunctions.vkBindBufferMemory = core<1, 0>().vkBindBufferMemory;
  vmaVulkanFunctions.vkBindImageMemory = core<1, 0>().vkBindImageMemory;
  vmaVulkanFunctions.vkCreateBuffer = core<1, 0>().vkCreateBuffer;
  vmaVulkanFunctions.vkCreateImage = core<1, 0>().vkCreateImage;
  vmaVulkanFunctions.vkDestroyBuffer = core<1, 0>().vkDestroyBuffer;
  vmaVulkanFunctions.vkDestroyImage = core<1, 0>().vkDestroyImage;
  vmaVulkanFunctions.vkFlushMappedMemoryRanges =
      core<1, 0>().vkFlushMappedMemoryRanges;
  vmaVulkanFunctions.vkFreeMemory = core<1, 0>().vkFreeMemory;
  vmaVulkanFunctions.vkGetBufferMemoryRequirements =
      core<1, 0>().vkGetBufferMemoryRequirements;
  vmaVulkanFunctions.vkGetImageMemoryRequirements =
      core<1, 0>().vkGetImageMemoryRequirements;
  vmaVulkanFunctions.vkGetPhysicalDeviceMemoryProperties =
      m_parent.core<1, 0>().vkGetPhysicalDeviceMemoryProperties;
  vmaVulkanFunctions.vkGetPhysicalDeviceProperties =
      m_parent.core<1, 0>().vkGetPhysicalDeviceProperties;
  vmaVulkanFunctions.vkInvalidateMappedMemoryRanges =
      core<1, 0>().vkInvalidateMappedMemoryRanges;
  vmaVulkanFunctions.vkMapMemory = core<1, 0>().vkMapMemory;
  vmaVulkanFunctions.vkUnmapMemory = core<1, 0>().vkUnmapMemory;

  allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;

  VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &m_allocator))
}

Device::~Device() {

  if (m_device == VK_NULL_HANDLE)
    return;

  vmaDestroyAllocator(m_allocator);

  core<1, 0>().vkDestroyDevice(m_device, nullptr);
}

std::shared_ptr<Queue> Device::getQueue(uint32_t queueFamilyIndex,
                                        uint32_t queueIndex) {
  if (m_queues.contains({queueFamilyIndex, queueIndex})) {
    return m_queues.at({queueFamilyIndex, queueIndex});
  }

  if (!std::any_of(
          m_available_queues.begin(), m_available_queues.end(),
          [queueFamilyIndex, queueIndex](std::pair<uint32_t, uint32_t> e) {
            return e.first == queueFamilyIndex && queueIndex < e.second;
          })) {
    throw Error("Bad queue request");
  }

  return m_queues
      .emplace(std::make_pair(
          std::pair<uint32_t, uint32_t>{queueFamilyIndex, queueIndex},
          std::make_shared<Queue>(Queue(*this, queueFamilyIndex, queueIndex))))
      .first->second;
}

std::unique_ptr<BufferBase>
Device::createBuffer(VmaAllocationCreateInfo const &allocCreateInfo,
                     VkBufferCreateInfo const &createInfo) {
  return std::make_unique<BufferBase>(
      BufferBase(m_allocator, createInfo, allocCreateInfo));
}

void Device::waitIdle() {
  VK_CHECK_RESULT(core<1, 0>().vkDeviceWaitIdle(m_device))
}
} // namespace vkw