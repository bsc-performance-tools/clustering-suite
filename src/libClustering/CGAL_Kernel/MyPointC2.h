#ifndef MY_POINTC2_H
#define MY_POINTC2_H


#include <CGAL/Origin.h>
#include <CGAL/Bbox_2.h>

/*
#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>
typedef  CGAL::Quotient<CGAL::MP_Float> ET; */

/* Try using this data type in case it fails */
#include <CGAL/Gmpq.h>
typedef CGAL::Lazy_exact_nt<CGAL::Gmpq> ET;


class MyPointC2 {

private:
  ET     vec[2];
  long long _Instance;
  long long _NeighbourhoodSize;


public:

  MyPointC2()
    : _Instance(0), _NeighbourhoodSize(0)
  {
    *vec = 0;
    *(vec+1) = 0;
  }


  MyPointC2(const ET x, const ET y, const long long int Instance = 0, const long long int NeighbourhoodSize = 0)
    : _Instance(Instance), _NeighbourhoodSize(NeighbourhoodSize)
  {
    *vec = x;
    *(vec+1) = y;
  }

  const ET& x() const  { return *vec; }

  const ET& y() const { return *(vec+1); }

  ET& x() { return *vec; }

  ET& y() { return *(vec+1); }

  long long Instance() const { return _Instance; }

  long long& Instance() { return _Instance; }

  long long NeighbourhoodSize() const { return _NeighbourhoodSize; }

  long long& NeighbourhoodSize() { return _NeighbourhoodSize; }


  bool operator==(const MyPointC2 &p) const
  {
    return ( *vec == *(p.vec) )  && ( *(vec+1) == *(p.vec + 1) && (( _Instance == p._Instance) &&  (_NeighbourhoodSize == p._NeighbourhoodSize)));
  }

  bool operator!=(const MyPointC2 &p) const
  {
      return !(*this == p);
  }

};

#endif // MY_POINTC2_H
