#pragma once

#include "bimap_impl.h"

template <typename LeftT, typename RightT, typename LeftComp = std::less<LeftT>, typename RightComp = std::less<RightT>>
class bimap
    : bimap_impl<LeftT, RightT, LeftComp, RightComp, left_tag>
    , bimap_impl<LeftT, RightT, RightComp, LeftComp, right_tag> {
public:
  using left_bimap = bimap_impl<LeftT, RightT, LeftComp, RightComp, left_tag>;
  using right_bimap = bimap_impl<LeftT, RightT, RightComp, LeftComp, right_tag>;

  using left_iterator = left_bimap::iterator;
  using right_iterator = right_bimap::iterator;

  template <typename, typename, typename, typename, typename>
  friend class bimap_impl;

public:
  bimap(const bimap& other)
      : left_bimap(other._pull_left_tree())
      , right_bimap(other._pull_right_tree()) {
    try {
      for (auto it = other.begin_right(); it != other.end_right(); ++it) {
        insert(*it.flip(), *it);
      }
    } catch (...) {
      erase_right(begin_right(), end_right());
      throw;
    }
  }

  bimap(bimap&& other) noexcept
      : left_bimap(std::move(other._pull_left_tree()))
      , right_bimap(std::move(other._pull_right_tree()))
      , _sentinel(std::move(other._sentinel))
      , _size(std::exchange(other._size, 0)) {}

  bimap& operator=(const bimap& other) {
    if (this != &other) {
      bimap copy(other);
      swap(copy);
    }
    return *this;
  }

  bimap& operator=(bimap&& other) noexcept {
    if (this != &other) {
      bimap();
      swap(other);
    }
    return *this;
  }

  ~bimap() {
    erase_right(begin_right(), end_right());
  }

  explicit bimap(LeftComp compare_left = LeftComp({}), RightComp compare_right = RightComp({}))
      : left_bimap(std::move(compare_left))
      , right_bimap(std::move(compare_right)) {}

  friend void swap(bimap& lhs, bimap& rhs) noexcept {
    lhs.swap(rhs);
  }

  left_iterator insert(const LeftT& left, const RightT& right) {
    return _universal_insert(left, right);
  }

  left_iterator insert(const LeftT& left, RightT&& right) {
    return _universal_insert(left, std::move(right));
  }

  left_iterator insert(LeftT&& left, const RightT& right) {
    return _universal_insert(std::move(left), right);
  }

  left_iterator insert(LeftT&& left, RightT&& right) {
    return _universal_insert(std::move(left), std::move(right));
  }

  left_iterator erase_left(left_iterator itL) noexcept {
    right_iterator itR = itL.flip();

    _size--;
    control_node* next_node = left_bimap::erase(itL);
    right_bimap ::erase(itR);

    delete static_cast<bimap_node<LeftT, RightT>*>(static_cast<tagged_node<left_tag>*>(itL._ptr));
    return left_iterator(next_node);
  }

  right_iterator erase_right(right_iterator itR) noexcept {
    left_iterator itL = itR.flip();

    _size--;
    control_node* next_node = right_bimap::erase(itR);
    left_bimap ::erase(itL);

    delete static_cast<bimap_node<LeftT, RightT>*>(static_cast<tagged_node<right_tag>*>(itR._ptr));
    return right_iterator(next_node);
  }

  bool erase_left(const LeftT& left) {
    left_iterator itL = find_left(left);
    bool is_present = (itL != end_left());
    if (is_present) {
      erase_left(itL);
    }
    return is_present;
  }

  bool erase_right(const RightT& right) {
    right_iterator itR = find_right(right);
    bool is_present = (itR != end_right());
    if (is_present) {
      erase_right(itR);
    }
    return is_present;
  }

  left_iterator erase_left(left_iterator first, left_iterator last) noexcept {
    auto it = first;
    for (; it != last; it = erase_left(it)) {}
    return it;
  }

  right_iterator erase_right(right_iterator first, right_iterator last) noexcept {
    auto it = first;
    for (; it != last; it = erase_right(it)) {}
    return it;
  }

  left_iterator find_left(const LeftT& left) const {
    return left_bimap::find(left);
  }

  right_iterator find_right(const RightT& right) const {
    return right_bimap::find(right);
  }

  const RightT& at_left(const LeftT& key) const {
    left_iterator it = find_left(key);
    if (it == end_left()) {
      throw std::out_of_range("the requested item does not exist");
    }
    right_iterator it1 = it.flip();
    return *it1;
  }

  const LeftT& at_right(const RightT& key) const {
    right_iterator it = find_right(key);
    if (it == end_right()) {
      throw std::out_of_range("the requested item does not exist");
    }
    left_iterator it1 = it.flip();
    return *it1;
  }

  const RightT& at_left_or_default(const LeftT& key)
    requires (std::is_default_constructible_v<RightT>)
  {
    auto itL = lower_bound_left(key);
    if (left_bimap::template exists_by_lb<LeftT>(itL, key)) {
      return *itL.flip();
    }

    RightT element = RightT();
    auto itR = lower_bound_right(element);
    bool condition_exists = right_bimap::template exists_by_lb<RightT>(itR, element);

    auto* node = new bimap_node<LeftT, RightT>(key, std::move(element));
    if (condition_exists) {
      erase_right(itR++);
    }

    _separated_insert(node, itL, itR);
    return *(--itR);
  }

  const LeftT& at_right_or_default(const RightT& key)
    requires (std::is_default_constructible_v<LeftT>)
  {
    auto itR = lower_bound_right(key);
    if (right_bimap::template exists_by_lb<RightT>(itR, key)) {
      return *itR.flip();
    }

    LeftT element = LeftT();
    auto itL = lower_bound_left(element);
    bool condition_exists = left_bimap::template exists_by_lb<LeftT>(itL, element);

    auto* node = new bimap_node<LeftT, RightT>(std::move(element), key);
    if (condition_exists) {
      erase_left(itL++);
    }

    _separated_insert(node, itL, itR);
    return *(--itL);
  }

  left_iterator lower_bound_left(const LeftT& left) const {
    return left_bimap::lower_bound(left);
  }

  left_iterator upper_bound_left(const LeftT& left) const {
    return left_bimap::upper_bound(left);
  }

  right_iterator lower_bound_right(const RightT& right) const {
    return right_bimap::lower_bound(right);
  }

  right_iterator upper_bound_right(const RightT& right) const {
    return right_bimap::upper_bound(right);
  }

  left_iterator begin_left() const {
    return left_bimap::begin();
  }

  left_iterator end_left() const {
    return left_bimap::end();
  }

  right_iterator begin_right() const {
    return right_bimap::begin();
  }

  right_iterator end_right() const {
    return right_bimap::end();
  }

  bool empty() const {
    return (size() == 0);
  }

  std::size_t size() const {
    return _size;
  }

  void swap(bimap& other) noexcept {
    std::swap(_size, other._size);
    left_bimap ::swap(other._pull_left_tree());
    right_bimap::swap(other._pull_right_tree());
  }

  friend bool operator==(const bimap& lhs, const bimap& rhs) {
    bool is_equal_left = (lhs._pull_left_tree() == rhs._pull_left_tree());
    bool is_equal_right = (lhs._pull_right_tree() == rhs._pull_right_tree());
    return is_equal_left && is_equal_right;
  }

  friend bool operator!=(const bimap& lhs, const bimap& rhs) {
    return !(lhs == rhs);
  }

private:
  sentinel_node<LeftT, RightT> _sentinel{};
  std::size_t _size{0};

  template <typename U, typename V>
  left_iterator _universal_insert(U&& first_elem, V&& second_elem) {
    auto it1 = lower_bound_left(first_elem);
    auto it2 = lower_bound_right(second_elem);

    bool condition_exists = left_bimap::template exists_by_lb<U>(it1, std::forward<U>(first_elem)) ||
                            right_bimap::template exists_by_lb<V>(it2, std::forward<V>(second_elem));
    if (condition_exists) {
      return end_left();
    }

    auto* node = new bimap_node<LeftT, RightT>(std::forward<U>(first_elem), std::forward<V>(second_elem));
    return _separated_insert(node, it1, it2);
  }

  left_iterator _separated_insert(bimap_node<LeftT, RightT>* node, left_iterator itL, right_iterator itR) noexcept {
    control_node* left_ptr = bimap_node<LeftT, RightT>::template get_control_base<LeftT, left_tag>(node);
    control_node* right_ptr = bimap_node<LeftT, RightT>::template get_control_base<RightT, right_tag>(node);
    _size++;

    control_node::null_node(left_ptr);
    control_node::null_node(right_ptr);

    left_bimap::insert(left_ptr, itL);
    right_bimap::insert(right_ptr, itR);

    return left_iterator(left_ptr);
  }

  const left_bimap& _pull_left_tree() const {
    return static_cast<const left_bimap&>(*this);
  }

  left_bimap& _pull_left_tree() {
    return static_cast<left_bimap&>(*this);
  }

  const right_bimap& _pull_right_tree() const {
    return static_cast<const right_bimap&>(*this);
  }

  right_bimap& _pull_right_tree() {
    return static_cast<right_bimap&>(*this);
  }
};
