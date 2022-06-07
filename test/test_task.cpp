#include "task.hpp"
#include <thread>
#include <vector>

#define BOOST_TEST_MODULE io1::progress test
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(default_construction)
{
  bool flag = false;
  auto const finished = [&flag](bool) { flag = true; };

  {
    io1::progress::basic_task<100> t;
    t.set_finish_callback(finished);
  }

  BOOST_CHECK(!flag);
  return;
}

BOOST_AUTO_TEST_CASE(range_semantic)
{
  size_t index = 0;
  bool started = false;
  bool finished = false;

  std::vector<int> const vec = {1, 2, 3, 4, 5};

  auto const start = [&index, &started, &finished](std::string_view name)
  {
    BOOST_CHECK(std::empty(name));
    BOOST_CHECK_EQUAL(0, index);
    BOOST_CHECK(!started);
    BOOST_CHECK(!finished);
    started = true;
  };

  auto const report = [&index, &started, &finished](float progress)
  {
    constexpr float expected[] = {0.2f, 0.4f, 0.6f, 0.8f, 1.f};
    BOOST_CHECK_EQUAL(progress, expected[index]);
    BOOST_CHECK(started);
    BOOST_CHECK(!finished);
    ++index;
  };

  auto const finish = [&index, &started, &finished, &vec](bool success)
  {
    BOOST_CHECK_EQUAL(vec.size(), index);
    BOOST_CHECK(success);
    BOOST_CHECK(started);
    BOOST_CHECK(!finished);
    finished = true;
  };

  {
    io1::progress::basic_task<100> t({start, report, finish});
    size_t i = 0;
    for (auto const & v : vec | t)
    {
      BOOST_CHECK_EQUAL(v, vec[i++]);
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
  }

  BOOST_CHECK(started);
  BOOST_CHECK(finished);

  return;
}