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
    : m_parent(another.m_parent), m_ph_device(std::move(another.m_ph_device)),
      m_queues(std::move(another.m_queues)), m_device(VK_NULL_HANDLE),
      m_allocator(another.m_allocator), m_apiVer(another.m_apiVer),
      m_enabledExtensions(std::move(another.m_enabledExtensions)),
      m_coreDeviceSymbols(std::move(another.m_coreDeviceSymbols)) {
  std::swap(m_device, another.m_device);
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

  auto const &queueFamilies = m_ph_device.queueFamilies();
  queueCreateInfos.reserve(queueFamilies.size());

  std::for_each(queueFamilies.begin(), queueFamilies.end(),
                [&queueCreateInfos](auto const &family) {
                  if (!family.hasRequestedQueues())
                    return;

                  VkDeviceQueueCreateInfo queueInfo{};
                  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                  queueInfo.queueFamilyIndex = family.index();
                  queueInfo.queueCount = family.queueRequestedCount();
                  queueInfo.pQueuePriorities = family.queuePrioritiesRaw();
                  queueCreateInfos.push_back(queueInfo);
                });

  VkDeviceCreateInfo deviceCreateInfo = {};
  deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  deviceCreateInfo.queueCreateInfoCount =
      static_cast<uint32_t>(queueCreateInfos.size());
  deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
  deviceCreateInfo.pEnabledFeatures = &m_ph_device.enabledFeatures();

  std::vector<const char *> extensionsRaw{};
  extensionsRaw.reserve(m_ph_device.enabledExtensions().size());
  auto &libRef = parent.parent();
  std::transform(
      m_ph_device.enabledExtensions().begin(),
      m_ph_device.enabledExtensions().end(), std::back_inserter(extensionsRaw),
      [](ext extension) { return Library::ExtensionName(extension); });

  deviceCreateInfo.enabledExtensionCount = extensionsRaw.size();
  deviceCreateInfo.ppEnabledExtensionNames = extensionsRaw.data();

  VK_CHECK_RESULT(m_parent.get().core<1, 0>().vkCreateDevice(
      m_ph_device, &deviceCreateInfo, nullptr, &m_device))

  // TODO: add option to load specific Vulkan version symbols
  m_coreDeviceSymbols = std::make_unique<DeviceCore<1, 0>>(
      m_parent.get().core<1, 0>().vkGetDeviceProcAddr, m_device);

  for (auto &ext : m_ph_device.enabledExtensions()) {
    m_enabledExtensions.emplace(ext);
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
      m_parent.get().core<1, 0>().vkGetPhysicalDeviceMemoryProperties;
  vmaVulkanFunctions.vkGetPhysicalDeviceProperties =
      m_parent.get().core<1, 0>().vkGetPhysicalDeviceProperties;
  vmaVulkanFunctions.vkInvalidateMappedMemoryRanges =
      core<1, 0>().vkInvalidateMappedMemoryRanges;
  vmaVulkanFunctions.vkMapMemory = core<1, 0>().vkMapMemory;
  vmaVulkanFunctions.vkUnmapMemory = core<1, 0>().vkUnmapMemory;

  allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;

  VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &m_allocator))

  std::transform(queueFamilies.begin(), queueFamilies.end(),
                 std::back_inserter(m_queues),
                 [this](QueueFamily const &family) {
                   std::vector<std::unique_ptr<Queue>> queues;
                   auto familyIndex = family.index();
                   auto queueCount = family.queueRequestedCount();

                   for (unsigned i = 0; i < queueCount; ++i)
                     queues.emplace_back(new Queue{*this, familyIndex, i});

                   return queues;
                 });
}

Device::~Device() {

  if (m_device == VK_NULL_HANDLE)
    return;

  vmaDestroyAllocator(m_allocator);

  core<1, 0>().vkDestroyDevice(m_device, nullptr);
}

