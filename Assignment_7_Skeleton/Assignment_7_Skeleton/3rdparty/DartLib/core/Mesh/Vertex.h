#ifndef _DARTLIB_VERTEX_H_
#define _DARTLIB_VERTEX_H_

#include "../Geometry/Point.h"
#include "Cell.h"

namespace DartLib
{

template<int N>
class TVertex : public TCell< N >
{
  public:
    TVertex() {};

    CPoint& point() { return m_point; };
  protected:
    
    // geometric coordinate
    CPoint m_point;
};


}
#endif // !_DARTLIB_VERTEX_H_
