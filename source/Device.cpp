#include "vkw/Device.hpp"
#include "Utils.hpp"
#include "vkw/Buffer.hpp"
#include "vkw/Instance.hpp"
#include "vkw/Queue.hpp"
#include "vkw/SymbolTable.hpp"
#include <cassert>
#include <iostream>

#undef max
#include <limits>

namespace vkw {
namespace {
template <unsigned major = 1, unsigned minor = 0>
std::unique_ptr<DeviceCore<1, 0>>
loadDeviceSymbols(vkw::Instance const &instance, VkDevice device,
                  ApiVersion version) noexcept(ExceptionsDisabled) {
  // Here template magic is being used to automatically generate load of
  // every available DeviceCore<major, minor> classes from SymbolTable.inc
  if (version >
      ApiVersion{major, minor, std::numeric_limits<unsigned>::max()}) {
    if constexpr (std::derived_from<DeviceCore<major, minor + 1>,
                                    SymbolTableBase<VkDevice>>)
      return loadDeviceSymbols<major, minor + 1>(instance, device, version);
    else if constexpr (std::derived_from<DeviceCore<major + 1, 0>,
                                         SymbolTableBase<VkDevice>>)
      return loadDeviceSymbols<major + 1, 0>(instance, device, version);
    else
      postError(ApiVersionUnsupported(
          "Could not load device symbols for requested api version",
          ApiVersion{major, minor, 0}, version));
  } else {
    return std::make_unique<DeviceCore<major, minor>>(
        instance.core<1, 0>().vkGetDeviceProcAddr, device);
  }
}

} // namespace

Device::Device(Instance const &instance,
               PhysicalDevice phDevice) noexcept(ExceptionsDisabled)
    : DeviceInfo(instance, std::move(phDevice)),
      UniqueVulkanObject<VkDevice>(instance, physicalDevice(), info()),
      m_allocator(m_allocatorCreateImpl()) {

  auto const &queueFamilies = physicalDevice().queueFamilies();
  m_coreDeviceSymbols = loadDeviceSymbols(
      parent(), handle(), physicalDevice().requestedApiVersion());

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

DeviceInfo::DeviceInfo(Instance const &parent,
                       PhysicalDevice phDevice) noexcept(ExceptionsDisabled)
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

  // Add memory VK_EXT_memory_budget if possible.
  if (parent.isExtensionEnabled(ext::KHR_get_physical_device_properties2) &&
      m_ph_device.extensionSupported(ext::EXT_memory_budget) &&
      std::ranges::none_of(m_ph_device.enabledExtensions(),
                           [](auto &extension) {
                             return extension == ext::EXT_memory_budget;
                           })) {
    m_ph_device.enableExtension(ext::EXT_memory_budget);
  }

  m_enabledExtensionsRaw.reserve(m_ph_device.enabledExtensions().size());
  std::transform(m_ph_device.enabledExtensions().begin(),
                 m_ph_device.enabledExtensions().end(),
                 std::back_inserter(m_enabledExtensionsRaw), [](ext extension) {
                   return Library::ExtensionName(extension);
                 });

  m_createInfo.enabledExtensionCount = m_enabledExtensionsRaw.size();
  m_createInfo.ppEnabledExtensionNames = m_enabledExtensionsRaw.data();
#ifdef VK_VERSION_1_2
  if (phDevice.requestedApiVersion() >= ApiVersion(1, 1, 0))
    m_createInfo.pNext = &m_ph_device.enabledVulkan11Features();
  else
    m_createInfo.pNext = nullptr;
#endif
  for (auto &ext : m_ph_device.enabledExtensions()) {
    m_enabledExtensions.emplace(ext);
  }

  m_apiVer = m_ph_device.requestedApiVersion();
}

Queue const &Device::anyGraphicsQueue() const noexcept(ExceptionsDisabled) {
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

  postError(Error{"Device does not have any queues supporting graphics"});
}

Queue const &Device::anyComputeQueue() const noexcept(ExceptionsDisabled) {
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

  postError(Error{"Device does not have any queues supporting compute"});
}

Queue const &Device::anyTransferQueue() const noexcept(ExceptionsDisabled) {
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

  postError(Error{"Device does not have any queues supporting transfer"});
}

Queue const &Device::getSpecificQueue(QueueFamily::Type type) const
    noexcept(ExceptionsDisabled) {
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
  postError(Error{ss.str()});
}

void Device::waitIdle() noexcept(ExceptionsDisabled){
    VK_CHECK_RESULT(core<1, 0>().vkDeviceWaitIdle(handle()))}

VmaAllocator Device::m_allocatorCreateImpl() noexcept(ExceptionsDisabled) {
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.vulkanApiVersion = apiVersion();
  allocatorInfo.physicalDevice = physicalDevice();
  allocatorInfo.device = handle();
  allocatorInfo.instance = parent();
  if (std::ranges::any_of(physicalDevice().enabledExtensions(), [](auto &id) {
        return id == ext::EXT_memory_budget;
      }))
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;

  VmaVulkanFunctions vmaVulkanFunctions{};
  vmaVulkanFunctions.vkGetInstanceProcAddr =
      parent().parent().vkGetInstanceProcAddr;
  vmaVulkanFunctions.vkGetDeviceProcAddr =
      parent().core<1, 0>().vkGetDeviceProcAddr;

  allocatorInfo.pVulkanFunctions = &vmaVulkanFunctions;
  allocatorInfo.pAllocationCallbacks = hostAllocator().allocator();
  VmaAllocator allocator;

  VK_CHECK_RESULT(vmaCreateAllocator(&allocatorInfo, &allocator))

  return allocator;
}

void Device::AllocatorDeleter::operator()(VmaAllocator a) {
  vmaDestroyAllocator(a);
}

Device::~Device() {}

void Device::frame() {
  m_currentFrame++;
  vmaSetCurrentFrameIndex(m_allocator.get(), m_currentFrame);
}

std::vector<std::pair<VkDeviceSize, VkDeviceSize>>
Device::getDeviceMemoryUsage() {
  auto heapCount = physicalDevice().memoryProperties().memoryHeapCount;
  std::vector<VmaBudget> heapsInfo;
  heapsInfo.resize(heapCount);
  vmaGetHeapBudgets(m_allocator.get(), heapsInfo.data());
  std::vector<std::pair<VkDeviceSize, VkDeviceSize>> ret;
  std::transform(heapsInfo.begin(), heapsInfo.end(), std::back_inserter(ret),
                 [](auto &heapInfo) {
                   return std::make_pair(heapInfo.usage, heapInfo.budget);
                 });
  return ret;
}
} // namespace vkw