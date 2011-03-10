/*
  Simple timer class
  ==================
  History:
 
  Created - Sarah "Voodoo Doll" White (2006/01/26)
  ==================
  Description:

  A quick and easy timer class to save people
  from using ad hoc timers. The constructor
  begins the timer and the end() member function
  stops it, as well as returns the result.
  Before starting the timer again with begin(),
  users can see the last result with last()...

  So unintuitive. ;)
  ==================
  Bugs:

  The timing method used is neither strictly
  portable, though it only uses standard
  functions, nor is it guaranteed to support
  timing resolution less than a second.
*/
#include <ctime>

class Timer {

  public:
    typedef double diff_type;

    // Same as Timer t; t.begin();
    Timer(): start(std::clock()), elapsed(0) {}
    // Last result before a call to begin()
    diff_type last() const { return elapsed; }
    // Reset the timer
    void begin() { start = std::clock(); elapsed = 0; }
    // Save the result
    diff_type end();

  private:
    std::clock_t start;
    diff_type    elapsed;
};

Timer::diff_type Timer::end()
{
  elapsed = (diff_type) std::clock() - start;
  elapsed /= CLOCKS_PER_SEC;
  return elapsed;
}
