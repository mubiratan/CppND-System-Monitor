#include "processor.h"
#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
double Processor::Utilization() {
  long total, total_old, idle, idle_old;

  total = LinuxParser::Jiffies();
  idle = LinuxParser::IdleJiffies();

  // Keep values
  total_old = up_total;
  idle_old = up_idle;

  UpdateValues(total, idle);

  double total_delta = static_cast<double>(total) - static_cast<double>(total_old);
  double idle_delta = static_cast<double>(idle) - static_cast<double>(idle_old);

  return (total_delta - idle_delta) / total_delta;
}

void Processor::UpdateValues(long total, long idle) {
  up_idle = idle;
  up_total = total;
}
