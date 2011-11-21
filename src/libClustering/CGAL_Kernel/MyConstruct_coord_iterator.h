#ifndef MYCONSTRUCT_COORD_ITERATOR_H
#define MYCONSTRUCT_COORD_ITERATOR_H

class MyConstruct_coord_iterator {
public:
  const float* operator()(const MyPointC2& p)
  {
    return &p.x();
  }

  const float* operator()(const MyPointC2& p, int)
  {
    const float* pyptr = &p.y();
    pyptr++;
    return pyptr;
  }
};

#endif //MYCONSTRUCT_COORD_ITERATOR_H
