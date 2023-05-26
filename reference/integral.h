#include "parallel.h"

using namespace parlay;

template <class Func>

double integral(const Func& f, size_t n, double low, double high) {
  // double dx = (high - low) / n;
  if(n<1024) {
    double dx = (high - low)/n;
    double ans = 0;
    while(n--) {
      ans += f(low + n*dx);
    }
    return dx*ans;
  }
  // parallel_for(0, n, [&] (size_t i) {
  //   ans += f(low + dx * i);
  // });

  double L,R,bound = low+(high-low)/2.0;
  auto f1 = [&]() {L = integral(f,n/2, low, bound);};
  auto f2 = [&]() {R = integral(f,n-n/2, bound, high);};
  par_do(f1,f2);
  return L+R;
}