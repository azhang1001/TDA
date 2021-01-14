#ifndef _MY_TMESH_H_
#define _MY_TMESH_H_

#include "Mesh/BaseMesh_3.h"
#include "Mesh/Dart.h"
#include "Mesh/Vertex.h"
#include "Mesh/Edge.h"
#include "Mesh/Face.h"
#include "Mesh/Volume.h"
#include "Mesh/Boundary.h"
#include "Mesh/Iterators_3.h"
#include "Geometry/Plane.h"

namespace DartLib
{

class CMyVertex;
class CMyEdge;
class CMyFace;
class CMyTet;
class CMyDart;

class CMyVertex : public TVertex<3>
{
  public:
    CMyVertex() : m_outside(false), m_idx(0), m_pair(NULL), 
                  m_generator(false), m_valence(0) { };

    bool    & outside()   { return m_outside; };
    int     & idx()       { return m_idx; };
    CMyEdge*& pair()      { return m_pair; };
    bool    & generator() { return m_generator; };
    int     & valence()   { return m_valence; };
  protected:
    bool     m_outside;
    int      m_idx;
    CMyEdge* m_pair;
    bool     m_generator;
    int      m_valence;
};

class CMyEdge : public TEdge<3>
{
  public:
    CMyEdge() : m_sharp(false), m_idx(0), m_pair(NULL), m_generator(false), m_green(false) {};

    bool& sharp()    { return m_sharp; };
	bool& green() { return m_green; };
    int & idx()      { return m_idx; };
    CMyFace*& pair() { return m_pair; };
    bool& generator(){ return m_generator; };
  protected:
    bool     m_sharp;
	bool     m_green;
    int      m_idx;
    CMyFace* m_pair;
    bool     m_generator;
};

class CMyFace : public TFace<3>
{
  public:
    CMyFace() : m_idx(0), m_pair(NULL), m_generator(false){};

    CPoint & normal()    { return m_normal; }
    int    & idx()       { return m_idx; };
    CMyTet*& pair()      { return m_pair; };
    bool   & generator() { return m_generator; };
  protected:
    /*!
     *  Face normal, defined based on the darts on the face
     */
    CPoint  m_normal;
    int     m_idx;
    CMyTet* m_pair;
    bool    m_generator;
};

class CMyTet : public TVolume<3>
{
  public:
    CMyTet() : m_outside(false) {};

    bool& outside() { return m_outside; };
  protected:
    bool m_outside;
};

class CMyDart : public TDart<3>
{
  public:
  protected:
};

template <typename V, typename E, typename F, typename T, typename Dart>
class TMyTMesh : public TBaseMesh_3<V, E, F, T, Dart>
{
  public:
    typedef V    CVertex;
    typedef E    CEdge;
    typedef F    CFace;
    typedef T    CVolume;
    typedef Dart CDart;        // We can not use D here, because we have a D function

    using CBoundary            = Dim3::CBoundary<TMyTMesh<V, E, F, T, Dart>>;

    using VertexIterator       = Dim3::VertexIterator<TMyTMesh<V, E, F, T, Dart>>;
    using EdgeIterator         = Dim3::EdgeIterator<TMyTMesh<V, E, F, T, Dart>>;
    using FaceIterator         = Dim3::FaceIterator<TMyTMesh<V, E, F, T, Dart>>;
    using TetIterator          = Dim3::VolumeIterator<TMyTMesh<V, E, F, T, Dart>>;
    using DartIterator         = Dim3::DartIterator<TMyTMesh<V, E, F, T, Dart>>;

    using VertexVolumeIterator = Dim3::VertexVolumeIterator<TMyTMesh<V, E, F, T, Dart>>;
    using VertexEdgeIterator   = Dim3::VertexEdgeIterator<TMyTMesh<V, E, F, T, Dart>>;
    using VertexVertexIterator = Dim3::VertexVertexIterator<TMyTMesh<V, E, F, T, Dart>>;

    using EdgeVolumeIterator   = Dim3::EdgeVolumeIterator<TMyTMesh<V, E, F, T, Dart>>;
    using EdgeFaceIterator     = Dim3::EdgeFaceIterator<TMyTMesh<V, E, F, T, Dart>>;

    using FaceVertexIterator   = Dim3::FaceVertexIterator<TMyTMesh<V, E, F, T, Dart>>;
    using FaceEdgeIterator     = Dim3::FaceEdgeIterator<TMyTMesh<V, E, F, T, Dart>>;

    using TetVertexIterator    = Dim3::VolumeVertexIterator<TMyTMesh<V, E, F, T, Dart>>;

    void normalize();
    
    void compute_face_normal();
    
    void cut(const CPlane & plane);

    std::vector<CDart*>& halffaces_above() { return m_pHalffaces_above; };
    std::vector<CDart*>& halffaces_below() { return m_pHalffaces_below; };

