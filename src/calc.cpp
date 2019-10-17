#include "calc.hpp"
#include <cmath>

namespace chfs {
namespace calc {
int ceil(int dividend, int divisor) {
  return (dividend + divisor - 1) / divisor;
}
int floor(int dividend, int divisor) { return dividend / divisor; }
int ceil(double dividend, double divisor) {
  return std::ceil(dividend / divisor);
}
int floor(double dividend, double divisor) {
  return std::floor(dividend / divisor);
}
int round(double dividend, double divisor) {
  return std::round(dividend / divisor);
}
double quotient(double dividend, double divisor) {
  return floor(dividend, divisor);
}
double remainder(double dividend, double divisor) {
  return dividend - quotient(dividend, divisor) * divisor;
}
}  // namespace calc
}  // namespace chfs
