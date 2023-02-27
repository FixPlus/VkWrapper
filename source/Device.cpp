#include "vkw/Device.hpp"
#include "Utils.hpp"
#include "vkw/Buffer.hpp"
#include "vkw/Instance.hpp"
#include "vkw/Queue.hpp"
#include "vkw/SymbolTable.hpp"
#include <cassert>
#include <iostream>

namespace vkw {

Device::Device(Instance const &instance, PhysicalDevice phDevice)
    : DeviceInfo(std::move(phDevice)), UniqueVulkanObject<VkDevice>(
                                           instance, physicalDevice(), info()),
      m_allocator(m_allocatorCreateImpl()) {

  auto const &queueFamilies = physicalDevice().queueFamilies();
  // TODO: add option to load specific Vulkan version symbols
  m_coreDeviceSymbols = std::make_unique<DeviceCore<1, 0>>(
      parent().core<1, 0>().vkGetDeviceProcAddr, handle());

  std::transform(queueFamilies.begin(), queueFamilies.end(),
                 std::back_inserter(m_queues),
                 [this](QueueFamily const &family) {
                   InFamilyQueueContainerT queues;
                   auto familyIndex = family.index();
                   auto queueCount = family.queueRequestedCount();

                   for (unsigned i = 0; i < queueCount; ++i)
                     queues.emplace_back(new Queue{*this, familyIndex, i});

                   return queues;
                 });
}

DeviceInfo::DeviceInfo(PhysicalDevice phDevice)
    : m_ph_device(std::move(phDevice)) {

  auto const &queueFamilies = m_ph_device.queueFamilies();
  m_queueCreateInfo.reserve(queueFamilies.size());

  std::for_each(queueFamilies.begin(), queueFamilies.end(),
                [this](auto const &family) {
                  if (!family.hasRequestedQueues())
                    return;

                  VkDeviceQueueCreateInfo queueInfo{};
                  queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                  queueInfo.queueFamilyIndex = family.index();
                  queueInfo.queueCount = family.queueRequestedCount();
                  queueInfo.pQueuePriorities = family.queuePrioritiesRaw();
                  m_queueCreateInfo.push_back(queueInfo);
                });

  m_createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  m_createInfo.queueCreateInfoCount =
      static_cast<uint32_t>(m_queueCreateInfo.size());
  m_createInfo.pQueueCreateInfos = m_queueCreateInfo.data();
  m_createInfo.pEnabledFeatures = &m_ph_device.enabledFeatures();

  m_enabledExtensionsRaw.reserve(m_ph_device.enabledExtensions().size());
  std::transform(m_ph_device.enabledExtensions().begin(),
                 m_ph_device.enabledExtensions().end(),
                 std::back_inserter(m_enabledExtensionsRaw), [](ext extension) {
                   return Library::ExtensionName(extension);
                 });

  m_createInfo.enabledExtensionCount = m_enabledExtensionsRaw.size();
  m_createInfo.ppEnabledExtensionNames = m_enabledExtensionsRaw.data();

  for (auto &ext : m_ph_device.enabledExtensions()) {
    m_enabledExtensions.emplace(ext);
  }

  m_apiVer = {1, 0, 0};
}

Queue const &Device::anyGraphicsQueue() const {
  auto graphicsFamilyPred = [](QueueFamily const &family) {
    return family.graphics();
  };
  auto graphicsFamily =
      std::find_if(physicalDevice().queueFamilies().begin(),
                   physicalDevice().queueFamilies().end(), graphicsFamilyPred);
  while (graphicsFamily != physicalDevice().queueFamilies().end()) {
    if (graphicsFamily->hasRequestedQueues()) {
      return *m_queues.at(graphicsFamily->index()).at(0);
    }
    graphicsFamily++;
    graphicsFamily =
        std::find_if(graphicsFamily, physicalDevice().queueFamilies().end(),
                     graphicsFamilyPred);
  }

  throw Error{"Device does not have any queues supporting graphics"};
}

Queue const &Device::anyComputeQueue() const {
  auto computeFamilyPred = [](QueueFamily const &family) {
    return family.compute();
  };
  auto transferFamily =
      std::find_if(physicalDevice().queueFamilies().begin(),
                   physicalDevice().queueFamilies().end(), computeFamilyPred);
  while (transferFamily != physicalDevice().queueFamilies().end()) {
    if (transferFamily->hasRequestedQueues()) {
      return *m_queues.at(transferFamily->index()).at(0);
    }
    transferFamily++;
    transferFamily =
        std::find_if(transferFamily, physicalDevice().queueFamilies().end(),
                     computeFamilyPred);
  }

  throw Error{"Device does not have any queues supporting compute"};
}

Queue const &Device::anyTransferQueue() const {
  auto transferFamilyPred = [](QueueFamily const &family) {
    return family.transfer();
  };
  auto transferFamily =
      std::find_if(physicalDevice().queueFamilies().begin(),
                   physicalDevice().queueFamilies().end(), transferFamilyPred);
  while (transferFamily != physicalDevice().queueFamilies().end()) {
    if (transferFamily->hasRequestedQueues()) {
      return *m_queues.at(transferFamily->index()).at(0);
    }
    transferFamily++;
    transferFamily =
        std::find_if(transferFamily, physicalDevice().queueFamilies().end(),
                     transferFamilyPred);
  }

  throw Error{"Device does not have any queues supporting transfer"};
}

Queue const &Device::getSpecificQueue(QueueFamily::Type type) const {
  auto specificFamilyPred = [type](QueueFamily const &family) {
    return family.strictly(type);
  };
  auto specificFamily =
      std::find_if(physicalDevice().queueFamilies().begin(),
                   physicalDevice().queueFamilies().end(), specificFamilyPred);
  while (specificFamily != physicalDevice().queueFamilies().end()) {
    if (specificFamily->hasRequestedQueues()) {
      return *m_queues.at(specificFamily->index()).at(0);
    }
    specificFamily++;
    specificFamily =
        std::find_if(specificFamily, physicalDevice().queueFamilies().end(),
                     specificFamilyPred);
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
      BufferBase(m_allocator.get(), createInfo, allocCreateInfo));
}

void Device::waitIdle(){
    VK_CHECK_RESULT(core<1, 0>().vkDeviceWaitIdle(handle()))}

VmaAllocator Device::m_allocatorCreateImpl() {
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
  allocatorInfo.physicalDevice = physicalDevice();
  allocatorInfo.device = handle();
  allocatorInfo.instance = parent();

  VmaVulkanFunctions vmaVulkanFunctions{};
  vmaVulkanFunctions.vkGetInstanceProcAddr =
      parent().parent().vkGetInstanceProcAddr;
  vmaVulkanFunctions.vkGetDeviceProcAddr =
      parent().core<1, 0>().vkGetDeviceProcAddr;

  allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
  VmaAllocator allocator;

  VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &allocator))

  return allocator;
}

void Device::AllocatorDeleter::operator()(VmaAllocator a) {
  vmaDestroyAllocator(a);
}
} // namespace vkw