#pragma once
#include <functional>
#include <iostream>

namespace io1::progress
{

  struct report_functions
  {
    std::function<void()> start{[] {}};
    std::function<void(float)> progress{[](auto) {}};
    std::function<void(bool)> finish{[](auto) {}};
  };

  class task
  {
  public:
    task() = default;
    task(task const &) = delete;
    task(task &&) = default;
    task & operator=(task const &) = delete;
    task & operator=(task &&) = default;
    ~task() noexcept { report_.finish(success()); };

    explicit task(report_functions report) noexcept : report_(std::move(report)){};

    void start(size_t target) noexcept
    {
      target_ = target;
      progress_ = 0;
      report_.start();
    };

    task & operator++()
    {
      ++progress_;
      report_.progress((100.f * progress_) / target_);
      return *this;
    };
    [[nodiscard]] bool success() const noexcept { return target_ <= progress_; };

  private:
    size_t target_;
    size_t progress_;
    report_functions report_;
  };

  template <typename ITERATOR>
  class task_iterator_wrapper
  {
  public:
    task_iterator_wrapper(ITERATOR it, task & t) : t_(&t), it_(it){};
    using value_type = typename ITERATOR::value_type;
    const value_type operator*() const noexcept { return *it_; };
    const value_type & operator->() const noexcept { return it_.operator->(); };
    task_iterator_wrapper & operator++()
    {
      ++it_;
      ++(*t_);
      return *this;
    };

    bool operator<(task_iterator_wrapper rhs) const { return it_ < rhs.it_; };
    bool operator==(task_iterator_wrapper rhs) const { return it_ == rhs.it_; };

  private:
    ITERATOR it_;
    task * t_;
  };

  template <typename RANGE>
  class task_view
  {
  public:
    using iterator_type = typename RANGE::const_iterator;
    explicit task_view(RANGE const & r, task & t) noexcept : r_(&r), t_(&t) { t_->start(std::size(*r_)); };

    auto begin() const { return task_iterator_wrapper<iterator_type>{std::begin(*r_), *t_}; };
    auto end() const { return task_iterator_wrapper<iterator_type>{std::end(*r_), *t_}; };

  private:
    RANGE const * r_;
    task * t_;
  };

  template <typename RANGE>
  auto operator|(RANGE const & r, task & t)
  {
    return task_view<RANGE>(r, t);
  };
} // namespace io1::progress
  /*
  #include <algorithm>
  #include <assert.h>
  #include <concepts>
  #include <iostream>
  #include <ranges>
  #include <string>
  #include <vector>
  
  namespace rg = std::ranges;
  
  template<rg::input_range R> requires rg::view<R>
  class custom_take_view : public rg::view_interface<custom_take_view<R>>
  {
  private:
      R                                         base_ {};
      std::iter_difference_t<rg::iterator_t<R>> count_ {};
      rg::iterator_t<R>                         iter_ {std::begin(base_)};
  public:
      custom_take_view() = default;
  
      constexpr custom_take_view(R base, std::iter_difference_t<rg::iterator_t<R>> count)
          : base_(base)
          , count_(count)
          , iter_(std::begin(base_))
      {}
  
      constexpr R base() const &
      {return base_;}
      constexpr R base() &&
      {return std::move(base_);}
  
      constexpr auto begin() const
      {return iter_;}
      constexpr auto end() const
      { return std::next(iter_, count_); }
  
      constexpr auto size() const requires rg::sized_range<const R>
      {
          const auto s = rg::size(base_);
          const auto c = static_cast<decltype(s)>(count_);
          return s < c ? 0 : s - c;
      }
  };
  
  template<class R>
  custom_take_view(R&& base, std::iter_difference_t<rg::iterator_t<R>>)
      -> custom_take_view<rg::views::all_t<R>>;
  
  namespace details
  {
      struct custom_take_range_adaptor_closure
      {
          std::size_t count_;
          constexpr custom_take_range_adaptor_closure(std::size_t count): count_(count)
          {}
  
          template <rg::viewable_range R>
          constexpr auto operator()(R && r) const
          {
              return custom_take_view(std::forward<R>(r), count_);
          }
      } ;
  
      struct custom_take_range_adaptor
      {
          template<rg::viewable_range R>
          constexpr auto operator () (R && r, std::iter_difference_t<rg::iterator_t<R>> count)
          {
              return custom_take_view( std::forward<R>(r), count ) ;
          }
  
          constexpr auto operator () (std::size_t count)
          {
              return custom_take_range_adaptor_closure(count);
          }
      };
  
      template <rg::viewable_range R>
      constexpr auto operator | (R&& r, custom_take_range_adaptor_closure const & a)
      {
          return a(std::forward<R>(r)) ;
      }
  }
  
  namespace views
  {
      details::custom_take_range_adaptor custom_take;
  }*/