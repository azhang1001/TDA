#include "HandleTunnelLoop.h"

namespace DartLib
{

/*!
 *	constructor of the CHandleTunnelLoop
 */
CHandleTunnelLoop::CHandleTunnelLoop() : m_pMesh(NULL) {}

CHandleTunnelLoop::CHandleTunnelLoop(M* pMesh) { set_mesh(pMesh); }

void CHandleTunnelLoop::set_mesh(M* pMesh)
{
    m_pMesh = pMesh;

    for (M::VertexIterator viter(m_pMesh); !viter.end(); viter++)
    {
        M::CVertex* pV = *viter;
        pV->pair() = NULL;
    }

    for (M::EdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        pE->pair() = NULL;
    }

    for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
    {
        M::CFace* pF = *fiter;
        pF->pair() = NULL;
    }

    _extract_boundary_surface();
    _extract_interior_volume();
};

/*! extract the boundry surface, define the filtration
*
*/
void CHandleTunnelLoop::_extract_boundary_surface()
{
    for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
    {
        M::CFace* pF = *fiter;
        if (m_pMesh->boundary(pF))
        {
            m_boundary_faces.insert(pF);
        }
    }

    for (auto pF : m_boundary_faces)
    {
        for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
        {
            M::CEdge* pE = *feiter;
            m_boundary_edges.insert(pE);
        }
    }

    for (auto pE : m_boundary_edges)
    {
        M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
        M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);

        m_boundary_vertices.insert(pV);
        m_boundary_vertices.insert(pW);
    }

    int euler_number = m_boundary_vertices.size() + m_boundary_faces.size() - m_boundary_edges.size();
    m_genus = (2 - euler_number) / 2;
    std::cout << "Genus of the Boundary Mesh is " << m_genus << std::endl;
}

/*!
 *	extract interior volume, define the filtration
 */
void CHandleTunnelLoop::_extract_interior_volume()
{
    int vid = 1;
    for (auto pV : m_boundary_vertices)
    {
        pV->idx() = vid++;
    }

    for (M::VertexIterator viter(m_pMesh); !viter.end(); viter++)
    {
        M::CVertex* pV = *viter;
        if (pV->idx() > 0)
            continue;

        pV->idx() = vid++;
        m_inner_vertices.insert(pV);
    }

    int eid = 1;
    for (auto pE : m_boundary_edges)
    {
        pE->idx() = eid++;
    }

    for (M::EdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        if (pE->idx() > 0)
            continue;

        pE->idx() = eid++;
        m_inner_edges.insert(pE);
    }

    int fid = 1;
    for (auto pF : m_boundary_faces)
    {
        pF->idx() = fid++;
    }

    for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
    {
        M::CFace* pF = *fiter;
        if (pF->idx() > 0)
            continue;

        pF->idx() = fid++;
        m_inner_faces.insert(pF);
    }
}
/*!
*   pair the interior simplices 
*/
void CHandleTunnelLoop::interior_volume_pair()
{
    _pair(m_inner_vertices);
    _pair(m_inner_edges);
    _pair(m_inner_faces);

    std::cout << "After pairing the Interior Volume :" << std::endl;
    std::vector<M::CEdge*> handle_loops;
    for (auto eiter = m_generators.begin(); eiter != m_generators.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        if (pE->generator() && pE->pair() == NULL)
        {
            std::cout << "Generator Edge " << pE->idx() << std::endl;
        }
        else
        {
            handle_loops.push_back(pE);
        }
    }
    
    for (size_t i = 0; i < handle_loops.size(); i++)
    {
        M::CFace* pF = handle_loops[i]->pair();
        _mark_loop(pF);
    }
}

/*!
*   pair simplices on the boundaray surface
*/
void CHandleTunnelLoop::boundary_surface_pair()
{
    _pair(m_boundary_vertices);
    _pair(m_boundary_edges);
    _pair(m_boundary_faces);

    std::cout << "After Pairing the boundary surface: " << std::endl;
    for (auto eiter = m_boundary_edges.begin(); eiter != m_boundary_edges.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        if (pE->generator() && pE->pair() == NULL)
        {
            std::cout << "Generator Edge " << pE->idx() << std::endl;
            m_generators.insert(pE);
        }
    }
}

/*!
 * pair vertices
 */
