#ifndef VKRENDERER_VERTEXBUFFER_HPP
#define VKRENDERER_VERTEXBUFFER_HPP

#include "Buffer.hpp"

namespace vkw {

enum class VertexAttributeType {
  VEC4F,
  VEC3F,
  VEC2F,
  FLOAT,
  RGBA8_UNORM,
};

constexpr uint64_t size_of(VertexAttributeType attrType) {

  switch (attrType) {
  case VertexAttributeType::VEC4F:
    return 4 * sizeof(float);
  case VertexAttributeType::VEC3F:
    return 3 * sizeof(float);
  case VertexAttributeType::VEC2F:
    return 2 * sizeof(float);
  case VertexAttributeType::FLOAT:
    return 1 * sizeof(float);
  case VertexAttributeType::RGBA8_UNORM:
    return 1 * sizeof(uint32_t);
  }
  return 0;
}

constexpr VkFormat format_of(VertexAttributeType attrType) {
  switch (attrType) {
  case VertexAttributeType::VEC4F:
    return VK_FORMAT_R32G32B32A32_SFLOAT;
  case VertexAttributeType::VEC3F:
    return VK_FORMAT_R32G32B32_SFLOAT;
  case VertexAttributeType::VEC2F:
    return VK_FORMAT_R32G32_SFLOAT;
  case VertexAttributeType::FLOAT:
    return VK_FORMAT_R32_SFLOAT;
  case VertexAttributeType::RGBA8_UNORM:
    return VK_FORMAT_R8G8B8A8_UNORM;
  }

  return VK_FORMAT_MAX_ENUM;
}

constexpr uint32_t locations_hold(VertexAttributeType attrType) {
  switch (attrType) {
  case VertexAttributeType::VEC4F:
  case VertexAttributeType::VEC3F:
  case VertexAttributeType::VEC2F:
  case VertexAttributeType::FLOAT:
  case VertexAttributeType::RGBA8_UNORM:
    return 1u;
  }
  return 0xFFFFFFFF;
}

/** @Concept: Any struct used as attribute holder for Vertex Buffer should be
 * AttributeArrayLike */
template <typename T>
concept AttributeArrayLike = requires(T array) {
  // Static constexpr member required to query attribute count
  { T::count() } -> std::same_as<uint32_t>;
  // Static constexpr member required to query every attribute type
  { (T::getAttrType(0)) } -> std::same_as<VertexAttributeType>;
};

template <AttributeArrayLike T> struct is_attribute_array_valid_size {
  /**
   * @template: is_attribute_array_valid_size<T>
   *
   * Checks if actual structure size (sizeof(T)) matches sum of every attribute
   * size
   *
   */
private:
  constexpr static uint64_t sumSize(uint64_t elem) {
    auto size = size_of(T::getAttrType(elem));
    if (elem == 0)
      return size;

    return sumSize(elem - 1) + size;
  }

  constexpr static bool validate() {
    return sizeof(T) == sumSize(T::count() - 1);
  }

public:
  constexpr static const bool value = validate();
};

/** @Concept: Any struct used by Vertex Buffer should satisfy AttributeArray
 * restrictions */
template <typename T>
concept AttributeArray = is_attribute_array_valid_size<T>::value;

template <vkw::VertexAttributeType... attrs> class AttributeBase {
public:
  /**
   * Base class for any vertex attribute placeholders
   *
   * e.g.:
   *    struct MyAttributes: public AttributeBase<VEC4F, VEC3F>{
   *       glm::vec4 pos;
   *       float color[3];
   *    };
   *
   *    VertexBuffer<MyAttributes> v_buf{...}; //OK
   *
   */

  constexpr static uint32_t count() { return sizeof...(attrs); }
  constexpr static vkw::VertexAttributeType getAttrType(uint32_t elem) {
    return m_attrs[elem];
  }

private:
  constexpr static const vkw::VertexAttributeType m_attrs[sizeof...(attrs)] = {
      attrs...};
};

template <AttributeArray T> class VertexBuffer : public Buffer<T> {
public:
  VertexBuffer(Device &device, uint64_t count,
               VmaAllocationCreateInfo const &createInfo,
               VkBufferUsageFlags usage = 0)
      : Buffer<T>(device, count, usage | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                  createInfo) {}

private:
};

/** @template: type map VkIndexType<->(unsigned int sized type) */
template <VkIndexType type> struct vkr_index_type {};

template <> struct vkr_index_type<VK_INDEX_TYPE_UINT32> {
  typedef uint32_t Type;
};

template <> struct vkr_index_type<VK_INDEX_TYPE_UINT16> {
  typedef uint16_t Type;
};

template <> struct vkr_index_type<VK_INDEX_TYPE_UINT8_EXT> {
  typedef uint8_t Type;
};

template <VkIndexType type>
class IndexBuffer : public Buffer<typename vkr_index_type<type>::Type> {
public:
  IndexBuffer(Device &device, uint64_t count,
              VmaAllocationCreateInfo const &createInfo,
              VkBufferUsageFlags usage = 0)
      : Buffer<typename vkr_index_type<type>::Type>(
            device, count, usage | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            createInfo) {}
};
} // namespace vkw
#endif // VKRENDERER_VERTEXBUFFER_HPP
