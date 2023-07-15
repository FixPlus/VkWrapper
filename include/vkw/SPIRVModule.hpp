#ifndef VKWRAPPER_SPIRVMODULE_HPP
#define VKWRAPPER_SPIRVMODULE_HPP

#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <memory>
#include <ranges>
#include <span>
#include <vector>
#include <vkw/Exception.hpp>
#include <vkw/ReferenceGuard.hpp>

struct SpvReflectShaderModule;
struct SpvReflectEntryPoint;
struct SpvReflectInterfaceVariable;
struct SpvReflectDescriptorSet;
struct SpvReflectDescriptorBinding;

namespace spv_reflect {

class ShaderModule;

}
namespace vkw {

class SPIRVModuleInfo;

class SPIRVModule {
public:
  explicit SPIRVModule(std::span<const unsigned> code) noexcept(
      ExceptionsDisabled);

  template <std::ranges::range Modules>
  requires std::same_as<
      typename std::remove_cv<std::ranges::range_value_t<Modules>>::type,
      SPIRVModule>
  explicit SPIRVModule(Modules const &modules,
                       bool linkLibrary = false) noexcept(ExceptionsDisabled) {
#ifndef __linux__
    boost::container::small_vector<std::span<const unsigned>, 3> input;
#else
    std::vector<std::span<const unsigned>> input;
#endif
    std::transform(modules.begin(), modules.end(), std::back_inserter(input),
                   [](SPIRVModule const &module) { return module.code(); });
    m_link(m_code, std::span<const std::span<const unsigned>>{input},
           linkLibrary);
  }

  std::span<const unsigned> code() const noexcept { return m_code; }

  /// SPIRVModuleInfo is constructed lazily upon first request.
  const SPIRVModuleInfo &info() const noexcept(ExceptionsDisabled);

  /// Provides shared ownership of shader info (used by Shader class).
  std::shared_ptr<SPIRVModuleInfo> takeInfo() const
      noexcept(ExceptionsDisabled);

  virtual ~SPIRVModule();

private:
  static void m_link(std::vector<unsigned> &output,
                     std::span<const std::span<const unsigned>> input,
                     bool linkLibrary) noexcept(ExceptionsDisabled);
  std::vector<unsigned> m_code;
  // Make it shared to allow copy.
  // Mutable is for lazy construction.
  mutable std::shared_ptr<SPIRVModuleInfo> m_info;
};

template <typename T>
concept ReflectInfoIteratorTraits = requires {
  typename T::value_type;
  typename T::super_type;
};

template <ReflectInfoIteratorTraits T> struct ReflectInfoIteratorTraitsCommon {
  static typename T::value_type get_value(typename T::super_type *s,
                                          unsigned idx) noexcept;
  static unsigned get_size(typename T::super_type *s) noexcept;
};

template <ReflectInfoIteratorTraits T> class ReflectInfoIterator {
public:
  using super_type = typename T::super_type;
  using value_type = typename T::value_type;
  using reference = void;
  using pointer = void;
  using difference_type = long;
  using iterator_category = std::input_iterator_tag;

  explicit ReflectInfoIterator(super_type *s = nullptr,
                               unsigned index = 0) noexcept
      : m_super(s), m_index(index) {}

  auto operator*() const noexcept {
    return ReflectInfoIteratorTraitsCommon<T>::get_value(m_super, m_index);
  }

  auto &operator++() noexcept {
    m_index++;
    return *this;
  }

  const auto operator++(int) noexcept {
    auto ret = *this;
    m_index++;
    return ret;
  }

  bool operator==(ReflectInfoIterator const &another) const noexcept {
    return m_super == another.m_super && m_index == another.m_index;
  }

  bool operator!=(ReflectInfoIterator const &another) const noexcept {
    return !(*this == another);
  }

private:
  super_type *m_super;
  unsigned m_index;
};

template <ReflectInfoIteratorTraits T>
class ReflectInfoRange
    : public std::ranges::view_interface<ReflectInfoRange<T>> {
public:
  using super_type = typename T::super_type;

  explicit ReflectInfoRange(super_type *s) noexcept
      : m_super(s), m_size(ReflectInfoIteratorTraitsCommon<T>::get_size(s)){};

  auto begin() const noexcept { return ReflectInfoIterator<T>{m_super, 0}; }

  auto end() const noexcept { return ReflectInfoIterator<T>{m_super, m_size}; }

  auto size() const noexcept { return m_size; }

  auto empty() const noexcept { return m_size == 0; }

private:
  super_type *m_super;
  unsigned m_size;
};

class SPIRVInterfaceVariable;

struct ReflectInputVariableTraits {
  using value_type = SPIRVInterfaceVariable;
  using super_type = const SpvReflectEntryPoint;
};

struct ReflectOutputVariableTraits {
  using value_type = SPIRVInterfaceVariable;
  using super_type = const SpvReflectEntryPoint;
};

struct ReflectInterfaceVariableTraits {
  using value_type = SPIRVInterfaceVariable;
  using super_type = const SpvReflectEntryPoint;
};

class SPIRVInterfaceVariable final {
public:
  /// This class represents signle shader attribute.
  /// E.G.:
  /// (GLSL)
  ///
  /// ...
  /// layout(location = 3) in vec4 Pos;
  /// ...
  ///
  /// attr.location() == 3;
  /// attr.format() == VK_FORMAT_R32G32B32_SFLOAT;
  /// attr.name() == "Pos";
  /// attr.isInput() == true;
  /// attr.isOutput() == false;

