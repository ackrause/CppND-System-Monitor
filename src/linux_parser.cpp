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
  string line;
  string key;
  string value;
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
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, ignore_me, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> ignore_me >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
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

float LinuxParser::MemoryUtilization() {
  string line;
  string key;
  float value, mem_total, mem_free;
  std::ifstream filestream(kProcDirectory + kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          mem_total = value;
        } else if (key == "MemFree:") {
          mem_free = value;
        }
      }
    }
  }

  // Protect against dividing by zero if we misread the file
  return mem_total == 0 ? 0 : (mem_total - mem_free) / mem_total;
}

long LinuxParser::UpTime() {
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> uptime;
  }
  return uptime;
}

long LinuxParser::Jiffies() { return UpTime() * sysconf(_SC_CLK_TCK); }

long LinuxParser::ActiveJiffies(int pid) {
  long active_jiffies, user, kernel, children_user, children_kernel;
  string line, token;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    // skip first 13 tokens because user jiffies is the 14th
    for (int i = 0; i < 13; ++i) {
        linestream >> token;
    }

    linestream >> user >> kernel >> children_user >> children_kernel;
    active_jiffies = user + kernel + children_user + children_kernel;
  }

  return active_jiffies;
}

long LinuxParser::ActiveJiffies() {
  vector<long> cpu_utilization = CpuUtilization();

  return cpu_utilization[CPUStates::kUser_] + cpu_utilization[CPUStates::kNice_] +
         cpu_utilization[CPUStates::kSystem_] + cpu_utilization[CPUStates::kIRQ_] +
         cpu_utilization[CPUStates::kSoftIRQ_] + cpu_utilization[CPUStates::kSteal_] +
         cpu_utilization[CPUStates::kGuest_] + cpu_utilization[CPUStates::kGuestNice_];
}

long LinuxParser::IdleJiffies() {
  vector<long> cpu_utilization = CpuUtilization();

  return cpu_utilization[CPUStates::kIdle_] + cpu_utilization[CPUStates::kIOwait_];
}

vector<long> LinuxParser::CpuUtilization() {
  string line;
  string key;
  long token;
  vector<long> cpu_utilization;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "cpu") {
        while (linestream >> token) {
          cpu_utilization.push_back(token);
        }
      }
    }
  }

  return cpu_utilization;
}

int LinuxParser::TotalProcesses() {
  string line;
  string key;
  int num_total_processes;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "processes") {
        linestream >> num_total_processes;
      }
    }
  }

  return num_total_processes;
}

int LinuxParser::RunningProcesses() {
  string line;
  string key;
  int num_running_processes;
  std::ifstream filestream(kProcDirectory + kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "procs_running") {
        linestream >> num_running_processes;
      }
    }
  }

  return num_running_processes;
}

string LinuxParser::Command(int pid) {
  string command;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    std::getline(stream, command);
  }
  
  return command;
}

string LinuxParser::Ram(int pid) {
  long ram;
  string line, key;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "VmSize:") {
        linestream >> ram;
        ram *= kKbToMb;
      }
    }
  }

  return to_string(ram);
}

string LinuxParser::Uid(int pid) {
  string uid;
  string line, key;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> key;
      if (key == "Uid:") {
        linestream >> uid;
      }
    }
  }

  return uid;
}

string LinuxParser::User(int pid) {
  string target_uid{Uid(pid)};
  string name, password, uid;
  string line;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      linestream >> name >> password >> uid;
      if (uid == target_uid) {
        return name;
      }
    }
  }

  return "";
}

long LinuxParser::UpTime(int pid) {
  long start_time;
  string line, token;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);

    // skip first 21 tokens because start_time is the 22nd
    for (int i = 0; i < 21; ++i) {
        linestream >> token;
    }

    linestream >> start_time;
    start_time /= sysconf(_SC_CLK_TCK);
  }

  return UpTime() - start_time;
}