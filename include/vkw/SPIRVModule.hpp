#ifndef VKWRAPPER_SPIRVMODULE_HPP
#define VKWRAPPER_SPIRVMODULE_HPP

#include <boost/container/small_vector.hpp>
#include <ranges>
#include <span>
#include <vector>
#include <algorithm>

namespace vkw {

class SPIRVModule {
public:
  explicit SPIRVModule(std::span<const unsigned> code);

  template <std::ranges::range Modules>
  requires std::same_as<
      typename std::remove_cv<std::ranges::range_value_t<Modules>>::type,
      SPIRVModule>
  explicit SPIRVModule(Modules const &modules, bool linkLibrary = false) {
#ifndef __linux__
    boost::container::small_vector<std::span<const unsigned>, 3> input;
#else
    std::vector<std::span<const unsigned>> input;
#endif
    std::transform(modules.begin(), modules.end(), std::back_inserter(input),
                   [](SPIRVModule const &module) { return module.code(); });
    m_link(m_code, std::span<const std::span<const unsigned>>{input}, linkLibrary);
  }

  std::span<const unsigned> code() const { return m_code; }

  virtual ~SPIRVModule() = default;

private:
  static void m_link(std::vector<unsigned> &output,
                     std::span<const std::span<const unsigned>> input,
                     bool linkLibrary);
  std::vector<unsigned> m_code;
};

} // namespace vkw
#endif // VKWRAPPER_SPIRVMODULE_HPP
