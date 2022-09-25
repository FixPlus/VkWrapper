#ifndef VKWRAPPER_LAYERS_HPP
#define VKWRAPPER_LAYERS_HPP

#include "Exception.hpp"
namespace vkw {

enum class layer;

namespace internal {
bool isLayerEnabled(Instance const &instance, layer layer);
const char *layer_name(layer id);
} // namespace internal

template <layer id> class Layer {
public:
  explicit Layer(Instance const &instance) {
    if (!internal::isLayerEnabled(instance, id))
      throw LayerMissing(id, internal::layer_name(id));
  }
};

enum class layer {
#define VKW_LAYER_MAP_ENTRY(X) X,
#include "LayerMap.inc"
#undef VKW_LAYER_MAP_ENTRY
};

} // namespace vkw
#endif // VKWRAPPER_LAYERS_HPP
