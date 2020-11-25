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

	for (auto pE : m_boundary_edges)
	{
		M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
		M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
		graph[pV->idx()][pW->idx()] = (pW->point() - pV->point()).norm();
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
			//_mark_loop(pE);
        }
        else if (pE->generator() && pE->pair() != NULL)
        {
			// generator that has been killed
			std::cout << "Handle Loop Edge " << pE->idx() << std::endl;
            handle_loops.push_back(pE);
			_mark_loop(pE);
			//pE->sharp() = true;
        }
		else
		{
			std::cout << "this is a bug";
		}
    }
    
    for (size_t i = 0; i < handle_loops.size(); i++)
    {
        M::CFace* pF = handle_loops[i]->pair();
        //_mark_loop(pF);
    }
	for (auto pE : final_edges)
	{
		pE->sharp() = true;
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
            std::cout << "Generator Edge that has not been killed " << pE->idx() << std::endl;
            m_generators.insert(pE);
        }
    }
	/*for (auto eiter = m_boundary_edges.begin(); eiter != m_boundary_edges.end(); eiter++)
	{
		M::CEdge* pE = *eiter;
		if (pE->generator() && pE->pair() == NULL)
		{
			std::cout << "Generator Edge " << pE->idx() << std::endl;
			//_mark_loop(pE);
		}
		else if (pE->generator() && pE->pair() != NULL)
		{
			// generator that has been killed
			std::cout << "Handle Loop Edge " << pE->idx() << std::endl;
			//_mark_loop(pE);
			//pE->sharp() = true;
		}
		else
		{
			std::cout << "this is not a generator\n";
		}
	}*/

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
	int generator_edges = 0;
	int killer_edges = 0;
    for (auto eiter = edges.begin(); eiter != edges.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        std::cout << ".";

        //insert your code here
        Cycle<M::CVertex, Compare<M::CVertex>> vcycle;
		//start from youngest positive vertex (all vertices are positive)
		M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
		M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
		vcycle.add(pV); //vcycle.add(pV, in);
		vcycle.add(pW); //vcycle.add(pW, in);
		//vcycle.print();
		pV = vcycle.head();
		while (!vcycle.empty() && pV->pair())
		{
			/*std::cout << "--------\n";
			for (auto const& pair : in) {
				std::cout << "{" << pair.first << ": " << pair.second << "}\n";
			}*/
			M::CEdge* pE2 = pV->pair();
			//std::cout << pE2->idx() << " this was the index of the edge\n";
			M::CVertex* pV2 = m_pMesh->edge_vertex(pE2, 0);
			//std::cout << pV2->idx() << " this was the index of vertex 1\n";
			
			//vcycle.print();
			M::CVertex* pW2 = m_pMesh->edge_vertex(pE2, 1);
			//std::cout << pW2->idx() << " this was the index of vertex 2\n";
			vcycle.add(pV2); //vcycle.add(pV2, in);
			vcycle.add(pW2); // vcycle.add(pW2, in);
			//vcycle.print();
			pV = vcycle.head();
		}
		if (!vcycle.empty())
		{
			killer_edges += 1;
			pV->pair() = pE;
		}
		else
		{
			generator_edges += 1;
			pE->generator() = true;
		}
		

    }
	std::cout << "\nfinished with edges, there are " << killer_edges << " killers and " << generator_edges << " generator edges.\n";

};

/*!
 *	pair faces
 */
void CHandleTunnelLoop::_pair(std::set<M::CFace*>& faces)
{
	int killer_faces = 0;
	int generator_faces = 0;
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
			for (M::FaceEdgeIterator feiter2(pF2); !feiter2.end(); ++feiter2)
			{
				M::CEdge* pE2 = *feiter2;
				ecycle.add(pE2);
			}
			pE = ecycle.head();
		}
		if (!ecycle.empty())
		{
			killer_faces += 1;
			pE->pair() = pF;
		}
		else
		{
			generator_faces += 1;
			pF->generator() = true;
		}
    }
	std::cout << "finished, there are " << killer_faces << " killers and " << generator_faces << " generator faces.\n";
};

