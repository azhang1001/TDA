#ifndef _DARTLIB_DIM3_ITERATORS_H_
#define _DARTLIB_DIM3_ITERATORS_H_

#include <set>
// Use vector instead, unordered_set may change the order of the elements.
// #include <unordered_set>
#include <vector>

namespace DartLib
{
namespace Dim3
{

template <typename M>
class VertexIterator
{
  public:
    VertexIterator(M* pMesh)
    {
        m_pMesh = pMesh;
        m_iter = m_pMesh->vertices().begin();
    }

    typename M::CVertex* value() { return *m_iter; };

    typename M::CVertex* operator*() { return value(); };

    void operator++() { ++m_iter; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_pMesh->vertices().end(); }

  private:
    M* m_pMesh;
    typename std::vector<typename M::CVertex*>::iterator m_iter;
};

template <typename M>
class EdgeIterator
{
  public:
    EdgeIterator(M* pMesh)
    {
        m_pMesh = pMesh;
        m_iter = m_pMesh->edges().begin();
    }

    typename M::CEdge* value() { return *m_iter; };

    typename M::CEdge* operator*() { return value(); };

    void operator++() { ++m_iter; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_pMesh->edges().end(); }

  private:
    M* m_pMesh;
    typename std::vector<typename M::CEdge*>::iterator m_iter;
};

template <typename M>
class FaceIterator
{
  public:
    FaceIterator(M* pMesh)
    {
        m_pMesh = pMesh;
        m_iter = m_pMesh->faces().begin();
    }

    typename M::CFace* value() { return *m_iter; };

    typename M::CFace* operator*() { return value(); };

    void operator++() { ++m_iter; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_pMesh->faces().end(); }

  private:
    M* m_pMesh;
    typename std::vector<typename M::CFace*>::iterator m_iter;
};

template <typename M>
class VolumeIterator
{
  public:
    VolumeIterator(M* pMesh)
    {
        m_pMesh = pMesh;
        m_iter = m_pMesh->volumes().begin();
    }

    typename M::CVolume* value() { return *m_iter; };

    typename M::CVolume* operator*() { return value(); };

    void operator++() { ++m_iter; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_pMesh->volumes().end(); }

  private:
    M* m_pMesh;
    typename std::vector<typename M::CVolume*>::iterator m_iter;
};

template <typename M>
class DartIterator
{
  public:
    DartIterator(M* pMesh)
    {
        m_pMesh = pMesh;
        m_iter = m_pMesh->darts().begin();
    }

    typename M::CDart* value() { return *m_iter; };

    typename M::CDart* operator*() { return value(); };

    void operator++() { ++m_iter; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_pMesh->darts().end(); }

  private:
    M* m_pMesh;
    typename std::vector<typename M::CDart*>::iterator m_iter;
};

template <typename M>
class VertexVolumeIterator
{
  public:
    VertexVolumeIterator(M* pMesh, typename M::CVertex* pV)
    {
        m_pMesh = pMesh;

        std::vector<typename M::CDart*> darts = m_pMesh->vertex_incident_darts(pV);

        for (typename M::CDart* d : darts)
            m_volumes.insert(m_pMesh->C3(d));

        m_iter = m_volumes.begin();
    };

    ~VertexVolumeIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_volumes.end(); };

    typename M::CVolume* operator*() { return *m_iter; };

  protected:
    M* m_pMesh;
    std::set<typename M::CVolume*> m_volumes;
    typename std::set<typename M::CVolume*>::iterator m_iter;
};

template <typename M>
class VertexEdgeIterator
{
  public:
    VertexEdgeIterator(M* pMesh, typename M::CVertex* pV)
    {
        m_pMesh = pMesh;

        std::vector<typename M::CDart*> darts = m_pMesh->vertex_incident_darts(pV);

        for (typename M::CDart* d : darts)
            m_edges.insert((typename M::CEdge*) d->cell(1));

        m_iter = m_edges.begin();
    };

    ~VertexEdgeIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_edges.end(); };

    typename M::CEdge* operator*() { return *m_iter; };

  protected:
    M* m_pMesh;
    std::set<typename M::CEdge*> m_edges;
    typename std::set<typename M::CEdge*>::iterator m_iter;
};

template <typename M>
class VertexVertexIterator
{
  public:
    VertexVertexIterator(M* pMesh, typename M::CVertex* pV)
    {
        m_pMesh = pMesh;

        std::vector<typename M::CDart*> darts = m_pMesh->vertex_incident_darts(pV);

        for (typename M::CDart* d : darts)
            m_vertices.insert(m_pMesh->C0(m_pMesh->beta(2, d)));

        m_iter = m_vertices.begin();
    };

    ~VertexVertexIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_vertices.end(); };

    typename M::CVertex* operator*() { return *m_iter; };

  protected:
    M* m_pMesh;
    std::set<typename M::CVertex*> m_vertices;
    typename std::set<typename M::CVertex*>::iterator m_iter;
};

