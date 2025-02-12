#pragma once
#include "comn_inf.h"

struct control_node {
public:
  template <typename Tag>
  using other_tag = std::conditional_t<std::is_same_v<Tag, left_tag>, right_tag, left_tag>;

public:
  control_node() = default;
  control_node(const control_node&) = default;
  ~control_node() = default;

  control_node(control_node&& other) noexcept {
    *this = std::move(other);
  }

  control_node& operator=(control_node&& other) noexcept {
    if (!other.is_empty()) {
      copy_edges(other);
      other._parent = other._left = other._right = &other;
      _left->_parent = _parent->_left = this;
    } else {
      _parent = _left = _right = this;
    }
    return *this;
  }

  void copy_edges(const control_node& other) noexcept {
    _parent = other._parent;
    _left = other._left;
    _right = other._right;
  }

  [[nodiscard]] control_node* get_down_right() const {
    return _right;
  }

  [[nodiscard]] control_node* get_down_left() const {
    return _left;
  }

  [[nodiscard]] control_node* get_parent() const {
    return _parent;
  }

  void connect_less(control_node* node) {
    node->_parent = this;
    _left = node;
  }

  void connect_great(control_node* node) {
    node->_parent = this;
    _right = node;
  }

  bool is_empty() noexcept {
    return (_parent == this && _left == this && _right == this);
  }

  [[nodiscard]] bool is_left_child() const {
    assert(this->get_parent() != nullptr);
    return (this->get_parent()->get_down_left() == this);
  }

  [[nodiscard]] bool is_right_child() const {
    assert(this->get_parent() != nullptr);
    return (this->get_parent()->get_down_right() == this);
  }

  [[nodiscard]] bool is_sentinel() {
    return (get_down_right() == this);
  }

  [[nodiscard]] bool is_last() const {
    return (_left == nullptr) && (_right == nullptr);
  }

  void set_parent(control_node* node) {
    _parent = node;
  }

  void set_left(control_node* node) {
    _left = node;
  }

  void set_right(control_node* node) {
    _right = node;
  }

public:
  static void null_node(control_node* node) {
    node->_right = nullptr;
    node->_parent = nullptr;
    node->_left = nullptr;
  }

  template <typename LeftT, typename RightT, typename Tag>
  static control_node* change_control(control_node* ptr) {
    return static_cast<control_node*>(static_cast<tagged_node<other_tag<Tag>>*>(
        static_cast<bimap_node<LeftT, RightT>*>(static_cast<tagged_node<Tag>*>(ptr))
    ));
  }

  template <typename Tag>
  static control_node* change_control(control_node* ptr) {
    return static_cast<control_node*>(static_cast<tagged_node<other_tag<Tag>>*>(
        static_cast<sentinel_node<left_tag, right_tag>*>(static_cast<tagged_node<Tag>*>(ptr))
    ));
  }

private:
  control_node* _parent{this};
  control_node* _left{this};
  control_node* _right{this};
};

template <typename Tag>
struct tagged_node : public control_node {};

template <typename Type, typename Tag>
struct typed_node : public tagged_node<Tag> {
public:
  using value_type = Type;
  using node_tag = Tag;
  using node_base = control_node;

public:
  typed_node() = default;
  typed_node(const typed_node&) = default;
  typed_node& operator=(const typed_node&) = default;
  typed_node& operator=(typed_node&&) = default;

  template <typename U>
  explicit typed_node(U&& element)
      : _element(std::forward<U>(element)) {}

  value_type* get_element() {
    return &_element;
  }

  friend void swap(typed_node& lhs, typed_node& rhs) noexcept {
    std::swap(lhs._element, rhs._element);
  }

private:
  value_type _element;
};

template <typename TL, typename TR>
struct sentinel_node
    : public tagged_node<left_tag>
    , public tagged_node<right_tag> {
public:
  using left_node = tagged_node<left_tag>;
  using right_node = tagged_node<right_tag>;

  using left_value_type = TL;
  using right_value_type = TR;

public:
  sentinel_node() = default;
  sentinel_node(const sentinel_node&) = default;
  sentinel_node& operator=(const sentinel_node&) = default;
  sentinel_node& operator=(sentinel_node&&) noexcept = default;

  sentinel_node(sentinel_node&& other) noexcept
      : tagged_node<left_tag>(std::move(other))
      , tagged_node<right_tag>(std::move(other)) {}
};

template <typename TL, typename TR>
struct bimap_node
    : public typed_node<TL, left_tag>
    , public typed_node<TR, right_tag> {
public:
  using left_node = typed_node<TL, left_tag>;
  using right_node = typed_node<TR, right_tag>;

  using left_value_type = TL;
  using right_value_type = TR;

  template <typename, typename, typename, typename, typename>
  friend class bimap_impl;

public:
  bimap_node() = default;
  bimap_node(const bimap_node&) = default;
  bimap_node(bimap_node&&) = default;
  bimap_node& operator=(const bimap_node&) = default;
  bimap_node& operator=(bimap_node&&) = default;

  template <typename L, typename R>
  bimap_node(L&& first, R&& second)
      : typed_node<TL, left_tag>(std::forward<L>(first))
      , typed_node<TR, right_tag>(std::forward<R>(second)) {}

  friend void swap(bimap_node& a, bimap_node& b) noexcept {
    swap(static_cast<left_node>(a), static_cast<left_node>(b));
    swap(static_cast<right_node>(a), static_cast<right_node>(b));
  }

  template <typename Type, typename Tag>
  static control_node* get_control_base(bimap_node* node) {
    return static_cast<control_node*>(static_cast<tagged_node<Tag>*>(static_cast<typed_node<Type, Tag>*>(node)));
  }
};
