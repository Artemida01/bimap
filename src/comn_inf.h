#pragma once

#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

struct left_tag;
struct right_tag;

template <typename>
struct tagged_node;

template <typename, typename>
struct typed_node;

template <typename, typename>
struct sentinel_node;

template <typename, typename>
struct bimap_node;
