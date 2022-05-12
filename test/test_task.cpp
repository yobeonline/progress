#include "task.hpp"
#include <vector>

#define BOOST_TEST_MODULE io1::progress test
#include <boost/test/unit_test.hpp>

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
    constexpr float expected[] = {20.f, 40.f, 60.f, 80.f, 100.f};
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
    io1::progress::task t({start, report, finish});
    size_t i = 0;
    for (auto const & v : vec | t) BOOST_CHECK_EQUAL(v, vec[i++]);
  }

  BOOST_CHECK(started);
  BOOST_CHECK(finished);

  return;
}