#ifndef _DARTLIB_MESH_2D_H_
#define _DARTLIB_MESH_2D_H_

#include <time.h>
#include <unordered_map>
#include <queue>
#include <string>
#include <vector>
#include <fstream>

#include "../Geometry/Point.h"
#include "../Parser/strutil.h"
#include "Basic.h"

#define MAX_LINE 1024

#define T_TYPENAME template <typename CVertex, typename CEdge, typename CFace, typename CDart>
#define TBASEMESH TBaseMesh_2<CVertex, CEdge, CFace, CDart>

namespace DartLib
{
/*! \class TBaseMesh_2 BaseMesh_2.h "BaseMesh_2.h"
 *  \brief TBaseMesh_2, base class for all types of 2d-mesh classes
 *
 *  \tparam CVertex vertex class, derived from DartLib::CVertex class
 *  \tparam CEdge   edge   class, derived from DartLib::CEdge   class
 *  \tparam CFace   face   class, derived from DartLib::CFace   class
 *  \tparam CDart   dart   class, derived from DartLib::CDart   class
 */
T_TYPENAME
class TBaseMesh_2
{
  public:
    /*!
     *  Constructor function
     */
    TBaseMesh_2(){};

    /*!
     *  Destructor function
     */
    ~TBaseMesh_2() { unload(); };

    /*!
     *  Unload data
     */
    void unload();

    /*!
     *  Load mesh from the .m file
     *  \param input: the input name of the mesh file
     */
    void load_m(const std::string& input);

    /*!
     *  Load mesh from point cloud and faces
     *  \param vertices: the input point cloud
     *  \param indices: the indices of points to construct faces
     */
    void load(const std::vector<CPoint>& vertices, const std::vector<int>& indices);

    /*!
     *  Add a vertex cell.
     *  The vertex dart will be set in the processing of 
     *  creating 1-cell, so it returns a vertex pointer here.
     *  \return the added vertex pointer
     */
    CVertex* add_vertex();

    /*!
     *  Add a face cell
     *  \param indices: indices of the vertices to construct a face
     *  \return a dart pointer of the face
     */
    CDart* add_face(const std::vector<int>& indices);

    /*!
     *  Add a edge cell
     *  \param pF: face pointer the edge attaches to.
     *  \param indices: indices of the vertices to construct an edge
     *  \return a dart pointer of the edge
     */
    CDart* add_edge(CFace* pF, const std::vector<int>& indices);

    /*!
     *  Link the two faces
     *  \param f1: index of the first face
     *  \param f2: index of the second face     *
     */
    void link_faces(CDart* f1, CDart* f2);

    /*!
     *  Link the two edges
     *  \param e1: index of the first edge
     *  \param e2: index of the second edge
     *
     */
    void link_edges(CDart* e1, CDart* e2);

    /*!
     *  The beta function in combinatorial map
     *  \param i: represents for function beta_i, i \in {1, 2}
     *  \param dart: the input dart
     */
    CDart* beta(int i, CDart* dart) { return (CDart*) dart->beta(i); };


    /*=============================================================
                   Access dart and i-cell from index

        Recommend to use iterators instead of the following usage.
            for(int i = 0; i < darts().size(); ++i)
            {
                CDart * pD = dart(i);
                ...
            }
      =============================================================*/

    CDart*   dart  (int index) { return m_darts[index];    };
    CVertex* vertex(int index) { return m_vertices[index]; };
    CEdge*   edge  (int index) { return m_edges[index];    };
    CFace*   face  (int index) { return m_faces[index];    };


    /*=============================================================
                   Access container of dart and i-cell

        Recommend to use iterators instead of the container.
      =============================================================*/

    std::vector<CDart*  >& darts()    { return m_darts;    };
    std::vector<CVertex*>& vertices() { return m_vertices; };
    std::vector<CEdge*  >& edges()    { return m_edges;    };
    std::vector<CFace*  >& faces()    { return m_faces;    };

    
    /*=============================================================
                Access dart from i-cell and vice versa
    =============================================================*/
    
    CVertex*C0(CDart* dart)  { return dart != NULL ? (CVertex*)dart->cell(0) : NULL;};
    CEdge*  C1(CDart* dart)  { return dart != NULL ? (CEdge*)  dart->cell(1) : NULL;};
    CFace*  C2(CDart* dart)  { return dart != NULL ? (CFace*)  dart->cell(2) : NULL;};
    CDart*  D (CVertex* vert){ return vert != NULL ? (CDart*)  vert->dart()  : NULL;};
    CDart*  D (CEdge*   edge){ return edge != NULL ? (CDart*)  edge->dart()  : NULL;};
    CDart*  D (CFace*   face){ return face != NULL ? (CDart*)  face->dart()  : NULL;};
   

    /*=============================================================
                         Boundary detectors
    =============================================================*/

    bool boundary(CDart* dart) { return    dart->boundary(); };
    bool boundary(CEdge* edge) { return D(edge)->boundary(); };
    
