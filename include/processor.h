#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  double Utilization();
  void UpdateValues(long, long);

 private:
  long up_idle;
  long up_active;
  long up_total;
};
#endif