#include "task.hpp"
#include <vector>

#define BOOST_TEST_MODULE io1::progress test
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE(range)
{
  io1::progress::task t;
  std::vector<int> vec = { 1, 2, 3, 4, 5 };

  for (auto const & v : vec | t) std::cout << "parsing " << v << '\n';

  // should display "start, 20%, 40%, 60%, 80%, 100%, sucess
}