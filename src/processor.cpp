#include "processor.h"
#include "linux_parser.h"
#include <iostream>

// TODO: Return the aggregate CPU utilization
float Processor::Utilization() {
  long total, total_old, idle, idle_old;

  total = LinuxParser::Jiffies();
  idle = LinuxParser::IdleJiffies();

  // Keep values
  total_old = up_total;
  idle_old = up_idle;

  UpdateValues(total, idle);

  float total_delta = float(total) - float(total_old);
  float idle_delta = float(idle) - float(idle_old);

  return (total_delta - idle_delta) / total_delta;
}

void Processor::UpdateValues(long total, long idle) {
  up_idle = idle;
  up_total = total;
}
