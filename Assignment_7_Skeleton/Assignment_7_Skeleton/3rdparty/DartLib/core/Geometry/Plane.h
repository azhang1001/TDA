#ifndef  _PLANE_H_
#define  _PLANE_H_

#include "Point.h"

namespace DartLib
{

class CPlane
{
public:

  CPlane( CPoint normal, double d ){ m_vNormal = normal; m_d = d;  };
  CPlane(){ m_d = 0; };
  ~CPlane(){};
  
  CPoint & normal()     { return m_vNormal; };
  double      & d()     { return m_d; };

  double side( const CPoint & p ) const
  { 
      double f =  p * m_vNormal - m_d;
      return f;
  };

protected:
  CPoint    m_vNormal;
  double    m_d;
};

};
#endif