/*!
 *	mark the handle and tunnel loops as sharp edges
 */
void CHandleTunnelLoop::_mark_loop(M::CFace* face1)
{
    std::set<M::CFace*> section;
    //insert your code here

	//these are the faces that killed the generator edge
    if (m_inner_faces.find(face1) != m_inner_faces.end())
        section.insert(face1);
	std::cout << "starting face: " << face1->idx() << "\n";
    // Cycle<M::CEdge, Compare<M::CEdge>> ecycle;
	int count = 0;
	std::set<M::CFace*>::iterator face;
	for (face = section.begin();; ++face) 
	{
		std::cout << "face: " << (*face)->idx() << "\n";
		/*if (face1->idx() == (*face)->idx())
		{
			count += 1;
			if (count >= 2)
			{
				break;
			}
		}*/
		for (M::FaceEdgeIterator feiter(*face); !feiter.end(); ++feiter)
		{
			M::CEdge* pE = *feiter;
			pE->sharp() = true;
			//ecycle.add(pE);
			//sometimes it is unpaired!
			if (pE->pair() != NULL)
			{	
				std::cout << "inserted face: " << (pE->pair())->idx() << "\n";
				section.insert(pE->pair());
			}

		}
	}

	//while (!ecycle.empty())
	//{
	//	M::CEdge* pE = ecycle.head();
	//	ecycle.add(pE);
	//	pE->sharp() = true;

	//}

};

void CHandleTunnelLoop::_mark_loop(M::CEdge* pE)
{
	pE->sharp() = true;
	loop_edges.clear();
	loop_vertices.clear();
	loop_edges.push_back(pE);
	Cycle<M::CVertex, Compare<M::CVertex>> vcycle;
	//start from youngest positive vertex (all vertices are positive)
	M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
	M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
	vcycle.add(pV); //vcycle.add(pV, in);
	vcycle.add(pW); //vcycle.add(pW, in);
	pV = vcycle.head();
	while (!vcycle.empty() && pV->pair())
	{
		M::CEdge* pE2 = pV->pair();
		pE2->sharp() = true;
		loop_edges.push_back(pE2);
		M::CVertex* pV2 = m_pMesh->edge_vertex(pE2, 0);
		vcycle.add(pV2);
		M::CVertex* pW2 = m_pMesh->edge_vertex(pE2, 1);
		vcycle.add(pW2);
		pV = vcycle.head();
		
	}
	if (!vcycle.empty())
	{
		std::cout << "this shouldn't be happening\n";
		vcycle.print();
		pV->pair() = pE;
	}
	else
	{
		std::cout << "this is right";
		pE->generator() = true;
	}
	prune();
	for (auto pE : loop_edges)
	{
		pE->sharp() = false;
	}
	shorten();
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

void CHandleTunnelLoop::shorten()
{
	/*bool isContinue = false;
	do
	{
		_prune();
		isContinue = _shrink_triangles();
	} while (isContinue);*/
	// display the new list of sharp ones lol
	/*for (auto pE : m_boundary_edges)
	{
		pE->sharp() = false;
	}*/
	/*for (auto pE : loop_edges)
	{
		pE->sharp() = true;
	}*/
	// sort the loop edges first
	M::CEdge* pE = loop_edges[0];
	M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
	M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
	std::cout << "started with " << loop_edges.size() << " edges left\n";
	loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), pE), loop_edges.end());
	while (pV != pW)
	{
		loop_vertices.push_back(pV);
		std::cout << "sizes are  " << loop_edges.size() << " and " << loop_vertices.size() <<"\n";
		for (M::VertexEdgeIterator veiter(m_pMesh, pV); !veiter.end(); ++veiter)
		{
			M::CEdge* pE2 = *veiter;
			if (std::find(loop_edges.begin(), loop_edges.end(), pE2) != loop_edges.end())
			{
				loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), pE2), loop_edges.end());
				if (pV == m_pMesh->edge_vertex(pE2, 0))
				{
					pV = m_pMesh->edge_vertex(pE2, 1);
				}
				else if (pV == m_pMesh->edge_vertex(pE2, 1))
				{
					pV = m_pMesh->edge_vertex(pE2, 0);
				}
				else
				{
					std::cout << "something wrong happened with finding the vertices of the loop";
				}
				break;
			}
		}
	}


	std::cout << "there are now " << loop_edges.size() <<" edges left\n";
	std::cout << "there are  " << loop_vertices.size() << " vertices\n";
	_shorten();

}

