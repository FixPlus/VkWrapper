#include <vkw/Exception.hpp>

namespace vkw {
namespace {

std::vector<std::function<void(Error &)>> errorCallbacks;

}

[[noreturn]] void postError(Error &&e) {
#ifdef VKW_ENABLE_EXCEPTIONS
  throw e;
#else
  irrecoverableError(std::move(e));
#endif
}

[[noreturn]] void irrecoverableError(Error &&e) noexcept {
  for (auto &callback : errorCallbacks)
    callback(e);

  std::terminate();
}

void addIrrecoverableErrorCallback(std::function<void(Error &)> callback) {
  errorCallbacks.emplace_back(std::move(callback));
}

} // namespace vkw