void CHandleTunnelLoop::_pair(std::set<M::CVertex*>& vertices)
{
    //insert your code here
    for (auto viter = vertices.begin(); viter != vertices.end(); viter++)
    {
        M::CVertex* pV = *viter;
        //label the generators (all vertices are generators)
		pV->generator() = true;

    }
};

/*!
 *	pair edges;
 */
void CHandleTunnelLoop::_pair(std::set<M::CEdge*>& edges)
{

    for (auto eiter = edges.begin(); eiter != edges.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        std::cout << ".";

        //insert your code here
        Cycle<M::CVertex, Compare<M::CVertex>> vcycle;
		//start from youngest positive vertex (all vertices are positive)
		M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
		M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
		vcycle.add(pV);
		vcycle.add(pW);
		//vcycle.print();
		pV = vcycle.head();
		while (!vcycle.empty() && pV->pair())
		{
			M::CEdge* pE2 = pV->pair();
			//std::cout << pE2->idx() << " this was the index of the edge\n";
			M::CVertex* pV2 = m_pMesh->edge_vertex(pE2, 0);
			//std::cout << pV2->idx() << " this was the index of vertex 1\n";
			vcycle.add(pV2);
			//vcycle.print();
			M::CVertex* pW2 = m_pMesh->edge_vertex(pE2, 1);
			//std::cout << pW2->idx() << " this was the index of vertex 2\n";
			vcycle.add(pW2);
			//vcycle.print();
			pV = vcycle.head();
		}
		if (!vcycle.empty())
		{
			pV->pair() = pE;
		}
		else
		{
			pE->generator() = true;
		}
		

    }

};

/*!
 *	pair faces
 */
void CHandleTunnelLoop::_pair(std::set<M::CFace*>& faces)
{
    for (auto fiter = faces.begin(); fiter != faces.end(); fiter++)
    {
        M::CFace* pF = *fiter;
        std::cout << "-";

        //insert your code here
		Cycle<M::CEdge, Compare<M::CEdge>> ecycle;
		for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
		{
			M::CEdge* pEd = *feiter;
			ecycle.add(pEd);
		}
		M::CEdge* pE = ecycle.head();

		while (!ecycle.empty() && pE->pair())
		{
			M::CFace* pF2 = pE->pair();
			for (M::FaceEdgeIterator feiter(pF2); !feiter.end(); ++feiter)
			{
				M::CEdge* pE2 = *feiter;
				ecycle.add(pE2);
			}
			pE = ecycle.head();
		}
		if (!ecycle.empty())
		{
			pE->pair() = pF;
		}
		else
		{
			pF->generator() = true;
		}
    }
};

/*!
 *	mark the handle and tunnel loops as sharp edges
 */
void CHandleTunnelLoop::_mark_loop(M::CFace* face)
{
    std::set<M::CFace*> section;
    //insert your code here

    if (m_inner_faces.find(face) != m_inner_faces.end())
        section.insert(face);

    Cycle<M::CEdge, Compare<M::CEdge>> ecycle;
    for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
    {
        M::CEdge* pE = *feiter;
        ecycle.add(pE);
		pE->sharp() = true;
    }

};


void CHandleTunnelLoop::write_m(const std::string& output)
{
    std::fstream _os(output, std::fstream::out);

    if (_os.fail())
    {
        fprintf(stderr, "Error is opening file %s\n", output);
        return;
    }

    M::CBoundary boundary(m_pMesh);
    const auto& surface = boundary.boundary_surface();
    
    std::set<M::CVertex*> vSet;
    for (auto pF : surface)
    {
        for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            M::CVertex* pV = *fviter;
            vSet.insert(pV);
        }
    }

    for (auto pV : vSet)
    {
        CPoint& p = pV->point();
        _os << "Vertex " << pV->idx();
        _os << " " << p[0] << " " << p[1] << " " << p[2] << "\n";
    }

    for (auto pF : surface)
    {
        _os << "Face " << pF->idx();
        for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            M::CVertex* pV = *fviter;
            _os << " " << pV->idx();
        }
        _os << "\n";
    }

    for (auto pE : m_boundary_edges)
    {
        M::CVertex* pv1 = m_pMesh->edge_vertex(pE, 0);
        M::CVertex* pv2 = m_pMesh->edge_vertex(pE, 1);

        if (pE->sharp())
        {
            _os << "Edge " << pv1->idx() << " " << pv2->idx() << " ";
            _os << "{sharp}" << std::endl;
        }
    }

    _os.close();
}

