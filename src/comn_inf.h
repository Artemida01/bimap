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

template <typename Tag>
using other_tag = std::conditional_t<std::is_same_v<Tag, left_tag>, right_tag, left_tag>;

struct control_node;

template <typename>
struct tagged_node;

template <typename, typename>
struct typed_node;

template <typename, typename>
struct sentinel_node;

template <typename, typename>
struct bimap_node;

template <typename LeftT, typename RightT, typename Tag>
control_node* change_control(control_node* ptr) {
  return static_cast<control_node*>(static_cast<tagged_node<other_tag<Tag>>*>(
      static_cast<bimap_node<LeftT, RightT>*>(static_cast<tagged_node<Tag>*>(ptr))
  ));
}

template <typename Tag>
control_node* change_control(control_node* ptr) {
  return static_cast<control_node*>(static_cast<tagged_node<other_tag<Tag>>*>(
      static_cast<sentinel_node<left_tag, right_tag>*>(static_cast<tagged_node<Tag>*>(ptr))
  ));
}
