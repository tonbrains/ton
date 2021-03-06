/*
    This file is part of TON Blockchain Library.

    TON Blockchain Library is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 2 of the License, or
    (at your option) any later version.

    TON Blockchain Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with TON Blockchain Library.  If not, see <http://www.gnu.org/licenses/>.

    Copyright 2017-2020 Telegram Systems LLP
*/

#include "td/utils/common.h"
#include "td/utils/misc.h"
#include "td/utils/OptionParser.h"
#include "td/utils/Slice.h"
#include "td/utils/Status.h"
#include "td/utils/tests.h"

TEST(OptionParser, run) {
  td::OptionParser options;
  options.set_description("test description");

  td::string exename = "exename";
  td::vector<td::string> args;
  auto run_option_parser = [&](td::string command_line) {
    args = td::full_split(command_line, ' ');
    td::vector<char *> argv;
    argv.push_back(&exename[0]);
    for (auto &arg : args) {
      argv.push_back(&arg[0]);
    }
    return options.run(static_cast<int>(argv.size()), &argv[0]);
  };

  td::uint64 chosen_options = 0;
  td::vector<td::string> chosen_parameters;
  auto test_success = [&](td::string command_line, td::uint64 expected_options,
                          td::vector<td::string> expected_parameters, td::vector<td::string> expected_result) {
    chosen_options = 0;
    chosen_parameters.clear();
    auto result = run_option_parser(command_line);
    ASSERT_TRUE(result.is_ok());
    ASSERT_EQ(expected_options, chosen_options);
    ASSERT_EQ(expected_parameters, chosen_parameters);
    ASSERT_EQ(expected_result.size(), result.ok().size());
    for (size_t i = 0; i < expected_result.size(); i++) {
      ASSERT_STREQ(expected_result[i], td::string(result.ok()[i]));
    }
  };
  auto test_fail = [&](td::string command_line) {
    auto result = run_option_parser(command_line);
    ASSERT_TRUE(result.is_error());
  };

  options.add_option('q', "", "", [&] { chosen_options += 1; });
  options.add_option('\0', "http-port2", "", [&] { chosen_options += 10; });
  options.add_option('p', "http-port", "", [&](td::Slice parameter) {
    chosen_options += 100;
    chosen_parameters.push_back(parameter.str());
  });
  options.add_option('v', "test", "", [&] { chosen_options += 1000; });

  test_fail("-http-port2");
  test_success("-", 0, {}, {"-"});
  test_fail("--http-port");
  test_fail("--http-port3");
  test_fail("--http-por");
  test_fail("--http-port2=1");
  test_fail("--q");
  test_fail("-qvp");
  test_fail("-p");
  test_fail("-u");
  test_success("-q", 1, {}, {});
  test_success("-vvvvvvvvvv", 10000, {}, {});
  test_success("-qpv", 101, {"v"}, {});
  test_success("-qp -v", 101, {"-v"}, {});
  test_success("-qp --http-port2", 101, {"--http-port2"}, {});
  test_success("-qp -- -v", 1101, {"--"}, {});
  test_success("-qvqvpqv", 2102, {"qv"}, {});
  test_success("aba --http-port2 caba --http-port2 dabacaba", 20, {}, {"aba", "caba", "dabacaba"});
  test_success("das -pqwerty -- -v asd --http-port", 100, {"qwerty"}, {"das", "-v", "asd", "--http-port"});
  test_success("-p option --http-port option2 --http-port=option3 --http-port=", 400,
               {"option", "option2", "option3", ""}, {});
  test_success("", 0, {}, {});
  test_success("a", 0, {}, {"a"});
  test_success("", 0, {}, {});
}