Queue const &Device::anyGraphicsQueue() const {
  auto graphicsFamilyPred = [](QueueFamily const &family) {
    return family.graphics();
  };
  auto graphicsFamily =
      std::find_if(m_ph_device.queueFamilies().begin(),
                   m_ph_device.queueFamilies().end(), graphicsFamilyPred);
  while (graphicsFamily != m_ph_device.queueFamilies().end()) {
    if (graphicsFamily->hasRequestedQueues()) {
      return *m_queues.at(graphicsFamily->index()).at(0);
    }
    graphicsFamily++;
    graphicsFamily = std::find_if(
        graphicsFamily, m_ph_device.queueFamilies().end(), graphicsFamilyPred);
  }

  throw Error{"Device does not have any queues supporting graphics"};
}

Queue const &Device::anyComputeQueue() const {
  auto computeFamilyPred = [](QueueFamily const &family) {
    return family.compute();
  };
  auto transferFamily =
      std::find_if(m_ph_device.queueFamilies().begin(),
                   m_ph_device.queueFamilies().end(), computeFamilyPred);
  while (transferFamily != m_ph_device.queueFamilies().end()) {
    if (transferFamily->hasRequestedQueues()) {
      return *m_queues.at(transferFamily->index()).at(0);
    }
    transferFamily++;
    transferFamily = std::find_if(
        transferFamily, m_ph_device.queueFamilies().end(), computeFamilyPred);
  }

  throw Error{"Device does not have any queues supporting compute"};
}

Queue const &Device::anyTransferQueue() const {
  auto transferFamilyPred = [](QueueFamily const &family) {
    return family.transfer();
  };
  auto transferFamily =
      std::find_if(m_ph_device.queueFamilies().begin(),
                   m_ph_device.queueFamilies().end(), transferFamilyPred);
  while (transferFamily != m_ph_device.queueFamilies().end()) {
    if (transferFamily->hasRequestedQueues()) {
      return *m_queues.at(transferFamily->index()).at(0);
    }
    transferFamily++;
    transferFamily = std::find_if(
        transferFamily, m_ph_device.queueFamilies().end(), transferFamilyPred);
  }

  throw Error{"Device does not have any queues supporting transfer"};
}

Queue const &Device::getSpecificQueue(QueueFamily::Type type) const {
  auto specificFamilyPred = [type](QueueFamily const &family) {
    return family.strictly(type);
  };
  auto specificFamily =
      std::find_if(m_ph_device.queueFamilies().begin(),
                   m_ph_device.queueFamilies().end(), specificFamilyPred);
  while (specificFamily != m_ph_device.queueFamilies().end()) {
    if (specificFamily->hasRequestedQueues()) {
      return *m_queues.at(specificFamily->index()).at(0);
    }
    specificFamily++;
    specificFamily = std::find_if(
        specificFamily, m_ph_device.queueFamilies().end(), specificFamilyPred);
  }
  std::stringstream ss;
  ss << "Device does not have any specified queues of QueueFamily::Type = 0x"
     << std::hex << type.value;
  throw Error{ss.str()};
}
std::unique_ptr<BufferBase>
Device::createBuffer(VmaAllocationCreateInfo const &allocCreateInfo,
                     VkBufferCreateInfo const &createInfo) {
  return std::make_unique<BufferBase>(
      BufferBase(m_allocator, createInfo, allocCreateInfo));
}

void Device::waitIdle(){
    VK_CHECK_RESULT(core<1, 0>().vkDeviceWaitIdle(m_device))}

Device &Device::operator=(Device &&another) noexcept {

  m_ph_device = std::move(another.m_ph_device);

  m_parent = another.m_parent;

  m_queues = std::move(m_queues);

  m_allocator = another.m_allocator;

  m_coreDeviceSymbols = std::move(another.m_coreDeviceSymbols);

  m_enabledExtensions = std::move(another.m_enabledExtensions);

  m_apiVer = another.m_apiVer;

  std::swap(m_device, another.m_device);

  return *this;
}

} // namespace vkw