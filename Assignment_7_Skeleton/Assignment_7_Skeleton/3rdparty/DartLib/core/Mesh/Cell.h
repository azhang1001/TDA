#ifndef _DARTLIB_CELL_H_
#define _DARTLIB_CELL_H_

namespace DartLib
{
template <int N>
class TDart;

template <int N>
class TCell
{
  public:
    TCell() : m_pDart(NULL) {};

    TDart<N>*& dart() { return m_pDart; };
  protected:
    TDart<N>* m_pDart;
};
} // namespace DartLib
#endif //! _DARTLIB_CELL_H_
