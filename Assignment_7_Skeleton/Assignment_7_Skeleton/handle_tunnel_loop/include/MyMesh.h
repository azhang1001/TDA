#ifndef _MY_MESH_H_
#define _MY_MESH_H_

#include "Mesh/BaseMesh_2.h"
#include "Mesh/Dart.h"
#include "Mesh/Edge.h"
#include "Mesh/Face.h"
#include "Mesh/Vertex.h"

//#include "Mesh/Boundary.h"
#include "Mesh/Iterators_2.h"

namespace DartLib
{

class CMyVertex_2;
class CMyEdge_2;
class CMyFace_2;
class CMyDart_2;

class CMyVertex_2 : public TVertex<2>
{
  public:
    CMyVertex_2() : m_idx(0){};

    int& idx() { return m_idx; };
    CPoint& normal() { return m_normal; };
  protected:
    int m_idx;
    CPoint m_normal;
};

class CMyEdge_2 : public TEdge<2>
{
  public:
    CMyEdge_2() : m_sharp(false){};

    bool& sharp() { return m_sharp; };

  protected:
    bool m_sharp;
};

class CMyFace_2 : public TFace<2>
{
  public:
    CMyFace_2() : m_idx(0){};

    CPoint& normal() { return m_normal; }
    int& idx() { return m_idx; };

  protected:
    /*!
     *  Face normal, defined based on the darts on the face
     */
    CPoint m_normal;
    int m_idx;
};

class CMyDart_2 : public TDart<2>
{
  public:
  protected:
};

template <typename V, typename E, typename F, typename Dart>
class TMyMesh : public TBaseMesh_2<V, E, F, Dart>
{
  public:
    typedef V    CVertex;
    typedef E    CEdge;
    typedef F    CFace;
    typedef Dart CDart; // We can not use D here, because we have a D function
    
    using VertexIterator       = Dim2::VertexIterator<TMyMesh<V, E, F,Dart>>;
    using EdgeIterator         = Dim2::EdgeIterator<TMyMesh<V, E, F,Dart>>;
    using FaceIterator         = Dim2::FaceIterator<TMyMesh<V, E, F,Dart>>;
    using DartIterator         = Dim2::DartIterator<TMyMesh<V, E, F,Dart>>;

    using VertexInDartIterator = Dim2::VertexInDartIterator<TMyMesh<V, E, F,Dart>>;
    using VertexFaceIterator   = Dim2::VertexFaceIterator<TMyMesh<V, E, F,Dart>>;
    using VertexEdgeIterator   = Dim2::VertexEdgeIterator<TMyMesh<V, E, F,Dart>>;
    using VertexVertexIterator = Dim2::VertexVertexIterator<TMyMesh<V, E, F,Dart>>;

    using FaceVertexIterator   = Dim2::FaceVertexIterator<TMyMesh<V, E, F,Dart>>;
    using FaceEdgeIterator     = Dim2::FaceEdgeIterator<TMyMesh<V, E, F,Dart>>;
    

};

using CMyMesh = TMyMesh<CMyVertex_2, CMyEdge_2, CMyFace_2, CMyDart_2>;

} // namespace DartLib
#endif // !_MY_TMESH_H_
