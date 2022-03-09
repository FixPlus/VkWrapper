#include "DynamicLoader.hpp"
#include "vkw/Exception.hpp"
#include <sstream>

#ifdef _WIN32
#include <Libloaderapi.h>
#elif defined __linux__
#include <dlfcn.h>
#endif

namespace vkw {

#ifdef _WIN32
void DynamicLoader::m_open() {
  m_libHandle = ::LoadLibraryExA(m_libName.c_str(), nullptr, 0u);

  if (!m_libHandle) {
    std::stringstream ss{};
    ss << m_libName << " was not found. Error code: 0x" << std::hex
       << ::GetLastError();
    throw Error{ss.str(), ErrorCode::DYNAMIC_LIBRARY_NOT_FOUND};
  }
}

void DynamicLoader::m_close() {
  ::FreeLibrary(static_cast<HMODULE>(m_libHandle));
}
void *DynamicLoader::m_getSymbol(std::string const &symbolName) {
  auto *ret =
      ::GetProcAddress(static_cast<HMODULE>(m_libHandle), symbolName.c_str());

  if (!ret) {
    std::stringstream ss{};
    ss << "Cannot find " << symbolName << " in " << m_libName
       << ". Error code: 0x" << std::hex << ::GetLastError();
    throw Error{ss.str(), ErrorCode::DYNAMIC_LIBRARY_SYMBOL_MISSING};
  }

  return (void *)ret;
}
#elif defined __linux__
void DynamicLoader::m_open() {
  m_libHandle = dlopen(m_libName.c_str(), RTLD_LAZY);

  if (!m_libHandle) {
    std::stringstream ss{};
    ss << m_libName << " was not found. Error message: " << dlerror();
    throw Error{ss.str(), ErrorCode::DYNAMIC_LIBRARY_NOT_FOUND};
  }
}

void DynamicLoader::m_close() { dlclose(m_libHandle); }
void *DynamicLoader::m_getSymbol(std::string const &symbolName) {
  auto *ret = dlsym(m_libHandle, symbolName.c_str());

  if (!ret) {
    std::stringstream ss{};
    ss << "Cannot find " << symbolName << " in " << m_libName
       << ". Error message: " << dlerror();
    throw Error{ss.str(), ErrorCode::DYNAMIC_LIBRARY_SYMBOL_MISSING};
  }

  return ret;
}

#endif

} // namespace vkw