void CHandleTunnelLoop::exact_boundary(S& surface) 
{
    M::CBoundary boundary(m_pMesh);
    const auto& boundary_surface = boundary.boundary_surface();

    std::set<M::CVertex*> vSet;
    for (auto pF : boundary_surface)
    {
        for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            M::CVertex* pV = *fviter;
            vSet.insert(pV);
        }
    }

    std::vector<CPoint> pts;
    int vid = 1;
    for (auto pV : vSet)
    {
        pV->idx() = vid++;
        pts.push_back(pV->point());
    }

    std::vector<int> indices;
    for (auto pF : boundary_surface)
    {
        for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
        {
            M::CVertex* pV = *fviter;
            indices.push_back( pV->idx() - 1 );
        }
    }

    surface.load(pts, indices);
}

void CHandleTunnelLoop::prune()
{
    bool isContinue = false;
    do
    {
        _prune();
        isContinue = _shrink_triangles();
    } while (isContinue);
}

bool CHandleTunnelLoop::_shrink_triangles()
{
    int count = 0;

    for (auto pF : m_boundary_faces)
    {
        int nSharp = 0;
        std::vector<M::CEdge*> edges;
        for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
        {
            M::CEdge* pE = *feiter;
            edges.push_back(pE);
            if (pE->sharp())
                nSharp++;
        }

        /*
               .------.          .      .
                \    /   ----->   \    / 
                 \  /   (unsharp)  \  /
                  \/                \/
         */
        if (nSharp == 3)
        {
            edges[0]->sharp() = false;
            count++;
        }

        /*
               .      .          .------.
                \    /   ----->   
                 \  /    (switch  
                  \/   sharp edges) 
         */
        if (nSharp == 2)
        {
            std::vector<M::CVertex*> verts;
            M::CVertex* pCommonVertex = NULL;
            for (int i = 0; i < 3; ++i)
            {
                if (edges[i]->sharp())
                {
                    for (int j = 0; j < 2; ++j)
                    {
                        M::CVertex* pV = m_pMesh->edge_vertex(edges[i], j);
                        if (std::find(verts.begin(), verts.end(), pV) == verts.end())
                            verts.push_back(pV);
                        else
                        {
                            pCommonVertex = pV;
                            break;
                        }
                    }
                }
                if (pCommonVertex)
                    break;
            }

            if (pCommonVertex && pCommonVertex->valence() == 2)
            {
                for (int i = 0; i < 3; ++i)
                    edges[i]->sharp() = !edges[i]->sharp();
                count++;
            }
        }
    }

    return count > 0;
}

void CHandleTunnelLoop::_prune()
{
    std::set<M::CVertex*> vSet;

    for (auto pE : m_boundary_edges)
    {
        if (pE->sharp())
        {
            for (int i = 0; i < 2; ++i)
            {
                M::CVertex* pV = m_pMesh->edge_vertex(pE, i);
                pV->valence() = 0;
                vSet.insert(pV);
            }
        }
    }

    for (auto pE : m_boundary_edges)
    {
        if (pE->sharp())
        {
            for (int i = 0; i < 2; ++i)
            {
                M::CVertex* pV = m_pMesh->edge_vertex(pE, i);
                pV->valence() += 1;
            }
        }
    }

    std::queue<M::CVertex*> vQueue;
    for (auto pV : vSet)
    {
        if (pV->valence() == 1)
            vQueue.push(pV);
    }

    while (!vQueue.empty())
    {
        M::CVertex* pV = vQueue.front();
        vQueue.pop();

        for (M::VertexEdgeIterator veiter(m_pMesh, pV); !veiter.end(); ++veiter)
        {
            M::CEdge* pE = *veiter;
            if (m_pMesh->boundary(pE) && pE->sharp())
            {
                M::CVertex* pW = pV == m_pMesh->edge_vertex(pE, 0) ? m_pMesh->edge_vertex(pE, 1) : m_pMesh->edge_vertex(pE, 0);
                pW->valence() -= 1;

                pE->sharp() = false;

                if (pW->valence() == 1)
                    vQueue.push(pW);

                break;
            }
        }
    }
}

}
