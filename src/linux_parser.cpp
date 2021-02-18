#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line, key, value;

  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);

      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  filestream.close();

  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, kernel, line, version;

  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> kernel >> version;
  }
  return version;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;

  while ((file = readdir(directory)) != nullptr) {
    if (file->d_type == DT_DIR) {
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);

  return pids;
}

template <typename T>
T findValueByKey(std::string const &keyFilter, std::string const &filename) {
  std::string line, key;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          return value;
        }
      }
    }
  }
  stream.close();

  return value;
};

template <typename T>
T getValueOfFile(std::string const &filename) {
  std::string line;
  T value;

  std::ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> value;
  }
  stream.close();

  return value;
};

float LinuxParser::MemoryUtilization() {
  string memTotal = filterMemTotalString;
  string memFree = filterMemFreeString;

  float Total = findValueByKey<float>(memTotal, kMeminfoFilename);
  float Free = findValueByKey<float>(memFree, kMeminfoFilename);

  return (Total - Free) / Total;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  std::string line, upTime;

  std::ifstream filestream(LinuxParser::kProcDirectory + kUptimeFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> upTime;
      break;
    }
  }
  filestream.close();

  if (upTime.empty() || upTime == "")
    return 0;

  return std::stol(upTime);
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  return LinuxParser::ActiveJiffies()
         + LinuxParser::IdleJiffies();
}

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  long totalTime;
  std::string line, value;
  std::vector <std::string> values;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    while (linestream >> value) {
      values.push_back(value);
    }
  }
  filestream.close();

  try {
    long utime = 0, stime = 0, cutime = 0, cstime = 0;
    if (std::all_of(values[13].begin(), values[13].end(), isdigit))
      utime = stol(values[13]);

    if (std::all_of(values[14].begin(), values[14].end(), isdigit))
      stime = stol(values[14]);

    if (std::all_of(values[15].begin(), values[15].end(), isdigit))
      cutime = stol(values[15]);

    if (std::all_of(values[16].begin(), values[16].end(), isdigit))
      cstime = stol(values[16]);

    totalTime = (utime + stime + cutime + cstime);

    return totalTime / sysconf(_SC_CLK_TCK);
  } catch (...) {
    totalTime = 0;
  }
  return totalTime;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  auto jiffies = CpuUtilization();
  long jiffiesLong;

  try {
    return stol(jiffies[CPUStates::kUser_]) + stol(jiffies[CPUStates::kNice_]) +
           stol(jiffies[CPUStates::kSystem_]) +
           stol(jiffies[CPUStates::kIRQ_]) +
           stol(jiffies[CPUStates::kSoftIRQ_]) +
           stol(jiffies[CPUStates::kSteal_]);
  } catch (...) {
    jiffiesLong = 0;
  }

  return jiffiesLong;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  auto jiffies = CpuUtilization();

  // If empty argument returns zero
  if (jiffies.empty())
    return 0;

  return stol(jiffies[CPUStates::kIdle_]) + stol(jiffies[CPUStates::kIOwait_]);
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, cpu, value;
  vector<string> jiffies;

  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);

    linestream >> cpu;

    while (linestream >> value) {
      jiffies.push_back(value);
    }
  }
  filestream.close();

  return jiffies;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  std::string line, key, value;
  int processes = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);

      while (linestream >> key >> value) {
        if (value.empty() || value == "")
          value = 0.0;

        if (key == filterProcesses) {
          processes = std::stoi(value);
          break;
        }
      }
    }
  }
  filestream.close();

  return processes;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::string line, key, value;
  int processes = 0;

  std::ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);

      while (linestream >> key >> value) {
        if (value.empty() || value == "")
          value = 0.0;

        if (key == filterRunningProcesses) {
          processes = std::stoi(value);
          break;
        }
      }
    }
  }
  filestream.close();

  return processes;
}

// TODO: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  return std::string(getValueOfFile<std::string>(std::to_string(pid) + kCmdlineFilename));
}

// TODO: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line, key, mem_str;
  long memo;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);

      linestream >> key;

      if (key == filterProcMem) {
        linestream >> memo;
        memo /= 1000;
        mem_str = std::to_string(memo);
        break;
      }
    }
  }
  filestream.close();

  return mem_str;
}

// TODO: Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string line, key, uid;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatusFilename);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == filterUID) {
        linestream >> uid;
        break;
      }
    }
  }
  filestream.close();

  return uid;
}

// TODO: Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string id, x, temp, line;
  string name = filterDefault;
  string uid = Uid(pid);

  std::ifstream filestream(kPasswordPath);

  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);

      linestream >> temp >> x >> id;
      if (id == uid) {
        name = temp;
        break;
      }
    }
  }
  filestream.close();

  return name;
}

// TODO: Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, value;
  vector<string> values;
  long start_time = 0;

  std::ifstream filestream(kProcDirectory + std::to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    while (linestream >> value) {
      values.push_back(value);
    }
  }
  filestream.close();

  try {
    start_time = UpTime() - stol(values[21]) / sysconf(_SC_CLK_TCK);
  } catch (...) {
    start_time = 0;
  }
  return start_time;
}