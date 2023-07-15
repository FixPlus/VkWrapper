#include "vkw/SPIRVModule.hpp"
#include "spirv-tools/linker.hpp"
#include "spirv_reflect.h"
#include "vkw/Exception.hpp"
#include <sstream>
#include <unordered_map>
namespace vkw {

namespace {
std::string_view reflectErrorStr(SpvReflectResult result) noexcept {
  switch (result) {
#define CASE(X)                                                                \
  case X:                                                                      \
    return #X;
    CASE(SPV_REFLECT_RESULT_SUCCESS)
    CASE(SPV_REFLECT_RESULT_NOT_READY)
    CASE(SPV_REFLECT_RESULT_ERROR_PARSE_FAILED)
    CASE(SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED)
    CASE(SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED)
    CASE(SPV_REFLECT_RESULT_ERROR_NULL_POINTER)
    CASE(SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR)
    CASE(SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH)
    CASE(SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT)
    CASE(SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE)
#undef CASE
  default:
    return "**UNKNOWN**";
  }
}

} // namespace

class SPIRVReflectError : public Error {
public:
  SPIRVReflectError(SpvReflectResult result, std::string_view libCallName)
      : Error(
            [&]() {
              std::stringstream ss;
              ss << "SPIRV-Reflect - call to " << libCallName << " returned "
                 << reflectErrorStr(result) << " code";
              return ss.str();
            }(),
            ErrorCode::SPIRV_REFLECT_ERROR),
        m_result(result), m_libCallName(libCallName) {}

  auto result() const noexcept { return m_result; }

