#ifndef _DARTLIB_DART_H_
#define _DARTLIB_DART_H_

#include <stdio.h>
#include <assert.h>

namespace DartLib
{
template <int N>
class TCell;

template <int N>
class TDart
{
  public:
    
    TDart()
    {
        for (int i = 0; i < N; ++i)
            m_beta[i] = NULL;

        for (int i = 0; i < N + 1; ++i)
            m_cells[i] = NULL;
    };

    /*!
     *  Beta function in combinatorial map.
     *  \param i: index i should be 1, 2 or 3
     *  \return the reference of the dart pointer
     *
     *  NOTICE:
     *  The value of beta_1 function could be the next edge or the
     *  previous edge on the same face, but here we assume it always
     *  be the next counter-clockwise(ccw) edge.
     */
    TDart<N>*& beta(const int i)
    {
        assert(i >= 1 && i <= N);
        return m_beta[i - 1];
        //printf("[ERROR] The beta function here can only accept parameters 1, 2, ..., N!\n");
        //exit(EXIT_FAILURE);
    };

    TCell<N>*& cell(int i)
    {
        assert(i >= 0 && i <= N);
        return m_cells[i];
    };

    /*
     *  Whether the dart lies on the boundary.
     *  \return true or false
     */
    bool boundary() { return this->beta(N) == NULL; };

  protected:
    /*!
     *  Store the value(dart pointer) of beta_1, beta_2, ..., beta_n
     *  function.
     *
     *  NOTICE:
     *  In the combinatorial map, beta0(dart) = dart,
     *  so we do not need to store this function value.
     */
    TDart<N>* m_beta[N];

    TCell<N>* m_cells[N + 1];
};

} // namespace DartLib
#endif // !_DARTLIB_DART_H_
