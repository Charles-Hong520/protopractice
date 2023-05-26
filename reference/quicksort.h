#include <algorithm>
#include <cmath>
#include <utility>

#include "parallel.h"

using namespace parlay;
using std::cout;
using std::endl;
long long int *B;
bool *flag1,*flag2;
int *ps1,*ps2;
// template <class T>
// int reduce(T* In, int n) {
//   if(n<=100000) {
//     int sum = 0;
//     for(n--) sum+=In[n];
//     return sum;
//   }
//   int h = n/2;
//   auto r1 = [&]() { s1 = reduce(In, h); };
//   auto r2 = [&]() { s2 = reduce(In+h, n-h); };
//   return s1+s2;
// }

inline uint64_t random(uint64_t u) {
  uint64_t v = u * 3935559000370003845ul + 2691343689449507681ul;
  v ^= v >> 21;
  v ^= v << 37;
  v ^= v >> 4;
  v *= 4768777513237032717ul;
  v ^= v << 20;
  v ^= v >> 41;
  v ^= v << 5;
  return v;
}


void scan(bool *In, int l, int r, int* Out) {
  if(r-l < 1e6) {
    Out[l]=In[l];
    for(int i = l+1; i < r; i++) {
      Out[i] = In[i] + Out[i-1];
    }
    return;
  }
  int n = r-l;
  int m = std::sqrt(n); //chunks
  int sz = (n+m-1)/m; //sz of chunk, ceil
  while(m*sz<n) {m++;cout<<"m"<<m<<endl;}
  parallel_for(0, m, [&] (int i) {
    int s = l+i*sz;
    int e = std::min(s+sz,r)-1;
    Out[e] = 0;
    for(int j = s; j <= e; j++) {
      Out[e] += In[j];
    }

  });
  int slocal=l, elocal=l;
  for(int i = 1; i < m; i++) {
    slocal += sz;
    elocal = std::min(slocal+sz,r)-1;
    Out[elocal] += Out[slocal-1];
  }

  parallel_for(0, m, [&] (int i) {
    int s = l+i*sz;
    int e = std::min(s+sz,r)-1;
    for(int j = e; j-1 >= s; j--) {
      Out[j-1] = Out[j]-In[j];
    }
  });
}


template<class T>
std::pair<int,int> parallel_partition(T* A, int l, int r, long long pivot) {
//make flag
//filter flags
//move elem around
//return index of A[t]

  parallel_for(l, r, [&] (int i) {
    flag1[i] = (A[i]<pivot);
    flag2[i] = (A[i]>pivot);
  });
  scan(flag1, l, r, ps1);
  scan(flag2, l, r, ps2);
  int mid1 = l+ps1[r-1];
  int mid2 = r-ps2[r-1];
  parallel_for(l, r, [&] (int i) {
    if(flag1[i]) B[l+ps1[i]-1] = A[i];
    else if(flag2[i]) B[r-ps2[i]] = A[i];
    else B[ps1[r-1]+i-ps1[i]-ps2[i]] = pivot;
  });

  parallel_for(l, r, [&] (int i) {
    A[i]=B[i];
  });
  return {mid1,mid2};
}



template <class T>
void qsort(T *A, int l, int r) {
  // if(r-l<=1) return;
  if(r-l<5e5) {std::sort(A+l, A+r);}
  else {
      std::pair<int,int> part = parallel_partition(A, l, r, A[l+((l+r)%(r-l))]); //index
      par_do(
        [&]() {qsort(A, l, part.first);}, [&]() {qsort(A, part.second, r);});
    }
}

template <class T>
void quicksort(T *A, size_t n) {
  flag1 = (bool*)malloc(n * sizeof(bool));
  flag2 = (bool*)malloc(n * sizeof(bool));
  ps1 = (int*)malloc(n * sizeof(int));
  ps2 = (int*)malloc(n * sizeof(int));
  B = (long long int*)malloc(n * sizeof(long long int));

  qsort(A, 0, n);

  free(flag1);
  free(flag2);
  free(ps1);
  free(ps2);
  free(B);
}
