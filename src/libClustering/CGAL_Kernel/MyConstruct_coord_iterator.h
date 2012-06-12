#ifndef MYCONSTRUCT_COORD_ITERATOR_H
#define MYCONSTRUCT_COORD_ITERATOR_H

/*
#include <CGAL/MP_Float.h>
#include <CGAL/Quotient.h>
typedef  CGAL::Lazy_exact_nt< CGAL::Quotient<CGAL::MP_Float> > ET; */

/* Try using this data type in case it fails  */
#include <CGAL/Gmpq.h>
typedef CGAL::Lazy_exact_nt<CGAL::Gmpq> ET; 

/* Try using this data type in case it fails 
#include <CGAL/Gmpq.h>
typedef CGAL::Gmpq ET; */

class MyConstruct_coord_iterator {
public:
  const ET* operator()(const MyPointC2& p)
  {
    return &p.x();
  }

  const ET* operator()(const MyPointC2& p, int)
  {
    const ET* pyptr = &p.y();
    pyptr++;
    return pyptr;
  }
};

#endif //MYCONSTRUCT_COORD_ITERATOR_H
