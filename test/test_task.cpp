#include "task.hpp"
#include <thread>
#include <vector>

#define BOOST_TEST_MODULE io1::progress test
#include <boost/test/unit_test.hpp>
namespace
{
  class progression_check
  {
  public:
    explicit progression_check(std::vector<unsigned int> expected_steps, std::string expected_name = std::string())
        : expected_steps_(expected_steps), expected_name_(expected_name) {};

    [[nodiscard]] operator io1::progress::report_functions()
    {
      auto const start = [this](std::string_view name)
      {
        BOOST_CHECK_EQUAL(name, expected_name_);
        started_ = true;
      };

      auto const report = [this](unsigned int progress)
      { BOOST_CHECK_EQUAL(progress, expected_steps_[progress_index_++]); };

      auto const finish = [this](bool success)
      {
        BOOST_CHECK_EQUAL(success, expected_steps_.back() == 100);
        finished_ = true;
      };

      return {start, report, finish};
    };

    auto started() const { return started_; };
    auto finished() const { return finished_; };
    auto progressed() const
    {
      auto const temp = progress_index_ > last_progress_index_;
      last_progress_index_ = progress_index_;
      return temp;
    }

  private:
    std::vector<unsigned int> expected_steps_;
    std::string expected_name_;
    mutable size_t last_progress_index_{0};
    size_t progress_index_{0};
    bool started_{false};
    bool finished_{false};
  };
}

BOOST_AUTO_TEST_CASE(default_construction)
{
  bool flag = false;
  auto const finished = [&flag](bool) { flag = true; };

  {
    // unstarted tasks do not report on destruction
    io1::progress::pc_task t;
    t.set_finish_callback(finished);
  }

  BOOST_CHECK(!flag);
  return;
}

BOOST_AUTO_TEST_CASE(basic_incrementations_and_success)
{
  progression_check check({20 ,40, 80, 100});

  {
    io1::progress::pc_task t(check);
    BOOST_CHECK(!check.started());
    BOOST_CHECK(!check.finished());
    BOOST_CHECK(!check.progressed());

    t.start(5);
    BOOST_CHECK(check.started());
    BOOST_CHECK(!check.finished());
    BOOST_CHECK(!check.progressed());

    ++t;
    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());

    ++t;
    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());

    t += 2;
    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());

    ++t;
    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());
  }

  BOOST_CHECK(!check.progressed());
  BOOST_CHECK(check.finished());

  return;
}

BOOST_AUTO_TEST_CASE(basic_incrementations_and_failure)
{
  progression_check check({20, 40, 80});

  {
    io1::progress::pc_task t(check);
    BOOST_CHECK(!check.started());
    BOOST_CHECK(!check.finished());
    BOOST_CHECK(!check.progressed());

    t.start(5);
    BOOST_CHECK(check.started());
    BOOST_CHECK(!check.finished());
    BOOST_CHECK(!check.progressed());

    ++t;
    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());

    ++t;
    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());

    t += 2;
    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());
  }

  BOOST_CHECK(!check.progressed());
  BOOST_CHECK(check.finished());

  return;
}

BOOST_AUTO_TEST_CASE(range_semantic)
{
  progression_check check({20, 40, 60, 80, 100});

  {
    io1::progress::basic_task<100,100> t(check);

    std::vector<int> const vec = {1, 2, 3, 4, 5};

    BOOST_CHECK(!check.started());
    BOOST_CHECK(!check.progressed());
    BOOST_CHECK(!check.finished());

    for (auto const & v : vec | t)
    {
      static size_t i = 0;
      if (0 == i)
      {
        BOOST_CHECK(check.started());
        BOOST_CHECK(!check.progressed());
        BOOST_CHECK(!check.finished());
      }
      else
      {
        BOOST_CHECK(check.progressed());
        BOOST_CHECK(!check.finished());
      }

      BOOST_CHECK_EQUAL(v, vec[i++]);
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    BOOST_CHECK(check.progressed());
    BOOST_CHECK(!check.finished());
  }

  BOOST_CHECK(!check.progressed());
  BOOST_CHECK(check.finished());

  return;
}