  auto libCallName() const noexcept { return m_libCallName; }

private:
  SpvReflectResult m_result;
  std::string_view m_libCallName;
};

#define REFLECT_CALL(X, ...)                                                   \
  m_info->X(__VA_ARGS__);                                                      \
  if (m_info->GetResult() != SPV_REFLECT_RESULT_SUCCESS)                       \
    postError(SPIRVReflectError{result, #X});
SPIRVModule::SPIRVModule(std::span<const unsigned int> code) noexcept(
    ExceptionsDisabled) {
  m_code.reserve(code.size());
  std::copy(code.begin(), code.end(), std::back_inserter(m_code));
  // TODO: maybe some code verification checks here
}

namespace {

class MessageCollector {
public:
  void postMessage(spv_message_level_t messageLevel,
                   std::string_view message) noexcept(ExceptionsDisabled) {
    m_messages.emplace_back(messageLevel, message);
  }

  auto &messages() const noexcept { return m_messages; }

  void flushMessages() noexcept { m_messages.clear(); }

private:
  std::vector<std::pair<spv_message_level_t, std::string>> m_messages;
};

class SPIRVContext : public MessageCollector, public spvtools::Context {
public:
  SPIRVContext() noexcept
      : spvtools::Context(SPV_ENV_VULKAN_1_0),
        m_consumer([this](spv_message_level_t messageLevel, const char *input,
                          const spv_position_t position, const char *message) {
          postMessage(messageLevel, message);
        }) {
    SetMessageConsumer(m_consumer);
  }

private:
  spvtools::MessageConsumer m_consumer;
};
} // namespace

void SPIRVModule::m_link(std::vector<unsigned int> &output,
                         std::span<const std::span<const unsigned int>> input,
                         bool linkLibrary) noexcept(ExceptionsDisabled) {
  static SPIRVContext context;
  boost::container::small_vector<size_t, 3> codeSizes;
  boost::container::small_vector<const unsigned int *, 3> codes;

  std::transform(
      input.begin(), input.end(), std::back_inserter(codeSizes),
      [](const std::span<const unsigned int> code) { return code.size(); });
  std::transform(
      input.begin(), input.end(), std::back_inserter(codes),
      [](const std::span<const unsigned int> code) { return code.data(); });

  spvtools::LinkerOptions options;

  if (linkLibrary) {
    options.SetCreateLibrary(true);
    options.SetAllowPartialLinkage(true);
  }

  context.flushMessages();

  auto result = spvtools::Link(context, codes.data(), codeSizes.data(),
                               codes.size(), &output, options);

  if (result != SPV_SUCCESS) {
    std::stringstream ss;
    ss << "SPIRV-Link failed. spirv-link log: \n";

    for (auto &message : context.messages()) {
      ss << message.second << std::endl;
    }

    postError(vkw::Error{ss.str(), ErrorCode::SPIRV_LINK_ERROR});
  }
}
const SPIRVModuleInfo &SPIRVModule::info() const noexcept(ExceptionsDisabled) {
  if (m_info)
    return *m_info;
  m_info = std::make_shared<SPIRVModuleInfo>(*this);
  return *m_info;
}

std::shared_ptr<SPIRVModuleInfo> SPIRVModule::takeInfo() const
    noexcept(ExceptionsDisabled) {
  if (m_info)
    return m_info;
  m_info = std::make_shared<SPIRVModuleInfo>(*this);
  return m_info;
}

SPIRVModule::~SPIRVModule() = default;

SPIRVModuleInfo::SPIRVModuleInfo(SPIRVModule const &module) {
  auto code = module.code();
  m_info = std::make_unique<spv_reflect::ShaderModule>(
      sizeof(uint32_t) * code.size(), code.data(),
      SPV_REFLECT_MODULE_FLAG_NO_COPY);
  if (m_info->GetResult() != SPV_REFLECT_RESULT_SUCCESS)
    postError(
        SPIRVReflectError{m_info->GetResult(), "ShaderModule::ShaderModule()"});
}

const SpvReflectShaderModule *SPIRVModuleInfo::rawModule() const noexcept {
  return &m_info->GetShaderModule();
}

std::string_view SPIRVEntryPointInfo::name() const noexcept {
  return m_handle->name;
}
unsigned SPIRVEntryPointInfo::id() const noexcept { return m_handle->id; }

template <>
SPIRVEntryPointInfo
ReflectInfoIteratorTraitsCommon<ReflectEntryPointTraits>::get_value(
    const SpvReflectShaderModule *s, unsigned idx) noexcept {
  return SPIRVEntryPointInfo{s->entry_points + idx};
}

template <>
unsigned ReflectInfoIteratorTraitsCommon<ReflectEntryPointTraits>::get_size(
    const SpvReflectShaderModule *s) noexcept {
  return s->entry_point_count;
}

template <>
SPIRVInterfaceVariable
ReflectInfoIteratorTraitsCommon<ReflectInterfaceVariableTraits>::get_value(
    const SpvReflectEntryPoint *s, unsigned idx) noexcept {
  return SPIRVInterfaceVariable{s->interface_variables + idx};
}

template <>
unsigned
ReflectInfoIteratorTraitsCommon<ReflectInterfaceVariableTraits>::get_size(
    const SpvReflectEntryPoint *s) noexcept {
  return s->interface_variable_count;
}

template <>
SPIRVInterfaceVariable
ReflectInfoIteratorTraitsCommon<ReflectInputVariableTraits>::get_value(
    const SpvReflectEntryPoint *s, unsigned idx) noexcept {
  return SPIRVInterfaceVariable{s->input_variables[idx]};
}

template <>
unsigned ReflectInfoIteratorTraitsCommon<ReflectInputVariableTraits>::get_size(
    const SpvReflectEntryPoint *s) noexcept {
  return s->input_variable_count;
}

template <>
SPIRVInterfaceVariable
ReflectInfoIteratorTraitsCommon<ReflectOutputVariableTraits>::get_value(
    const SpvReflectEntryPoint *s, unsigned idx) noexcept {
  return SPIRVInterfaceVariable{s->output_variables[idx]};
}

template <>
unsigned ReflectInfoIteratorTraitsCommon<ReflectOutputVariableTraits>::get_size(
    const SpvReflectEntryPoint *s) noexcept {
  return s->output_variable_count;
}

template <>
SPIRVDescriptorBindingInfo
ReflectInfoIteratorTraitsCommon<ReflectDescriptorBindingTraits>::get_value(
    const SpvReflectDescriptorSet *s, unsigned idx) noexcept {
  return SPIRVDescriptorBindingInfo{s->bindings[idx]};
}

template <>
unsigned
ReflectInfoIteratorTraitsCommon<ReflectDescriptorBindingTraits>::get_size(
    const SpvReflectDescriptorSet *s) noexcept {
  return s->binding_count;
}

template <>
SPIRVDescriptorSetInfo
ReflectInfoIteratorTraitsCommon<ReflectDescriptorSetTraits>::get_value(
    const SpvReflectEntryPoint *s, unsigned idx) noexcept {
  return SPIRVDescriptorSetInfo{s->descriptor_sets + idx};
}

template <>
unsigned ReflectInfoIteratorTraitsCommon<ReflectDescriptorSetTraits>::get_size(
    const SpvReflectEntryPoint *s) noexcept {
  return s->descriptor_set_count;
}

template <>
SPIRVDescriptorSetInfo
ReflectInfoIteratorTraitsCommon<ReflectDescriptorSetModuleTraits>::get_value(
    const SpvReflectShaderModule *s, unsigned idx) noexcept {
  return SPIRVDescriptorSetInfo{s->descriptor_sets + idx};
}

template <>
unsigned
ReflectInfoIteratorTraitsCommon<ReflectDescriptorSetModuleTraits>::get_size(
    const SpvReflectShaderModule *s) noexcept {
  return s->descriptor_set_count;
}

namespace {

VkFormat mapFormat(SpvReflectFormat format) noexcept {
  // VkFormat <-> SpvReflectFormat are mapped one to one.
  return VkFormat(format);
}

VkShaderStageFlagBits mapStage(SpvReflectShaderStageFlagBits stage) noexcept {
  // VkShaderStageFlagBits <-> SpvReflectShaderStageFlagBits are mapped one to
  // one.
  return VkShaderStageFlagBits(stage);
}

VkDescriptorType mapDescriptorType(SpvReflectDescriptorType type) noexcept {
  // VkDescriptorType <-> SpvReflectDescriptorType are mapped one to one.
  return VkDescriptorType(type);
}
} // namespace

VkShaderStageFlagBits SPIRVEntryPointInfo::stage() const noexcept {
  return mapStage(m_handle->shader_stage);
}

unsigned SPIRVInterfaceVariable::location() const noexcept {
  return m_var->location;
}

VkFormat SPIRVInterfaceVariable::format() const noexcept {
  return mapFormat(m_var->format);
}

std::string_view SPIRVInterfaceVariable::name() const noexcept {
  return m_var->name;
}

bool SPIRVInterfaceVariable::isInput() const noexcept {
  return m_var->storage_class == SpvStorageClassInput;
}

bool SPIRVInterfaceVariable::isOutput() const noexcept {
  return m_var->storage_class == SpvStorageClassOutput;
}

VkDescriptorType SPIRVDescriptorBindingInfo::descriptorType() const noexcept {
  return mapDescriptorType(m_binding->descriptor_type);
}

unsigned SPIRVDescriptorBindingInfo::descriptorCount() const noexcept {
  return m_binding->count;
}

std::string_view SPIRVDescriptorBindingInfo::name() const noexcept {
  return m_binding->name;
}

unsigned SPIRVDescriptorBindingInfo::index() const noexcept {
  return m_binding->binding;
}
unsigned SPIRVDescriptorSetInfo::index() const noexcept { return m_set->set; }

SPIRVModuleInfo::~SPIRVModuleInfo() = default;
} // namespace vkw