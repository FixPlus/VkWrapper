#include "Device.hpp"
#include "Buffer.hpp"
#include "Instance.hpp"
#include "Queue.hpp"
#include "SymbolTable.hpp"
#include "Utils.hpp"
#include <cassert>
#include <iostream>
#include <vector>

namespace vkw {

Device::Device(Device &&another) : m_parent(another.m_parent) {
  m_device = another.m_device;
  m_info = another.m_info;

  ph_device = another.ph_device;

  m_properties = another.m_properties;

  m_features = another.m_features;

  m_enabledFeatures = another.m_enabledFeatures;

  m_memoryProperties = another.m_memoryProperties;

  m_queueFamilyProperties = another.m_queueFamilyProperties;

  m_supportedExtensions = another.m_supportedExtensions;

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

Device::Device(Instance &parent, VkPhysicalDevice phDevice) : m_parent(parent) {

  ph_device = phDevice;

  // Store Properties features, limits and properties of the physical device for
  // later use Device properties also contain limits and sparse properties
  m_parent.core<1, 0>().vkGetPhysicalDeviceProperties(ph_device, &m_properties);
  // Features should be checked by the examples before using them
  m_parent.core<1, 0>().vkGetPhysicalDeviceFeatures(ph_device, &m_features);
  // Memory properties are used regularly for creating all kinds of buffers
  m_parent.core<1, 0>().vkGetPhysicalDeviceMemoryProperties(
      ph_device, &m_memoryProperties);
  // Queue family properties, used for setting up requested queues upon device
  // creation
  uint32_t queueFamilyCount;
  m_parent.core<1, 0>().vkGetPhysicalDeviceQueueFamilyProperties(
      ph_device, &queueFamilyCount, nullptr);

  m_queueFamilyProperties.resize(queueFamilyCount);
  m_parent.core<1, 0>().vkGetPhysicalDeviceQueueFamilyProperties(
      ph_device, &queueFamilyCount, m_queueFamilyProperties.data());

  m_info.name = m_properties.deviceName;

  // Get list of supported extensions
  uint32_t extCount = 0;
  m_parent.core<1, 0>().vkEnumerateDeviceExtensionProperties(
      ph_device, nullptr, &extCount, nullptr);
  if (extCount > 0) {
    std::vector<VkExtensionProperties> extensions(extCount);
    if (m_parent.core<1, 0>().vkEnumerateDeviceExtensionProperties(
            ph_device, nullptr, &extCount, &extensions.front()) == VK_SUCCESS) {
      for (auto ext : extensions) {
        m_supportedExtensions.emplace_back(ext.extensionName);
      }
    }
  }

  // TODO: move this logic to DeviceCreateInfo level

  auto requestedQueueTypes = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_GRAPHICS_BIT;

  std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

  const float defaultQueuePriority(0.0f);

  // Graphics queue
  if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT) {
    queueFamilyIndices.graphics =
        getQueueFamilyIndex(m_queueFamilyProperties, VK_QUEUE_GRAPHICS_BIT);
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
    queueFamilyIndices.compute =
        getQueueFamilyIndex(m_queueFamilyProperties, VK_QUEUE_COMPUTE_BIT);
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
    queueFamilyIndices.transfer =
        getQueueFamilyIndex(m_queueFamilyProperties, VK_QUEUE_TRANSFER_BIT);
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

  std::vector<const char *> deviceExtensions{};

  // TODO: ask for swap chain support on DeviceCreateInfo level
  deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &m_enabledFeatures;

  // TODO: enable only if validation/debug mode enabled
  if (extensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
    deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
  }

  if (!deviceExtensions.empty()) {
    for (const char *enabledExtension : deviceExtensions) {
      if (!extensionSupported(enabledExtension)) {
        throw Error("device has no support for enabled extension " +
                    std::string(enabledExtension));
      }
    }

    deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
    deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
  }

  VK_CHECK_RESULT(m_parent.core<1, 0>().vkCreateDevice(
      ph_device, &deviceCreateInfo, nullptr, &m_device))

  m_coreDeviceSymbols = std::make_unique<DeviceCore<1, 0>>(
      m_parent.core<1, 0>().vkGetDeviceProcAddr, m_device);

  for (auto &extName : deviceExtensions) {
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
  allocatorInfo.physicalDevice = ph_device;
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