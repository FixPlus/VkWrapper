#include "vkw/SPIRVModule.hpp"
#include "spirv-tools/linker.hpp"
#include "vkw/Exception.hpp"
#include <sstream>
#include <unordered_map>
namespace vkw {

SPIRVModule::SPIRVModule(std::span<const unsigned int> code) {
  m_code.reserve(code.size());
  std::copy(code.begin(), code.end(), std::back_inserter(m_code));
  // TODO: maybe some code verification checks here
}

namespace {

class MessageCollector {
public:
  void postMessage(spv_message_level_t messageLevel, std::string_view message) {
    m_messages.emplace_back(messageLevel, message);
  }

  auto &messages() const { return m_messages; }

  void flushMessages() { m_messages.clear(); }

private:
  std::vector<std::pair<spv_message_level_t, std::string>> m_messages;
};

class SPIRVContext : public MessageCollector, public spvtools::Context {
public:
  SPIRVContext()
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
                         bool linkLibrary) {
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

    throw vkw::Error{ss.str(), ErrorCode::SPIRV_LINK_ERROR};
  }
}

} // namespace vkw