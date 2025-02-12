#pragma once

#include "bimap_node.h"

template <typename FirstT, typename SecondT, typename Tag>
class bimap_iterator {
public:
  template <typename T>
  using other_tag = std::conditional_t<std::is_same_v<T, left_tag>, right_tag, left_tag>;

public:
  using value_type = FirstT;

  using const_reference = const value_type&;
  using const_pointer = const value_type*;

  using reference = const_reference;
  using pointer = const_pointer;

  using difference_type = std::ptrdiff_t;
  using iterator_category = std::bidirectional_iterator_tag;

  using other_iterator = bimap_iterator<SecondT, FirstT, other_tag<Tag>>;
  using node_func = control_node* (*) (control_node*);

  template <typename, typename, typename, typename>
  friend class bimap;

  template <typename, typename, typename, typename, typename>
  friend class bimap_impl;

  friend class bimap_iterator<SecondT, FirstT, other_tag<Tag>>;

public:
  bimap_iterator() = default;

  bimap_iterator& operator++() {
    auto go_less = [](control_node* ptr) {
      return ptr->get_down_left();
    };
    auto go_great = [](control_node* ptr) {
      return ptr->get_down_right();
    };
    _comn_operator(_ptr, go_less, go_great);
    return *this;
  }

  bimap_iterator& operator--() {
    auto go_less = [](control_node* ptr) {
      return ptr->get_down_right();
    };
    auto go_great = [](control_node* ptr) {
      return ptr->get_down_left();
    };
    _comn_operator(_ptr, go_less, go_great);
    return *this;
  }

  bimap_iterator operator++(int) {
    bimap_iterator res = *this;
    ++*this;
    return res;
  }

  bimap_iterator operator--(int) {
    bimap_iterator res = *this;
    --*this;
    return res;
  }

  pointer operator->() const {
    auto* answer = static_cast<typed_node<FirstT, Tag>*>(static_cast<tagged_node<Tag>*>(_ptr));
    return answer->get_element();
  }

  reference operator*() const {
    return *operator->();
  }

  friend bool operator==(const bimap_iterator& left, const bimap_iterator& right) {
    return (left._ptr == right._ptr);
  }

  friend bool operator!=(const bimap_iterator& left, const bimap_iterator& right) {
    return !(left == right);
  }

  other_iterator flip() const noexcept {
    if (_ptr->is_sentinel()) {
      return other_iterator(control_node::change_control<Tag>(_ptr));
    }
    return other_iterator(control_node::change_control<FirstT, SecondT, Tag>(_ptr));
  }

private:
  control_node* _ptr;

  explicit bimap_iterator(control_node* ptr)
      : _ptr(ptr) {}

  static void _comn_operator(control_node*& ptr, node_func down_left, node_func down_right) {
    if (down_right(ptr) == nullptr) {
      while (ptr->get_parent() && down_right(ptr->get_parent()) == ptr) {
        ptr = ptr->get_parent();
      }
      if (ptr->get_parent()) {
        ptr = ptr->get_parent();
      }
      return;
    }
    ptr = down_right(ptr);
    while (down_left(ptr)) {
      ptr = down_left(ptr);
    }
  }
};