void CHandleTunnelLoop::_shorten()
{
	int pV1 = loop_vertices[0]->idx();
	int pV2 = loop_vertices[loop_vertices.size() / 3]->idx();
	int pV3 = loop_vertices[2 * loop_vertices.size() / 3]->idx();

	/*for (auto pE : m_boundary_edges)
	{
		int pV = m_pMesh->edge_vertex(pE, 0)->idx();
		int pW = m_pMesh->edge_vertex(pE, 1)->idx();
		if (pV == pV1 || pV == pV2 || pV == pV3 || pW == pV1 || pW == pV2 || pW == pV3)
		{
			pE->sharp() = true;
		}
	}*/

	// get shortest path between pV1, pV2, and pV3
	std::vector<int> path1 = dijkstra(graph, pV1, pV2);
	std::vector<int> path2 = dijkstra(graph, pV2, pV3);
	std::vector<int> path3 = dijkstra(graph, pV3, pV1);

	int old = loop_vertices.size();
	std::vector<int> new_loop;
	new_loop.insert(new_loop.end(), path1.begin(), path1.end());
	new_loop.insert(new_loop.end(), path2.begin(), path2.end());
	new_loop.insert(new_loop.end(), path3.begin(), path3.end());
	/*for (auto pE : m_boundary_edges)
	{
		int pV = m_pMesh->edge_vertex(pE, 0)->idx();
		int pW = m_pMesh->edge_vertex(pE, 1)->idx();
		std::vector<int>::iterator i1 = std::find(new_loop.begin(), new_loop.end(), pV);
		std::vector<int>::iterator i2 = std::find(new_loop.begin(), new_loop.end(), pW);
		if (i1 != new_loop.end() && i2 != new_loop.end())
		{
			if (std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == -1 || std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == 1)
			{
				//n += 1;
				//std::cout << "edge " << n << "\n";
				pE->sharp() = true;
			}
		}
	}*/
	while (old - new_loop.size() >= 1)
	{
		std::cout << "did it again\n";
		pV1 = new_loop[0 + new_loop.size() / 6];
		pV2 = new_loop[new_loop.size() / 3 + new_loop.size() / 6];
		pV3 = new_loop[2 * new_loop.size() / 3 + new_loop.size() / 6];
		/*for (auto pE : m_boundary_edges)
		{
			int pV = m_pMesh->edge_vertex(pE, 0)->idx();
			int pW = m_pMesh->edge_vertex(pE, 1)->idx();
			if (pV == pV1 || pV == pV2 || pV == pV3 || pW == pV1 || pW == pV2 || pW == pV3)
			{
				pE->sharp() = true;
			}
		}*/
		std::vector<int> path1 = dijkstra(graph, pV1, pV2);
		std::vector<int> path2 = dijkstra(graph, pV2, pV3);
		std::vector<int> path3 = dijkstra(graph, pV3, pV1);
		old = new_loop.size();
		new_loop.clear();
		new_loop.insert(new_loop.end(), path1.begin(), path1.end());
		new_loop.insert(new_loop.end(), path2.begin(), path2.end());
		new_loop.insert(new_loop.end(), path3.begin(), path3.end());
	}
	int n = 0;
	
	for (auto pE : m_boundary_edges)
	{
		int pV = m_pMesh->edge_vertex(pE, 0)->idx();
		int pW = m_pMesh->edge_vertex(pE, 1)->idx();
		std::vector<int>::iterator i1 = std::find(new_loop.begin(), new_loop.end(), pV);
		std::vector<int>::iterator i2 = std::find(new_loop.begin(), new_loop.end(), pW);
		if ( i1 != new_loop.end() && i2 != new_loop.end())
		{
			if (std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == -1
				|| std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == 1
				|| std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == new_loop.size() - 1
				|| std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == -1 * new_loop.size() + 1)
			{
				n += 1;
				std::cout << "edge " << n << "\n";
				final_edges.push_back(pE);
			}
		}
	}
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
			loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), edges[0]), loop_edges.end());
			/*std::vector<M::CEdge*>::iterator position = std::find(loop_edges.begin(), loop_edges.end(), edges[0]);
			if (position != loop_edges.end())
			{
				std::cout << "we erased a 3-edge called " << *position << "\n";
				loop_edges.erase(position);
			}
			else
			{
				std::cout << "we can't find the edge which is part of a 3edge " << edges[0] << "\n";
			}*/
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
				{
					if (edges[i]->sharp())
					{
						/*std::vector<M::CEdge*>::iterator position = std::find(loop_edges.begin(), loop_edges.end(), edges[i]);
						if (position != loop_edges.end())
						{
							std::cout << "we erased a 2-edge called " << *position << "\n";
							loop_edges.erase(position);
						}
						else
						{
							std::cout << "we can't find " << edges[i] << "\n";
						}*/
						loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), edges[i]), loop_edges.end());
					}
					else if (!edges[i]->sharp())
					{
						std::cout << "we added an edge here\n";
						loop_edges.push_back(edges[i]);
					}
					edges[i]->sharp() = !edges[i]->sharp();
					
				}
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
				/*std::vector<M::CEdge*>::iterator position = std::find(loop_edges.begin(), loop_edges.end(), pE);
				if (position != loop_edges.end())
				{
					std::cout << "we erased an edge during the pruning called " << *position << "\n";
					loop_edges.erase(position);
				}
				else
				{
					std::cout << "we couldn't find " << pE << " in the list of loop_edges\n";
				}*/
				loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), pE), loop_edges.end());

                if (pW->valence() == 1)
                    vQueue.push(pW);

                break;
            }
        }
    }
}