  unsigned location() const noexcept;
  VkFormat format() const noexcept;
  std::string_view name() const noexcept;
  bool isInput() const noexcept;
  bool isOutput() const noexcept;

private:
  friend class ReflectInfoIteratorTraitsCommon<ReflectInputVariableTraits>;
  friend class ReflectInfoIteratorTraitsCommon<ReflectOutputVariableTraits>;
  friend class ReflectInfoIteratorTraitsCommon<ReflectInterfaceVariableTraits>;

  explicit SPIRVInterfaceVariable(
      const SpvReflectInterfaceVariable *var) noexcept
      : m_var{var} {}
  const SpvReflectInterfaceVariable *m_var;
};

class SPIRVDescriptorBindingInfo;

struct ReflectDescriptorBindingTraits {
  using value_type = SPIRVDescriptorBindingInfo;
  using super_type = const SpvReflectDescriptorSet;
};

class SPIRVDescriptorBindingInfo {
public:
  /// Descriptor type that should be passed to
  /// VkDescriptorSetLayoutBinding.
  VkDescriptorType descriptorType() const noexcept;

  /// Descriptor count that should be passed to
  /// VkDescriptorSetLayoutBinding.
  unsigned descriptorCount() const noexcept;

  /// Name of binding given in source code.
  std::string_view name() const noexcept;

  /// Index of binding from layout.
  unsigned index() const noexcept;

private:
  friend class ReflectInfoIteratorTraitsCommon<ReflectDescriptorBindingTraits>;
  explicit SPIRVDescriptorBindingInfo(
      const SpvReflectDescriptorBinding *binding) noexcept
      : m_binding{binding} {}
  const SpvReflectDescriptorBinding *m_binding;
};

class SPIRVDescriptorSetInfo;

struct ReflectDescriptorSetTraits {
  using value_type = SPIRVDescriptorSetInfo;
  using super_type = const SpvReflectEntryPoint;
};

struct ReflectDescriptorSetModuleTraits {
  using value_type = SPIRVDescriptorSetInfo;
  using super_type = const SpvReflectShaderModule;
};

class SPIRVDescriptorSetInfo {
public:
  using Bindings = ReflectInfoRange<ReflectDescriptorBindingTraits>;

  /// Set index from layout.
  unsigned index() const noexcept;

  /// Range of bindings in set.
  auto bindings() const noexcept { return Bindings{m_set}; }

private:
  friend class ReflectInfoIteratorTraitsCommon<ReflectDescriptorSetTraits>;
  friend class ReflectInfoIteratorTraitsCommon<
      ReflectDescriptorSetModuleTraits>;
  explicit SPIRVDescriptorSetInfo(const SpvReflectDescriptorSet *set) noexcept
      : m_set{set} {}
  const SpvReflectDescriptorSet *m_set;
};

class SPIRVEntryPointInfo;

struct ReflectEntryPointTraits {
  using value_type = SPIRVEntryPointInfo;
  using super_type = const SpvReflectShaderModule;
};

class SPIRVEntryPointInfo final {
public:
  using InputVariables = ReflectInfoRange<ReflectInputVariableTraits>;
  using OutputVariables = ReflectInfoRange<ReflectOutputVariableTraits>;
  using InterfaceVariables = ReflectInfoRange<ReflectInterfaceVariableTraits>;
  using DescriptorSets = ReflectInfoRange<ReflectDescriptorSetTraits>;

  std::string_view name() const noexcept;
  unsigned id() const noexcept;

  VkShaderStageFlagBits stage() const noexcept;

  /// Input shader attributes used by this kernel. In GLSL are marked as 'in'.
  /// @note they are NOT sorted by their location.
  auto inputVariables() const noexcept { return InputVariables(m_handle); }

  /// Output shader attributes used by this kernel. In GLSL are marked as 'out'.
  /// @note they are NOT sorted by their location.
  auto outputVariables() const noexcept { return OutputVariables(m_handle); }

  /// Combined shader attributes used by this kernel (both input and output).
  /// @note they are NOT sorted by their location.
  auto interfaceVariables() const noexcept {
    return InterfaceVariables(m_handle);
  }

  /// Descriptors sets used by this entry point.
  auto sets() const noexcept { return DescriptorSets(m_handle); }

private:
  friend class ReflectInfoIteratorTraitsCommon<ReflectEntryPointTraits>;
  explicit SPIRVEntryPointInfo(const SpvReflectEntryPoint *handle) noexcept
      : m_handle(handle) {}
  const SpvReflectEntryPoint *m_handle;
};

class SPIRVModuleInfo final {
public:
  explicit SPIRVModuleInfo(SPIRVModule const &module) noexcept(
      ExceptionsDisabled);

  using EntryPoints = ReflectInfoRange<ReflectEntryPointTraits>;
  using DescriptorSets = ReflectInfoRange<ReflectDescriptorSetModuleTraits>;

  /// List of all entry points defined by that module.
  auto entryPoints() const noexcept { return EntryPoints(rawModule()); }

  /// Descriptors sets used in the first entry point. If there is no
  /// entry points it shows all descriptors for this module.
  auto sets() const noexcept { return DescriptorSets(rawModule()); }

  virtual ~SPIRVModuleInfo();

private:
  const SpvReflectShaderModule *rawModule() const noexcept;
  std::unique_ptr<spv_reflect::ShaderModule> m_info;
};

} // namespace vkw
#endif // VKWRAPPER_SPIRVMODULE_HPP
