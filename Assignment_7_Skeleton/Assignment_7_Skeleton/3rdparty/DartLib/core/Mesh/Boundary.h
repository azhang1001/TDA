#ifndef _DARTLIB_BOUNDARY_H_
#define _DARTLIB_BOUNDARY_H_

#include <string>
#include <set>
#include <vector>
#include <fstream>

#include "Iterators_3.h"
#include "../Geometry/Point.h"

namespace DartLib
{
namespace Dim3
{

template <class M>
class CBoundary
{
  public:
    CBoundary(M* pMesh);

    std::vector<typename M::CFace*>& boundary_surface() { return m_boundary_surface; };

    void write_m(const std::string& output);

  protected:
    M* m_pMesh;

    std::vector<typename M::CFace*> m_boundary_surface;
};

template <class M>
CBoundary<M>::CBoundary(M* pMesh)
{
    m_pMesh = pMesh;

    for (FaceIterator<M> fiter(m_pMesh); !fiter.end(); ++fiter)
    {
        typename M::CFace* pF = *fiter;
        if (m_pMesh->boundary(pF))
            m_boundary_surface.push_back(pF);
    }
}

template <class M>
void CBoundary<M>::write_m(const std::string& output)
{
    std::fstream _os(output, std::fstream::out);

    if (_os.fail())
    {
        fprintf(stderr, "Error is opening file %s\n", output);
        return;
    }

    std::set<typename M::CVertex*> vSet;
    int vid = 1;
    for (auto pF : m_boundary_surface)
    {
        for (FaceVertexIterator<M> fviter(pF); !fviter.end(); ++fviter)
        {
            typename M::CVertex* pV = *fviter;
            pV->idx() = vid++;
            vSet.insert(pV);
        }
    }

    for (auto pV : vSet)
    {
        CPoint& p = pV->point();
        _os << "Vertex " << pV->idx();
        _os << " " << p[0] << " " << p[1] << " " << p[2] << "\n";
    }

    int fid = 1;
    for (auto pF : m_boundary_surface)
    {
        _os << "Face " << fid++;
        for (FaceVertexIterator<M> fviter(pF); !fviter.end(); ++fviter)
        {
            typename M::CVertex* pV = *fviter;
            _os << " " << pV->idx();
        }
        _os << "\n";
    }

    _os.close();
}
} // namespace Dim3
} // namespace DartLib
#endif // !_DARTLIB_BOUNDARY_H_
