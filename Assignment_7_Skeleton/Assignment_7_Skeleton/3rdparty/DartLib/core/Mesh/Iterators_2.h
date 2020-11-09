#ifndef _DARTLIB_DIM2_ITERATORS_H_
#define _DARTLIB_DIM2_ITERATORS_H_

#include <set>
// Use vector instead, unordered_set may change the order of the elements.
// #include <unordered_set>
#include <vector>

namespace DartLib
{
namespace Dim2
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
class VertexInDartIterator
{
  public:
    VertexInDartIterator(typename M::CVertex* pV)
    {
        typename M::CDart* pD0 = (typename M::CDart*) pV->dart();
        typename M::CDart* pD = pD0;

        do
        {
            m_darts.push_back(pD);

            if (pD->beta(1)->beta(2) != NULL)
                pD = (typename M::CDart*) pD->beta(1)->beta(2);
            else
                break;
        } while (pD != pD0);

        m_iter = m_darts.begin();
    };

    ~VertexInDartIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_darts.end(); };

    typename M::CDart* operator*() { return *m_iter; };

  protected:
    std::vector<typename M::CDart*> m_darts;
    typename std::vector<typename M::CDart*>::iterator m_iter;
};

template <typename M>
class VertexFaceIterator
{
  public:
    VertexFaceIterator(typename M::CVertex* pV)
    {
        typename M::CDart* pD0 = (typename M::CDart*) pV->dart();
        typename M::CDart* pD = pD0;

        do
        {
            m_faces.push_back((typename M::CFace*) pD->cell(2));

            if (pD->beta(1)->beta(2) != NULL)
                pD = (typename M::CDart*) pD->beta(1)->beta(2);
            else
                break;
        } while (pD != pD0);

        m_iter = m_faces.begin();
    };

    ~VertexFaceIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_faces.end(); };

    typename M::CFace* operator*() { return *m_iter; };

  protected:
    std::vector<typename M::CFace*> m_faces;
    typename std::vector<typename M::CFace*>::iterator m_iter;
};

template <typename M>
class VertexEdgeIterator
{
  public:
    VertexEdgeIterator(typename M::CVertex* pV)
    {
        typename M::CDart * pD0 = (typename M::CDart*) pV->dart();
        typename M::CDart * pD  = pD0;
        
        do
        {
            typename M::CEdge* pE = (typename M::CEdge*) pD->cell(1);
            m_edges.push_back(pE);
            
            if (pD->beta(1)->beta(2) != NULL)
            {
                pD = (typename M::CDart*) pD->beta(1)->beta(2);
            }
            else
            {
                pE = (typename M::CEdge*) pD->beta(1)->cell(1);
                m_edges.push_back(pE);
                break;
            }
        } while (pD != pD0);

        m_iter = m_edges.begin();
    };

    ~VertexEdgeIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_edges.end(); };

    typename M::CEdge* operator*() { return *m_iter; };

  protected:
    std::vector<typename M::CEdge*> m_edges;
    typename std::vector<typename M::CEdge*>::iterator m_iter;
};

template <typename M>
class VertexVertexIterator
{
  public:
    VertexVertexIterator(typename M::CVertex* pV)
    {
        typename M::CDart* pD = (typename M::CDart*) pV->dart();
        if (pD->boundary())
        {
            typename M::CDart* pDNext = pD;
            while (true)
            {
                if (beta(1, pDNext) != pD)
                    pDNext = beta(1, pDNext);
                else
                {
                    m_vertices.push_back(C0(pDNext));
                    break;
                }
            }
        }

        typename M::CDart* pD0 = beta(1, pD);
        pD = pD0;
        do
        {
            m_vertices.push_back(C0(pD));

            pD = beta(2, pD);
            if (pD == NULL) break;
            else            pD = beta(1, pD);
        } while (pD != pD0);

        m_iter = m_vertices.begin();
    };

    ~VertexVertexIterator(){};

    void operator++() { m_iter++; };

    void operator++(int) { m_iter++; };

    bool end() { return m_iter == m_vertices.end(); };

    typename M::CVertex* operator*() { return *m_iter; };

  protected:
    std::vector<typename M::CVertex*> m_vertices;
    typename std::vector<typename M::CVertex*>::iterator m_iter;
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

} // namespace Dim2
} // namespace DartLib

#endif // !_DARTLIB_DIM2_ITERATORS_H_
