//
// Created by squadrick on 24/8/19.
//

#ifndef shadesmar_TMP_H
#define shadesmar_TMP_H

#include <sys/stat.h>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <random>
#include <string>

#if __cplusplus >= 201703L
#ifdef __cpp_lib_filesystem
#include <filesystem>
#else
#include <experimental/filesystem>
namespace std {
namespace filesystem = experimental::filesystem;
}
#endif
#else
#include <dirent.h>
#include <stdlib.h>
#endif

namespace shm::memory::tmp {
std::string const default_chars =
    "abcdefghijklmnaoqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

std::string const tmp_prefix = "/tmp/shm/";

inline std::string
random_string(size_t len = 15,
              std::string const &allowed_chars = default_chars) {
  std::mt19937_64 gen{std::random_device()()};
  std::uniform_int_distribution<size_t> dist{0, allowed_chars.length() - 1};
  std::string ret;
  std::generate_n(std::back_inserter(ret), len,
                  [&] { return allowed_chars[dist(gen)]; });
  return ret;
}

inline bool file_exists(const std::string &file_name) {
  // POSIX only
  struct stat buffer {};
  return (stat(file_name.c_str(), &buffer) == 0);
}

inline void write(const std::string &name) {
  if (!file_exists(tmp_prefix)) {
#if __cplusplus >= 201703L
    std::filesystem::create_directories(tmp_prefix);
#else
    auto opts = S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH;
    mkdir(tmp_prefix.c_str(), opts);
#endif
  }
  std::fstream file;
  std::string file_name = tmp_prefix + random_string();
  file.open(file_name, std::ios::out);

  file << name.c_str() << std::endl;
  file.close();
}

inline std::vector<std::string> get_tmp_names() {
  std::vector<std::string> names{};
  if (!file_exists(tmp_prefix)) {
    return names;
  }

#if __cplusplus >= 201703L
  for (const auto &entry : std::filesystem::directory_iterator(tmp_prefix)) {
    std::fstream file;
    file.open(entry.path().generic_string(), std::ios::in);
    std::string name;
    file >> name;
    names.push_back(name);
  }
#else
  struct dirent *entry = nullptr;
  DIR *dp = nullptr;
  dp = opendir(tmp_prefix.c_str());
  if (dp != nullptr) {
    while ((entry = readdir(dp))) {
      std::fstream file;
      file.open(tmp_prefix + entry->d_name, std::ios::in);
      std::string name;
      file >> name;
      names.push_back(name);
    }
  }
  closedir(dp);
#endif

  return names;
}

inline bool exists(const std::string &name) {
  auto existing_names = get_tmp_names();
  for (auto &existing_name : existing_names) {
    if (existing_name == name) {
      return true;
    }
  }
  return false;
}

inline void delete_topics() {
#if __cplusplus >= 201703L
  std::filesystem::remove_all(tmp_prefix);
#else
  system(("rm -rf " + tmp_prefix).c_str());
#endif
}
} // namespace shm::memory::tmp
#endif // shadesmar_TMP_H