template <typename M>
class EdgeFaceIterator
{
  public:
    EdgeFaceIterator(typename M::CEdge* pE)
    {
        // If pE lies on boundary, then the dart on the edge
        // must lies on boundary too.
        typename M::CDart* pD0 = (typename M::CDart*) pE->dart();
        typename M::CDart* pD = pD0;
        m_faces.push_back((typename M::CFace*) pD->face());

        do
        {
            pD = (typename M::CDart*) pD->beta(2);
            if (pD->beta(3) != NULL)
                pD = (typename M::CDart*) pD->beta(3);

            m_faces.push_back((typename M::CFace*) pD->face());
        } while (!pD->boundary() && pD != pD0);

        m_iter = m_faces.begin();
    };

    ~EdgeFaceIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_faces.end(); };

    typename M::CFace* operator*() { return *m_iter; };

  protected:
    std::vector<typename M::CFace*> m_faces;
    typename std::vector<typename M::CFace*>::iterator m_iter;
};

template <typename M>
class EdgeVolumeIterator
{
  public:
    EdgeVolumeIterator(typename M::CEdge* pE)
    {
        // If pE lies on boundary, then the dart on the edge
        // must lies on boundary too.
        typename M::CDart* pD0 = (typename M::CDart*) pE->dart();
        typename M::CDart* pD = pD0;

        do
        {
            m_volumes.push_back((typename M::CVolume*) pD->volume());
            pD = (typename M::CDart*) pD->beta(2)->beta(3);
        } while (pD != NULL && pD != pD0);

        m_iter = m_volumes.begin();
    };

    ~EdgeVolumeIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_volumes.end(); };

    typename M::CVolume* operator*() { return *m_iter; };

  protected:
    std::vector<typename M::CVolume*> m_volumes;
    typename std::vector<typename M::CVolume*>::iterator m_iter;
};

template <typename M>
class FaceVertexIterator
{
  public:
    FaceVertexIterator(typename M::CFace* pF)
    {
        m_pFace = pF;
        m_pDart = (typename M::CDart*) m_pFace->dart();
    };

    ~FaceVertexIterator(){};

    void operator++()
    {
        m_pDart = (typename M::CDart*) m_pDart->beta(1);

        if (m_pDart == (typename M::CDart*) m_pFace->dart())
        {
            m_pDart = NULL;
            return;
        }
    };

    void operator++(int)
    {
        m_pDart = (typename M::CDart*) m_pDart->beta(1);

        if (m_pDart == (typename M::CDart*) m_pFace->dart())
        {
            m_pDart = NULL;
            return;
        }
    };

    bool end() { return m_pDart == NULL; };

    typename M::CVertex* operator*() { return (typename M::CVertex*) m_pDart->cell(0); };

  protected:
    typename M::CFace* m_pFace;
    typename M::CDart* m_pDart;
};

template <typename M>
class FaceEdgeIterator
{
  public:
    FaceEdgeIterator(typename M::CFace* pF)
    {
        m_pFace = pF;
        m_pDart = (typename M::CDart*) m_pFace->dart();
    };

    ~FaceEdgeIterator(){};

    void operator++()
    {
        m_pDart = (typename M::CDart*) m_pDart->beta(1);

        if (m_pDart == (typename M::CDart*) m_pFace->dart())
        {
            m_pDart = NULL;
            return;
        }
    };

    void operator++(int)
    {
        m_pDart = (typename M::CDart*) m_pDart->beta(1);

        if (m_pDart == (typename M::CDart*) m_pFace->dart())
        {
            m_pDart = NULL;
            return;
        }
    };

    bool end() { return m_pDart == NULL; };

    typename M::CEdge* operator*() { return (typename M::CEdge*) m_pDart->cell(1); };

  protected:
    typename M::CFace* m_pFace;
    typename M::CDart* m_pDart;
};

template <typename M>
class VolumeVertexIterator
{
  public:
    VolumeVertexIterator(typename M::CVolume* pVol)
    {
        typename M::CDart* pD = (typename M::CDart*) pVol->dart();
        m_vertices.push_back((typename M::CVertex*) pD->cell(0));

        // TODO: The following implementation is effcient but only work on tet mesh.
        int orbit[4] = {1, 1, 2, 1};
        int mask[4] = {1, 1, 0, 1};
        for (int i = 0; i < 4; ++i)
        {
            pD = (typename M::CDart*) pD->beta(orbit[i]);
            if (mask[i] == 1)
                m_vertices.push_back((typename M::CVertex*) pD->cell(0));
        }

        m_iter = m_vertices.begin();
    };

    ~VolumeVertexIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_vertices.end(); };

    typename M::CVertex* operator*() { return *m_iter; };

  protected:
    std::vector<typename M::CVertex*> m_vertices;
    typename std::vector<typename M::CVertex*>::iterator m_iter;
};
} // namespace Dim3
} // namespace DartLib
#endif // !_DARTLIB_DIM3_ITERATORS_H_
