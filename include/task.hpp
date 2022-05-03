#pragma once
#include <iostream>

namespace io1::progress{
  class task
  {
  public:
    void start(size_t target) noexcept { target_ = target; progress_ = 0; std::cout << "start\n"; };
    ~task() noexcept { if (success()) std::cout << "success\n"; else std::cout << "failed\n"; };
    task& operator++()
    {
      ++progress_;
      std::cout << (100.f * progress_) / target_ <<"%\n";
      return *this;
    };
    [[nodiscard]] bool success() const noexcept { return target_ <= progress_; };

  private:
    size_t target_;
    size_t progress_;
  };

  template<typename ITERATOR>
  class task_iterator_wrapper
  {
  public:
    task_iterator_wrapper(ITERATOR it, task& t) :t_(&t), it_(it) {};
    using value_type = typename ITERATOR::value_type;
    const value_type operator*() const noexcept { return *it_; };
    const value_type& operator->() const noexcept { return it_.operator->(); };
    task_iterator_wrapper& operator++() { ++it_; ++(*t_); return *this; };

    bool operator<(task_iterator_wrapper rhs) const { return it_ < rhs.it_; };
    bool operator==(task_iterator_wrapper rhs) const { return it_ == rhs.it_; };

  private:
    ITERATOR it_;
    task* t_;
  };

  template<typename RANGE>
  class task_view
  {
  public:
    using iterator_type = typename RANGE::const_iterator;
    explicit task_view(RANGE const& r, task & t) noexcept:r_(&r), t_(&t)
    {
      t_->start(std::size(*r_));
    };

    auto begin() const { return task_iterator_wrapper<iterator_type>{std::begin(*r_), *t_}; };
    auto end() const { return task_iterator_wrapper<iterator_type>{ std::end(*r_), *t_ }; };

  private:
    RANGE const* r_;
    task* t_;
  };

  template<typename RANGE>
  auto operator | (RANGE const& r, task & t)
  {
    return task_view<RANGE>(r, t);
  };
}