int CHandleTunnelLoop::minDistance(double dist[], bool sptSet[])
{
	// Initialize min value 
	double min = DBL_MAX;
	int min_index;

	for (int v = 0; v < V; v++)
		if (sptSet[v] == false && dist[v] <= min)
			min = dist[v], min_index = v;

	return min_index;
}

std::vector<int> CHandleTunnelLoop::dijkstra(double graph[V][V], int src, int dest)
{

	// The output array. dist[i] 
	// will hold the shortest 
	// distance from src to i 
	double dist[V];

	// sptSet[i] will true if vertex 
	// i is included / in shortest 
	// path tree or shortest distance  
	// from src to i is finalized 
	bool sptSet[V];

	// Parent array to store 
	// shortest path tree 
	int parent[V];

	// Initialize all distances as  
	// INFINITE and stpSet[] as false 
	for (int i = 0; i < V; i++)
	{
		parent[0] = -1;
		dist[i] = DBL_MAX;
		sptSet[i] = false;
	}

	// Distance of source vertex  
	// from itself is always 0 
	dist[src] = 0.0;

	// Find shortest path 
	// for all vertices 
	for (int count = 0; count < V - 1; count++)
	{
		// Pick the minimum distance 
		// vertex from the set of 
		// vertices not yet processed.  
		// u is always equal to src 
		// in first iteration. 
		int u = minDistance(dist, sptSet);

		// Mark the picked vertex  
		// as processed 
		sptSet[u] = true;

		// Update dist value of the  
		// adjacent vertices of the 
		// picked vertex. 
		for (int v = 0; v < V; v++)

			// Update dist[v] only if is 
			// not in sptSet, there is 
			// an edge from u to v, and  
			// total weight of path from 
			// src to v through u is smaller 
			// than current value of 
			// dist[v] 
			if (!sptSet[v] && graph[u][v] &&
				dist[u] + graph[u][v] < dist[v])
			{
				parent[v] = u;
				dist[v] = dist[u] + graph[u][v];
			}
	}

	// print the constructed 
	// distance array 
	int loc = dest;
	std::vector<int> new_verts;
	while (loc != src)
	{
		loc = parent[loc];
		new_verts.insert(new_verts.begin(), loc);
	}
	return new_verts;
}
}