    /*!
     *  Get all darts incident to the given vertex.
     *  \param vertex: current vertex
     *  \return all incident darts
     */
    std::vector<CDart*> vertex_incident_darts(CVertex* vertex);

    /*!
     *  Get the i-th vertex on the edge.
     *  \param edge: the input edge
     *  \param i: the index, i \in {0, 1}
     *  \return the i-th endpoint
     */
    CVertex* edge_vertex(CEdge* edge, int index);

    int nVertices() { return m_vertices.size(); };
    int nEdges()    { return m_edges.size();    };
    int nFaces()    { return m_faces.size();    };
    int nDarts()    { return m_darts.size();    };
  protected:
    void _post_processing();

  protected:
    std::vector<CDart*>   m_darts;
    std::vector<CVertex*> m_vertices;
    std::vector<CEdge*>   m_edges;
    std::vector<CFace*>   m_faces;

    std::unordered_map<EdgeMapKey, int, EdgeMapKey_hasher> m_map_edge_keys;
};

T_TYPENAME
void TBASEMESH::unload()
{
    for (auto v : m_darts)    delete v;
    m_darts.clear();

    for (auto v : m_vertices) delete v;
    m_vertices.clear();

    for (auto v : m_edges)    delete v;
    m_edges.clear();

    for (auto v : m_faces)    delete v;
    m_faces.clear();

    m_map_edge_keys.clear();
}

T_TYPENAME
void TBASEMESH::load_m(const std::string& input)
{
    std::vector<CPoint> vertices;
    std::vector<int> indices;

    // parse the .t file
    /*
    vertices.push_back(CPoint(0, 1, 0));
    vertices.push_back(CPoint(-1, 0, 0));
    vertices.push_back(CPoint(0, 0, 1));
    vertices.push_back(CPoint(1, 0, 0));
    vertices.push_back(CPoint(0, -1, 0));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(3);

    indices.push_back(4);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(3);
    */

    std::fstream is(input, std::fstream::in);
    if (is.fail())
    {
        fprintf(stderr, "Error in opening file %s\n", input);
        return;
    }

    char buffer[MAX_LINE];

    int nVertices = 0;
    int nFaces = 0;
    int nEdges = 0;
    
    while (!is.eof())
    {
        is.getline(buffer, MAX_LINE);
        std::string line(buffer);
        line = strutil::trim(line);
        strutil::Tokenizer stokenizer(line, " \r\n");

        stokenizer.nextToken();
        std::string token = stokenizer.getToken();

        if (token == "Vertex")
            nVertices++;
        if (token == "Face")
            nFaces++;
        if (token == "Edge")
            nEdges++;
    }

    is.clear();                 // forget we hit the end of file
    is.seekg(0, std::ios::beg); // move to the start of the file

    // read vertices
    for (int i = 0; i < nVertices && is.getline(buffer, MAX_LINE); i++)
    {
        std::string line(buffer);
        line = strutil::trim(line);
        strutil::Tokenizer stokenizer(line, " \r\n");

        stokenizer.nextToken();
        std::string token = stokenizer.getToken();

        if (token != "Vertex")
        {
            i--;
            fprintf(stderr, "Warning: File Format Error\r\n");
            continue;
        }

        stokenizer.nextToken();
        token = stokenizer.getToken();
        int vid = std::atoi(token.c_str()); // strutil::parseString<int>(token);

        CPoint p;
        for (int k = 0; k < 3; k++)
        {
            stokenizer.nextToken();
            std::string token = stokenizer.getToken();
            p[k] = std::atof(token.c_str()); // strutil::parseString<float>(token);
        }
        vertices.push_back(p);

        if (!stokenizer.nextToken("\t\r\n"))
            continue;
        token = stokenizer.getToken();

        int sp = (int) token.find("{");
        int ep = (int) token.find("}");

        if (sp >= 0 && ep >= 0)
        {
            //v->string() = token.substr(sp + 1, ep - sp - 1);
        }
    }

    // read faces
    for (int i = 0; i < nFaces && is.getline(buffer, MAX_LINE); i++)
    {
        std::string line(buffer);
        line = strutil::trim(line);
        strutil::Tokenizer stokenizer(line, " \r\n");

        stokenizer.nextToken();
        std::string token = stokenizer.getToken();

        if (token != "Face")
        {
            i--;
            fprintf(stderr, "Warning: File Format Error.\r\n");
            continue;
        }

        // skip the first "3" in the line
        stokenizer.nextToken();
        token = stokenizer.getToken();
        int tid = std::atoi(token.c_str()); // strutil::parseString<int>(token);

        for (int k = 0; k < 3; k++)
        {
            stokenizer.nextToken();
            std::string token = stokenizer.getToken();
            int vid = std::atoi(token.c_str()); // strutil::parseString<int>(token);
            indices.push_back( vid - 1 );   // assume vid is from one to #V
        }

        // read in string
        if (!stokenizer.nextToken("\t\r\n"))
            continue;
        token = stokenizer.getToken();

        int sp = (int) token.find("{");
        int ep = (int) token.find("}");

        if (sp >= 0 && ep >= 0)
        {
            //pT->string() = token.substr(sp + 1, ep - sp - 1);
        }
    }

    is.close();

    load(vertices, indices);
}

T_TYPENAME
void TBASEMESH::load(const std::vector<CPoint>& points, const std::vector<int>& indices)
{
    // clear mesh
    this->unload();

    // add vertices
    m_vertices.resize(points.size());
    for (int i = 0; i < points.size(); ++i)
    {
        CVertex* pV = add_vertex();
        m_vertices[i] = pV;
        m_vertices[i]->point() = points[i];
    }

    // add faces
    int nTets = indices.size() / 3;
    std::vector<int> face(3);
    for (int i = 0; i < nTets; ++i)
    {
        for (int j = 0; j < 3; ++j)
            face[j] = indices[i * 3 + j];

        add_face(face);
    }

    // post processing
    _post_processing();
}

T_TYPENAME
CVertex* TBASEMESH::add_vertex() 
{
    return new CVertex;
}

T_TYPENAME
CDart* TBASEMESH::add_face(const std::vector<int>& indices)
{
    // add face
    CFace* pF = new CFace;
    m_faces.push_back(pF);

    // add edges attached on the face ccwly
    static int tbl_face_edge[3][2] = {{0, 1}, {1, 2}, {2, 0}};
    std::vector<int> edge_verts(2);
    std::vector<CDart*> edges; // darts on the edges
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 2; ++j)
            edge_verts[j] = indices[tbl_face_edge[i][j]];
        CDart* e = add_edge(pF, edge_verts);
        edges.push_back(e);
    }

    // set dart for new face
    pF->dart() = edges[0];

    // link the edges
    for (int i = 0; i < edges.size(); ++i)
    {
        CDart* e1 = edges[i];
        CDart* e2 = edges[(i + 1) % edges.size()];
        link_edges(e1, e2);
    }

    // reutrn the new added dart even if there is no new added face
    return D(pF);
}

