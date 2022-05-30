#pragma once
#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <string>

namespace io1::progress
{
  using start_callback_t = std::function<void(std::string_view)>;
  using progress_callback_t = std::function<void(float)>;
  using finish_callback_t = std::function<void(bool)>;

  struct report_functions
  {
    start_callback_t start{[](auto) {}};
    progress_callback_t progress{[](auto) {}};
    finish_callback_t finish{[](auto) {}};
  };

  template <unsigned int REPORT_INTERVAL_MS>
  class report_interval
  {
  public:
    report_interval() : next_report_(now() + report_interval_){};

    [[nodiscard]] bool report_now() const noexcept
    {
      auto const n = now();
      if (n < next_report_) return false;

      next_report_ = n + report_interval_;
      return true;
    };

  private:
    [[nodiscard]] static auto now() noexcept { return std::chrono::steady_clock::now(); };

  private:
    using time_point_t = decltype(std::chrono::steady_clock::now());
    std::chrono::milliseconds const report_interval_{REPORT_INTERVAL_MS};
    mutable time_point_t next_report_;
  };

  template <>
  class report_interval<0>
  {
  public:
    [[nodiscard]] bool report_now() const noexcept { return true; };
  };

  template <unsigned int REPORT_INTERVAL_MS = 100>
  class basic_task : private report_interval<REPORT_INTERVAL_MS>
  {
  public:
    basic_task() = default;
    explicit basic_task(std::string name) noexcept : name_(std::move(name)){};
    basic_task(basic_task const &) = delete;
    basic_task(basic_task &&) = default;
    basic_task & operator=(basic_task const &) = delete;
    basic_task & operator=(basic_task &&) = default;
    ~basic_task() noexcept
    {
      if (started()) report_.finish(success());
    };

    explicit basic_task(report_functions report) noexcept : report_(std::move(report)){};

    void start(size_t target) noexcept
    {
      assert(target && "There is no point in starting a task with a null target.");
      target_ = target;
      progress_ = 0;
      report_.start(name_);
    };

    basic_task & operator++() noexcept { return operator+=(1); };

    basic_task & operator+=(size_t n) noexcept
    {
      progress_ += n;
      report_progress();
      return *this;
    };
    [[nodiscard]] bool success() const noexcept { return target_ <= progress_; };
    [[nodiscard]] bool started() const noexcept { return target_ > 0; };

  public:
    void set_start_callback(start_callback_t f) noexcept { report_.start = f; };
    void set_progress_callback(progress_callback_t f) noexcept { report_.progress = f; };
    void set_finish_callback(finish_callback_t f) noexcept { report_.finish = f; };

  private:
    [[nodiscard]] auto calculate_progress() const noexcept { return (100.f * progress_) / target_; };

    void report_progress() const noexcept
    {
      if (this->report_now()) report_.progress(calculate_progress());
    }

  private:
    std::string name_;
    size_t nesting_{0};
    size_t target_{0};
    size_t progress_{0};
    report_functions report_;
  };

  template <typename ITERATOR>
  class task_iterator_wrapper
  {
  public:
    task_iterator_wrapper(ITERATOR it, basic_task<> & t) : t_(&t), it_(it){};
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
    basic_task<> * t_;
  };

  template <typename RANGE>
  class task_view
  {
  public:
    using iterator_type = typename RANGE::const_iterator;
    explicit task_view(RANGE const & r, basic_task<> & t) noexcept : r_(&r), t_(&t) { t_->start(std::size(*r_)); };

    auto begin() const { return task_iterator_wrapper<iterator_type>{std::begin(*r_), *t_}; };
    auto end() const { return task_iterator_wrapper<iterator_type>{std::end(*r_), *t_}; };

  private:
    RANGE const * r_;
    basic_task<> * t_;
  };

  template <typename RANGE>
  auto operator|(RANGE const & r, basic_task<> & t)
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