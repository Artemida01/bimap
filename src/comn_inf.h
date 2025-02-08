#pragma once

#include <assert.h>

#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

struct left_tag;
struct right_tag;

template <typename Tag>
using other_tag = std::conditional_t<std::is_same_v<Tag, left_tag>, right_tag, left_tag>;

template <typename>
struct tagged_node;

template <typename, typename>
struct typed_node;

template <typename, typename>
struct sentinel_node;

template <typename, typename>
struct bimap_node;
