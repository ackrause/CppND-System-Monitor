#include "processor.h"
#include "linux_parser.h"

float Processor::Utilization() {
    long active_jiffies{LinuxParser::ActiveJiffies()};
    long total_jiffies{LinuxParser::Jiffies()};
    long delta_active{active_jiffies - prev_active_};
    long delta_time{total_jiffies - prev_total_};

    prev_active_ = active_jiffies;
    prev_total_ = total_jiffies;

    return static_cast<float>(delta_active) / delta_time;
}