#if __cplusplus < 201703
#error "Must be С++17"
#endif
#include <args.hxx>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <map>
#include <utility>

#include "rapidcsv/src/rapidcsv.h"
#include "spdlog/spdlog.h"

/* mail, name */
typedef std::map<std::string, std::string> LoginName;
/* mail, time */
typedef std::map<std::string, float> MouthArray;
/* mail, motharray*/
typedef std::map<std::string, MouthArray> LoggedMonth;

#define DEFAULT_LEVEL (3)

spdlog::level::level_enum intToLevel(int value) {
  switch (value) {
    case SPDLOG_LEVEL_TRACE:
      return spdlog::level::trace;
      break;
    case SPDLOG_LEVEL_DEBUG:
      return spdlog::level::debug;
      break;
    case SPDLOG_LEVEL_INFO:
      return spdlog::level::info;
      break;
    case SPDLOG_LEVEL_WARN:
      return spdlog::level::warn;
      break;
    case SPDLOG_LEVEL_ERROR:
      return spdlog::level::err;
      break;
    case SPDLOG_LEVEL_CRITICAL:
      return spdlog::level::critical;
      break;
    case SPDLOG_LEVEL_OFF:
      return spdlog::level::off;
      break;

    default:
      return spdlog::level::info;
      break;
  }
}
std::string getLevelHelp() {
  std::string result{"Loging levels:\n"};
  std::string level_names[] SPDLOG_LEVEL_NAMES;
  for (auto it{0}; it != (sizeof(level_names) / sizeof(level_names[0])); it++) {
    result += std::to_string(it) + " - " + level_names[it] + '\n';
  }
  return result;
}

int main(int argc, char const* argv[]) try {
  spdlog::set_level(intToLevel(DEFAULT_LEVEL));
  LoginName UserNames{};
  LoggedMonth UserMonth{};
  std::filesystem::path InputFileName("./sample.csv");
  std::filesystem::path OutPutFileName("./main.csv");
  // CLI
  {
    args::ArgumentParser parser("This is a test program.", "Sam4uk wrote this");
    args::HelpFlag help(parser, "help", "Display this help menu",
                        {'h', "help"});
    args::ValueFlag<std::filesystem::path> InDataPath(
        parser, "input.csv", "The path to the file with the input data",
        {'i', "input"});
    args::ValueFlag<std::filesystem::path> OutDataPath(
        parser, "output.csv", "The path to the file for data storage",
        {'o', "output"});
    args::ValueFlag<int> LoggLevel(parser, std::to_string(DEFAULT_LEVEL),
                                   getLevelHelp(), {'l'});

    args::CompletionFlag completion(parser, {"complete"});
    try {
      parser.ParseCLI(argc, argv);
    }

    catch (const args::Help&) {
      std::cout << parser;
      return EXIT_SUCCESS;
    }

    if (InDataPath)
      InputFileName = InDataPath.Get();
    else
      throw std::runtime_error("No set input file name");
    if (LoggLevel) spdlog::set_level(intToLevel(LoggLevel.Get()));
    if (OutDataPath) OutPutFileName = OutDataPath.Get();
  }

  // Read file
  {
    if (!std::filesystem::exists(InputFileName))
      throw std::runtime_error("File " + InputFileName.string() + " no exist");
    if (std::filesystem::file_size(InputFileName) <= 1)
      spdlog::warn("File {} is posible empty", InputFileName.string());
    rapidcsv::Document doc(InputFileName, rapidcsv::LabelParams(0, -1),
                           rapidcsv::SeparatorParams(';'));
    spdlog::info("Opened file {0}. The table contains {1} records",
                 InputFileName.filename().string(), doc.GetRowCount());

    //читаємо всі рядки
    spdlog::trace("File {} reading", InputFileName.string());
    for (size_t row_itterator{0}; row_itterator != doc.GetRowCount();
         ++row_itterator) {
      spdlog::trace("Reading line {}", row_itterator);
      std::string UserMail{doc.GetCell<std::string>("email", row_itterator)};

      {
        std::string UserName{doc.GetCell<std::string>("Name", row_itterator)};
        auto foundUser{UserNames.find(UserMail)};
        if (foundUser == UserNames.end()) {
          spdlog::trace("Found new user email:{}", UserMail);
          UserNames.emplace(UserMail, UserName);
          MouthArray empty;
          UserMonth.emplace(UserMail, empty);
        }
      }
      {
        auto foundUser{UserMonth.find(UserMail)};
        std::string DateString{doc.GetCell<std::string>("date", row_itterator)};
        std::string KeyDate{DateString.substr(0, 4) + '-' +
                            DateString.substr(5, 2)};
        auto Hours{doc.GetCell<float>("logged hours", row_itterator)};

        if (foundUser != UserMonth.end()) {
          spdlog::trace("Add month {0} for {1}", UserMail, KeyDate);
          auto foundMonth{foundUser->second.find(KeyDate)};
          if (foundMonth == foundUser->second.end()) {
            foundUser->second.emplace(KeyDate, Hours);
          } else {
            foundMonth->second += Hours;
            spdlog::trace(foundMonth->second);
          }
        }
      }
    }
  }

  // RAW-CSV file writing
  {
    spdlog::trace("Try write file", OutPutFileName.string());
    std::ofstream output(OutPutFileName);
    spdlog::info("The data is stored in {}", OutPutFileName.string());
    output << "Name;Month;Total hours" << std::endl;
    for (auto user_itearator{UserMonth.cbegin()};
         user_itearator != UserMonth.cend(); user_itearator++) {
      auto [email, _MouthArray] = *user_itearator;

      for (auto user_date_iterator{_MouthArray.cbegin()};
           user_date_iterator != _MouthArray.cend(); user_date_iterator++) {
        auto [date, hours] = *user_date_iterator;

        spdlog::trace("{};{};{}", UserNames[email], date, hours);
        output << UserNames[email] << ';' << date << ';' << hours << std::endl;
      }
    }
    output.close();
    spdlog::info("Done!!!");
  }

  spdlog::trace("Normal flow end");
  return EXIT_SUCCESS;
}

catch (std::exception& err) {
  spdlog::error(err.what());
  return EXIT_FAILURE;
}

catch (...) {
  spdlog::error("Sompting wrong");
  return -EXIT_FAILURE;
}
