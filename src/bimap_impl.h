#pragma once

#include "bimap_iterator.h"
#include "bimap_node.h"

template <typename, typename, typename, typename>
class bimap;

template <typename LeftT, typename RightT, typename CompL, typename CompR, typename Tag>
class bimap_impl : CompL {
public:
  using value_type = std::conditional_t<std::is_same_v<Tag, left_tag>, LeftT, RightT>;
  using iterator = std::conditional_t<
      std::is_same_v<Tag, left_tag>,
      bimap_iterator<LeftT, RightT, left_tag>,
      bimap_iterator<RightT, LeftT, right_tag>>;
  using comparator = CompL;
  using original_bimap = std::conditional_t<
      std::is_same_v<Tag, left_tag>,
      bimap<LeftT, RightT, CompL, CompR>,
      bimap<LeftT, RightT, CompR, CompL>>;

public:
  bimap_impl() = default;
  bimap_impl(bimap_impl&& other) = default;
  bimap_impl& operator=(bimap_impl&&) = default;
  ~bimap_impl() = default;

  bimap_impl(const bimap_impl& other) = default;

  bimap_impl& operator=(const bimap_impl& other) {
    bimap_impl copy(other);
    swap(copy);
    return *this;
  }

  explicit bimap_impl(comparator cmp)
      : comparator(std::move(cmp)) {}

public:
  void insert(control_node* element, iterator pointer) noexcept {
    control_node& sentinel = _get_sentinel();

    if (empty()) {
      element->connect_less(&sentinel);
      sentinel.connect_less(element);
    } else { // !empty()
      if (pointer._ptr->get_down_left() == &sentinel) {
        pointer._ptr->connect_less(element);
        element->connect_less(&sentinel);
      } else if (pointer._ptr->get_down_left() == nullptr) {
        pointer._ptr->connect_less(element);
      } else {
        (--pointer)._ptr->connect_great(element);
      }
    }
  }

  control_node* erase(iterator it) noexcept {
    control_node* delete_ptr = it._ptr;
    control_node* next_ptr = (++it)._ptr;

    if (delete_ptr->is_last()) {
      delete_ptr->set_via_parent(nullptr);
    } else if (delete_ptr->get_down_right() == nullptr) {
      control_node* less_node = delete_ptr->get_down_left();
      delete_ptr->get_down_left()->set_parent(delete_ptr->get_parent());

      delete_ptr->set_via_parent(less_node);
    } else {
      control_node* greater_node = delete_ptr->get_down_right();
      delete_ptr->get_down_right()->set_parent(delete_ptr->get_parent());

      delete_ptr->set_via_parent(greater_node);

      if (delete_ptr->get_down_left() != nullptr) {
        next_ptr->set_left(delete_ptr->get_down_left());
        delete_ptr->get_down_left()->set_parent(next_ptr);
      }
    }

    return next_ptr;
  }

  iterator erase(const value_type& element) noexcept {
    iterator it = find(element);
    return (it == end()) ? end() : erase(it);
  }

  iterator begin() const {
    return iterator(_get_sentinel().get_parent());
  }

  iterator end() const {
    return iterator(const_cast<control_node*>(&_get_sentinel()));
  }

  iterator find(const value_type& element) const {
    iterator it = lower_bound(element);

    bool condition_exists = !exists_by_lb(it, element);
    return condition_exists ? end() : it;
  }

  iterator lower_bound(const value_type& element) const {
    control_node* bound = _both_bounds(element).first;
    return (bound == nullptr) ? end() : iterator(bound);
  }

  iterator upper_bound(const value_type& element) const {
    control_node* bound = _both_bounds(element).second;
    return (bound == nullptr) ? end() : iterator(bound);
  }

  bool empty() noexcept {
    return _get_sentinel().is_right_child();
  }

  void swap(bimap_impl& other) noexcept {
    std::swap(_get_sentinel(), other._get_sentinel());
    std::swap(static_cast<comparator&>(*this), static_cast<comparator&>(other));
  }

  template <typename V>
  bool exists_by_lb(iterator it, const V& elem) const {
    return (it != end() && !comparator ::operator()(elem, *it) && !comparator ::operator()(*it, elem));
  }

  friend bool operator==(const bimap_impl& lhs, const bimap_impl& rhs) {
    auto equal_check = [lhs](iterator it1, iterator it2) {
      return !lhs(*it1, *it2) && !lhs(*it2, *it1);
    };

    auto equal_check_rev = [lhs](iterator::other_iterator it1, iterator::other_iterator it2) {
      return !lhs(*it1, *it2) && !lhs(*it2, *it1);
    };

    bool condition_equals = true;
    auto itL = lhs.begin();
    auto itR = rhs.begin();
    for (; itR != rhs.end() && itL != lhs.end(); ++itL, ++itR) {
      condition_equals &= equal_check(itL, itR);
      condition_equals &= equal_check_rev(itL.flip(), itR.flip());
    }

    return condition_equals && itL == lhs.end() && itR == rhs.end();
  }

private:
  static const value_type& _pull_value(control_node* node) {
    return *iterator(node);
  }

  const original_bimap& _get_original() const {
    return static_cast<const original_bimap&>(*this);
  }

  original_bimap& _get_original() {
    return static_cast<original_bimap&>(*this);
  }

  const control_node& _get_sentinel() const {
    return static_cast<const control_node&>(static_cast<const tagged_node<Tag>&>(_get_original()._sentinel));
  }

  control_node& _get_sentinel() {
    return static_cast<control_node&>(static_cast<tagged_node<Tag>&>(_get_original()._sentinel));
  }

  std::pair<control_node*, control_node*> _both_bounds(const value_type& value) const {
    control_node* cur = _get_sentinel().get_down_left();
    control_node* LBound = nullptr;
    control_node* UBound = nullptr;

    while (cur != &_get_sentinel() && cur != nullptr) {
      if (this->operator()(_pull_value(cur), value)) {
        cur = cur->get_down_right();
        continue;
      }
      LBound = cur;
      UBound = (this->operator()(value, _pull_value(cur))) ? cur : UBound;

      cur = cur->get_down_left();
    }

    return {LBound, UBound};
  }
};