T_TYPENAME
CDart* TBASEMESH::add_edge(CFace* pF, const std::vector<int>& indices)
{
    // add edge
    auto result = m_map_edge_keys.find(&indices[0]);
    bool found = result != m_map_edge_keys.end();
    
    CEdge* pE = NULL;
    if (!found) // not found
    {
        pE = new CEdge;
        m_edges.push_back(pE);

        m_map_edge_keys.insert(std::make_pair(&indices[0], m_edges.size() - 1));
    }
    else
        pE = m_edges[result->second];

    // add new dart
    CDart* pD = new CDart;
    pD->cell(0) = m_vertices[indices[1]];
    pD->cell(1) = pE;
    pD->cell(2) = pF;
    m_darts.push_back(pD);

    // assign a dart to the target vertex in the situation of ccwly
    if (D(m_vertices[indices[1]]) == NULL) // not assigned yet
        m_vertices[indices[1]]->dart() = pD;

    // set dart for the new edge
    if (!found)
        pE->dart() = pD;
    else
        link_faces(D(pE), pD);
    // in the situation of combinatorial map
    // we do not need to link_vertices (set beta0)

    return pD;
}

T_TYPENAME
void TBASEMESH::link_faces(CDart* d1, CDart* d2)
{
    for (int i = 0; i < 3; ++i) // #edges of f1 = 3
    {
        for (int j = 0; j < 3; ++j) // #edges of f2 = 3
        {
            if (d1->cell(1) == d2->cell(1))
            {
                d1->beta(2) = d2;
                d2->beta(2) = d1;

                // We assume all faces are convex, so only one edge is shared
                // by f1 and f2. But it is not ture if there exists concave faces.
                return;
            }
            d2 = beta(1, d2); // beta1 has been set in link_edges
        }
        d1 = beta(1, d1);
    }
    printf("[ERROR] Should found!\n");
    exit(EXIT_FAILURE);
}

T_TYPENAME
void TBASEMESH::link_edges(CDart* d1, CDart* d2) { d1->beta(1) = d2; }

T_TYPENAME
CVertex* TBASEMESH::edge_vertex(CEdge* edge, int index)
{
    assert(index == 0 || index == 1);

    if (index == 0)
        return C0(D(edge));
    else if (index == 1)
    {
        CDart* pD = D(edge);
        if (boundary(pD))
        {
            do
            {
                if (C1(beta(1, pD)) == edge) return C0(pD);
                else                         pD = beta(1, pD);
            } while (true);
        }
        else
            return C0(beta(2, pD));
    }
}

T_TYPENAME
std::vector<CDart*> TBASEMESH::vertex_incident_darts(CVertex* vertex)
{
    std::vector<CDart*> darts;


    return darts;
}

T_TYPENAME
void TBASEMESH::_post_processing()
{
    // 1. set boundary darts on boundary vertices
    CVertex* pV;
    for (CDart* pD : m_darts)
    {
        if (pD->boundary())
        {
            pV = C0(pD);
            pV->dart() = pD;
        }
    }
}

} // namespace DartLib

#undef T_TYPENAME
#undef TBASEMESH
#endif // !_DARTLIB_MESH_2D_H_
