#ifndef VKWRAPPER_DYNAMICLOADER_HPP
#define VKWRAPPER_DYNAMICLOADER_HPP

#include <string>

namespace vkw {

class DynamicLoader {
public:
  DynamicLoader(DynamicLoader const &another) = delete;
  DynamicLoader(DynamicLoader &&another) noexcept
      : m_libHandle(another.m_libHandle),
        m_libName(std::move(another.m_libName)) {
    another.m_libHandle = nullptr;
  };
  DynamicLoader const& operator=(DynamicLoader const& another) = delete;
  DynamicLoader& operator=(DynamicLoader&& another) noexcept{
      std::swap(m_libHandle, another.m_libHandle);
      std::swap(m_libName, another.m_libName);
      return *this;
  };

  DynamicLoader(std::string libName) : m_libName(std::move(libName)) {
    m_open();
  }

  void *getSymbol(std::string const &symbolName) {
    return m_getSymbol(symbolName);
  }

  ~DynamicLoader() {
    if (m_libHandle)
      m_close();
  }

private:
  std::string m_libName;

  // platform specific

  void *m_libHandle{nullptr};
  void m_open();
  void m_close();
  void *m_getSymbol(std::string const &symbolName);
};

} // namespace vkw
#endif // VKWRAPPER_DYNAMICLOADER_HPP
