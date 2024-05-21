#pragma once


#include <chrono>
#include <utility>
#include <string>
#include <iostream>

typedef std::chrono::high_resolution_clock::time_point timePoint;

#define duration(a) std::chrono::duration_cast<std::chrono::milliseconds>(a).count()
#define timeNow() std::chrono::high_resolution_clock::now()

class Timer {
private:
 timePoint _start;
 std::string _name;
public:
  Timer (std::string&& name) : _name(name) {_start = timeNow();}
  Timer();
  void log();
  virtual ~Timer ();
};

Timer::Timer(){
  _start = timeNow();
}
void Timer::log(){
  std::cout << _name << " " << duration(timeNow() - _start) << " ms\n";
}

Timer::~Timer(){
  log();
}