    void write_boundary(const std::string & output);
  protected:
    std::vector<CDart*> m_pHalffaces_above;
    std::vector<CDart*> m_pHalffaces_below;
};

using CMyTMesh = TMyTMesh<CMyVertex, CMyEdge, CMyFace, CMyTet, CMyDart>;

template <typename V, typename E, typename F, typename T, typename Dart>
void TMyTMesh<V, E, F, T, Dart>::normalize()
{
    CPoint vmax(-1e+10, -1e+10, -1e+10);
    CPoint vmin(1e+10, 1e+10, 1e+10);

    for (int i = 0; i < this->m_vertices.size(); i++)
    {
        const CPoint & p = this->m_vertices[i]->point();

        for (int k = 0; k < 3; k++)
        {
            vmax[k] = (vmax[k] > p[k]) ? vmax[k] : p[k];
            vmin[k] = (vmin[k] < p[k]) ? vmin[k] : p[k];
        }
    }

    CPoint center = (vmax + vmin) / 2.0;

    double d = 0;
    for (int i = 0; i < this->m_vertices.size(); i++)
    {
        CPoint & p = this->m_vertices[i]->point();
        p = p - center;

        for (int k = 0; k < 3; k++)
        {
            if (fabs(p[k]) > d)
                d = fabs(p[k]);
        }
    }

    for (int i = 0; i < this->m_vertices.size(); i++)
    {
        CPoint & p = this->m_vertices[i]->point();
        p /= d;
    }
}

template <typename V, typename E, typename F, typename T, typename Dart>
void TMyTMesh<V, E, F, T, Dart>::compute_face_normal()
{
    CPoint pts[3];
    int i = 0;
    for (FaceIterator fiter(this); !fiter.end(); ++fiter)
    {
        F* pF = *fiter;
        i = 0;
        for (FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            V* pV = *fviter;
            pts[i++] = pV->point();
            if (i >= 3) break;
        }
        CPoint n = (pts[1] - pts[0]) ^ (pts[2] - pts[0]);
        n /= n.norm();

        pF->normal() = n;
    }
}

template <typename V, typename E, typename F, typename T, typename Dart>
void TMyTMesh<V, E, F, T, Dart>::cut(const CPlane& plane)
{
    m_pHalffaces_above.clear();
    m_pHalffaces_below.clear();

    for (VertexIterator viter(this); !viter.end(); ++viter)
    {
        V* pV = *viter;
        const CPoint& pt = pV->point();
        pV->outside() = (plane.side(pt) >= 0);
    }

    for (TetIterator titer(this); !titer.end(); ++titer)
    {
        T* pT = *titer;
        pT->outside() = true;
        for (TetVertexIterator tviter(pT); !tviter.end(); ++tviter)
        {
            V* pV = *tviter;
            if (!pV->outside())
            {
                pT->outside() = false;
                break;
            }
        }
    }

    for (FaceIterator fiter(this); !fiter.end(); ++fiter)
    {
        F* pF = *fiter;
        for (int i = 0; i < 2; ++i)
        {
            CDart* pD = (i == 0) ? D(pF) : beta(3, D(pF));
            if (pD == NULL) continue;

            T* pT[2] = {C3(pD), C3(beta(3, pD))};
            
            if (pT[1] == NULL)
            {
                if (pT[0]->outside())
                    m_pHalffaces_above.push_back(pD);
                else
                    m_pHalffaces_below.push_back(pD);

                continue;
            }

            if (pT[0]->outside() && !pT[1]->outside())
            {
                m_pHalffaces_above.push_back(pD);
            }
            else if (!pT[0]->outside() && pT[1]->outside())
            {
                pD = beta(3, pD);
                m_pHalffaces_below.push_back(pD);
            }
        }
    }
}

template <typename V, typename E, typename F, typename T, typename Dart>
void TMyTMesh<V, E, F, T, Dart>::write_boundary(const std::string& output)
{
    std::fstream os(output, std::fstream::out);
    
    int vid = 1;
    for (VertexIterator viter(this); !viter.end(); ++viter)
    {
        V* pV = *viter;
        pV->idx() = vid++;
        CPoint& p = pV->point();
        os << "Vertex " << pV->idx();
        os << " " << p[0] << " " << p[1] << " " << p[2] << "\n"; 
    }

    int fid = 1;
    for (FaceIterator fiter(this); !fiter.end(); ++fiter)
    {
        F* pF = *fiter;
        pF->idx() = fid++;
        if (this->boundary(pF))
        {
            os << "Face " << pF->idx();
            for (FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
            {
                V* pV = *fviter;
                os << " " << pV->idx();
            }
            os << "\n";
        }
    }

    os.close();
}

} // namespace DartLib
#endif // !_MY_TMESH_H_
