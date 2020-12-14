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
	std::cout << "there are  " << m_boundary_vertices.size() << " vertices and " << m_boundary_edges.size() << " edges\n";
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
	std::map<int, double> m;
	std::vector < std::pair<int, double>> m2;
	idx_verts.push_back(NULL);
    for (auto pV : m_boundary_vertices)
    {
        pV->idx() = vid++;
		idx_verts.push_back(pV);
		graph.push_back(m);
		adj.push_back(m2);
    }
	graph.push_back(m);
	adj.push_back(m2);
	for (auto pE : m_boundary_edges)
	{
		M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
		M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
		graph[pV->idx()][pW->idx()] = (pW->point() - pV->point()).norm();
		graph[pW->idx()][pV->idx()] = (pV->point() - pW->point()).norm();
		adj[pV->idx()].push_back(std::make_pair(pW->idx(), (pW->point() - pV->point()).norm()));
		adj[pW->idx()].push_back(std::make_pair(pV->idx(), (pV->point() - pW->point()).norm()));
	}

    for (M::VertexIterator viter(m_pMesh); !viter.end(); viter++)
    {
        M::CVertex* pV = *viter;
        if (pV->idx() > 0)
            continue;

        pV->idx() = vid++;
		idx_verts.push_back(pV);
        m_inner_vertices.insert(pV);
    }

    int eid = 1;
	idx_edges.push_back(NULL);
    for (auto pE : m_boundary_edges)
    {
        pE->idx() = eid++;
		idx_edges.push_back(pE);
    }

    for (M::EdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        if (pE->idx() > 0)
            continue;

        pE->idx() = eid++;
		idx_edges.push_back(pE);
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
	clock_t b = clock();
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
			//handle_loops.push_back(pE);
        }
        else if (pE->generator() && pE->pair() != NULL)
        {
			// generator that has been killed
			std::cout << "Handle Loop Edge " << pE->idx() << std::endl;
            handle_loops.push_back(pE);
			//pE->sharp() = true;
        }
		else
		{
			std::cout << "this is a bug";
		}
    }
	clock_t e = clock();
	printf("Interior pair time: %g s\n", double(e - b) / CLOCKS_PER_SEC);
    for (size_t i = 0; i < handle_loops.size(); i++)
    {
		_mark_loop(handle_loops[i]);
        //M::CFace* pF = handle_loops[i]->pair();
        //_mark_loop(pF);
    }

}

/*!
*   pair simplices on the boundary surface
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
// =========================================== SMALL IMPROVEMENT: MOVE PAIR VERTEX TO BEGINNING ===============================================
/*!
 *	pair edges;
 */
void CHandleTunnelLoop::_pair(std::set<M::CEdge*>& edges)
{
	int generator_edges = 0;
	int killer_edges = 0;
	std::cout << "starting edge pairing";
	double assign = 0;
	double pairing = 0;

    for (auto eiter = edges.begin(); eiter != edges.end(); eiter++)
    {
        M::CEdge* pE = *eiter;
        std::cout << ".";
		int number = 0;
		clock_t b = clock();
		in.assign(m_inner_vertices.size() + m_boundary_vertices.size() + 1, false);
		clock_t e = clock();
		assign += double(e - b) / CLOCKS_PER_SEC;
		inSet.clear();
		b = clock();
        //insert your code here
        Cycle<M::CVertex, Compare<M::CVertex>> vcycle;
		//start from youngest positive vertex (all vertices are positive)
		M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
		M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
		/*vcycle.add(pV); //vcycle.add(pV, in);
		vcycle.add(pW); //vcycle.add(pW, in);*/
		if (in[pV->idx()])
		{
			in[pV->idx()] = false;
			inSet.erase(pV->idx());
			number -= 1;
		}
		else
		{
			in[pV->idx()] = true;
			inSet.insert(pV->idx());
			number += 1;
		}
		if (in[pW->idx()])
		{
			in[pW->idx()] = false;
			inSet.erase(pW->idx());
			number -= 1;
		}
		else
		{
			in[pW->idx()] = true;
			inSet.insert(pW->idx());
			number += 1;
		}
		//vcycle.print();
		pV = idx_verts[*inSet.rbegin()];
		//while (!vcycle.empty() && pV->pair())
		while (number > 0 && pV->pair())
		{
			M::CEdge* pE2 = pV->pair();
			M::CVertex* pV2 = m_pMesh->edge_vertex(pE2, 0);
			M::CVertex* pW2 = m_pMesh->edge_vertex(pE2, 1);
			//vcycle.add(pV2); //vcycle.add(pV2, in);
			//vcycle.add(pW2); // vcycle.add(pW2, in);
			if (in[pV2->idx()])
			{
				in[pV2->idx()] = false;
				inSet.erase(pV2->idx());
				number -= 1;
			}
			else
			{
				in[pV2->idx()] = true;
				inSet.insert(pV2->idx());
				number += 1;
			}
			if (in[pW2->idx()])
			{
				in[pW2->idx()] = false;
				inSet.erase(pW2->idx());
				number -= 1;
			}
			else
			{
				in[pW2->idx()] = true;
				inSet.insert(pW2->idx());
				number += 1;
			}
			if (*inSet.rbegin())
			{
				pV = idx_verts[*inSet.rbegin()];
			}
			//pV = vcycle.head();
		}
		//if (!vcycle.empty())
		if (number > 0)
		{
			killer_edges += 1;
			pV->pair() = pE;
		}
		else
		{
			generator_edges += 1;
			pE->generator() = true;
		}
		e = clock();
		pairing += double(e - b) / CLOCKS_PER_SEC;

    }
	std::cout << "\n edge pairing took " << pairing << " seconds, while assigning took " << assign << "seconds.";
	std::cout << "\nfinished with edges, there are " << killer_edges << " killers and " << generator_edges << " generator edges.\n";

};

/*!
 *	pair faces
 */
void CHandleTunnelLoop::_pair(std::set<M::CFace*>& faces)
{
	int killer_faces = 0;
	int generator_faces = 0;
	double add_time = 0;
	double head_time = 0;
    for (auto fiter = faces.begin(); fiter != faces.end(); fiter++)
    {
        M::CFace* pF = *fiter;
        std::cout << "-";

		int number = 0;
        //insert your code here
		in.assign(m_inner_edges.size() + m_boundary_edges.size() + 1, false);
		inSet.clear();
		Cycle<M::CEdge, Compare<M::CEdge>> ecycle;
		for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
		{
			M::CEdge* pEd = *feiter;
			//clock_t b = clock();
			// COMBINE THE TWO! USE THE BOOL VECTOR TO CHECK IF IN, THEN USE THE ORIGINAL HEAD(maybe use set)!!-------------------------------------------------------
			// remove is slow, so have an array/vector storing index of the item when it's added for easier removal
			if (in[pEd->idx()])
			{
				in[pEd->idx()] = false;
				inSet.erase(pEd->idx());
				number -= 1;
			}
			else
			{
				in[pEd->idx()] = true;
				inSet.insert(pEd->idx());
				number += 1;
			}
			//ecycle.add(pEd);
			//clock_t e = clock();
			//add_time += double(e - b) / CLOCKS_PER_SEC;


		}
		


		//clock_t b = clock();
		//M::CEdge* pE = ecycle.head();
		
		M::CEdge* pE = idx_edges[*inSet.rbegin()];
		/*int i;
		if (number > 0)
		{
			for (i = m_inner_edges.size() + m_boundary_edges.size(); i > 0; i--)
			{
				if (in[i] && idx_edges[i]->generator())
				{
					pE = idx_edges[i];
					break;
				}
			}
		}*/
		//clock_t e = clock();
		//head_time += double(e - b) / CLOCKS_PER_SEC;

		//while (!ecycle.empty() && pE->pair())
		while(number > 0 && pE->pair())
		{
			M::CFace* pF2 = pE->pair();
			for (M::FaceEdgeIterator feiter2(pF2); !feiter2.end(); ++feiter2)
			{
				M::CEdge* pE2 = *feiter2;
				//clock_t b = clock();
				if (in[pE2->idx()])
				{
					in[pE2->idx()] = false;
					inSet.erase(pE2->idx());
					number -= 1;
				}
				else
				{
					in[pE2->idx()] = true;
					inSet.insert(pE2->idx());
					number += 1;
				}
				//ecycle.add(pE2);
				//clock_t e = clock();
				//add_time += double(e - b) / CLOCKS_PER_SEC;
				
			}
			//clock_t b = clock();
			
			
			if (*inSet.rbegin())
			{
				pE = idx_edges[*inSet.rbegin()];
			}
			/*
			if (number > 0)
			{
				for (int i = m_inner_edges.size() + m_boundary_edges.size(); i > 0; i--)
				{
					if (in[i] && idx_edges[i]->generator())
					{
						pE = idx_edges[i];
						break;
					}
				}
			}*/
			//pE = ecycle.head();
			//clock_t e = clock();
			//head_time += double(e - b) / CLOCKS_PER_SEC;
			
		}
		//if (!ecycle.empty())
		if (number > 0)
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
	//std::cout << "time taken to add edges " << add_time << " while head time was " << head_time << "\n";
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
	std::cout << "====================== started with " << loop_edges.size() << " edges =================\n";
	prune();
	std::vector<M::CEdge*> old_cycle;
	for (auto pE : loop_edges)
	{
		old_cycle.push_back(pE);
		pE->sharp() = false;
	}
	before_edges.push_back(old_cycle);
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
	clock_t start = clock();
	bool cont = true;

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
	M::CEdge* pE = loop_edges[loop_edges.size() / 2];
	M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
	M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
	if (time == 1)
	{
		for (auto E : loop_edges)
		{
			//final_edges.push_back(E);
		}
	}
	time += 1;
	std::cout << "started with " << loop_edges.size() << " edges left\n";
	loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), pE), loop_edges.end());

	while (pV != pW)
	{
		//if (!cont)
		//{
		//	break;
		//}
		loop_vertices.push_back(pV);
		std::cout << "sizes are  " << loop_edges.size() << " and " << loop_vertices.size() <<"\n";
		cont = false;
		for (M::VertexEdgeIterator veiter(m_pMesh, pV); !veiter.end(); ++veiter)
		{
			
			M::CEdge* pE2 = *veiter;
			if (std::find(loop_edges.begin(), loop_edges.end(), pE2) != loop_edges.end())
			{
				loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), pE2), loop_edges.end());
				if (pV == m_pMesh->edge_vertex(pE2, 0))
				{
					pV = m_pMesh->edge_vertex(pE2, 1);
					cont = true;
				}
				else if (pV == m_pMesh->edge_vertex(pE2, 1))
				{
					pV = m_pMesh->edge_vertex(pE2, 0);
					cont = true;
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
	clock_t end = clock();
	std::cout << "shorten time took " << double(end - start) / CLOCKS_PER_SEC << "==============\n";
}

void CHandleTunnelLoop::_shorten()
{
	std::vector<M::CVertex*> old_loop_vertices = loop_vertices;
	int num_vertices = 3;
	std::vector<int> search_verts;
	old_dist = DBL_MAX;
	new_dist = 0;
	while (new_dist == 0 /*|| new_loop.size() <= num_vertices*/)
	{
		std::cout << "retry==================================\n";
		search_verts.clear();
		new_loop.clear();
		old_dist = DBL_MAX;
		new_dist = 0;
		for (int i = 0; i < num_vertices; i++)
		{
			search_verts.push_back(old_loop_vertices[i * old_loop_vertices.size() / num_vertices]->idx());
		}
		for (int i = 0; i < num_vertices; i++)
		{
			std::vector<int> path = dijkstra(search_verts[i % num_vertices], search_verts[(i + 1) % num_vertices]);
		}
		std::cout << "searching with " << search_verts.size() << " verts\n";
		std::cout << "we have " << new_loop.size() << " verts\n";
		/*int pV1 = loop_vertices[0]->idx();
	int pV2 = loop_vertices[loop_vertices.size() / 3]->idx();
	int pV3 = loop_vertices[2 * loop_vertices.size() / 3]->idx();*/
	//std::cout << "the vertices are " << pV1 << ", " << pV2 << ", and " << pV3 << "here\n";

	/*int oG1 = pV1;
	int oG2 = pV2;
	int oG3 = pV3;*/



	// get shortest path between pV1, pV2, and pV3
	/*std::vector<int> path1 = dijkstra(pV1, pV2);
	std::vector<int> path2 = dijkstra(pV2, pV3);
	std::vector<int> path3 = dijkstra(pV3, pV1);*/


	/*new_loop.insert(new_loop.end(), path1.begin(), path1.end());
	new_loop.insert(new_loop.end(), path2.begin(), path2.end());
	new_loop.insert(new_loop.end(), path3.begin(), path3.end());*/

	/*std::vector<int> oG_loop = new_loop;*/
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
		while (old_dist > new_dist)
		{
			std::cout << "did it again\n";
			search_verts.clear();
			old_dist = new_dist;
			new_dist = 0;
			for (int i = 0; i < num_vertices; i++)
			{
				search_verts.push_back(new_loop[(i * new_loop.size() / num_vertices + 1)%new_loop.size()]);
			}
			new_loop.clear();
			for (int i = 0; i < num_vertices; i++)
			{
				std::vector<int> path = dijkstra(search_verts[i % num_vertices], search_verts[(i + 1) % num_vertices]);
			}
			//pV1 = new_loop[0 + 1];
			//pV2 = new_loop[new_loop.size() / 3 + 1];
			//pV3 = new_loop[2 * new_loop.size() / 3 + 1];
			/*for (auto pE : m_boundary_edges)
			{
				int pV = m_pMesh->edge_vertex(pE, 0)->idx();
				int pW = m_pMesh->edge_vertex(pE, 1)->idx();
				if (pV == pV1 || pV == pV2 || pV == pV3 || pW == pV1 || pW == pV2 || pW == pV3)
				{
					pE->sharp() = true;
				}
			}*/
			//std::cout << "the vertices are " << pV1 << ", " << pV2 << ", and " << pV3 << "\n";
			//old_dist = new_dist;
			//new_dist = 0;
			//path1 = dijkstra(pV1, pV2);
			//path2 = dijkstra(pV2, pV3);
			//path3 = dijkstra(pV3, pV1);

			//new_loop.clear();
			//new_loop.insert(new_loop.end(), path1.begin(), path1.end());
			//new_loop.insert(new_loop.end(), path2.begin(), path2.end());
			//new_loop.insert(new_loop.end(), path3.begin(), path3.end());
			std::cout << "old length is " << old_dist << " while the new distance is, " << new_dist << "\n";

			std::cout << "there are " << new_loop.size() << " vertices\n\n";

		}
		std::cout << "final distance was " << new_dist << "\n";
		//for (auto pE : m_boundary_edges)
		//{
		//	int pV = m_pMesh->edge_vertex(pE, 0)->idx();
		//	int pW = m_pMesh->edge_vertex(pE, 1)->idx();
		//	if (pV == pV1 || pV == pV2 || pV == pV3 || pW == pV1 || pW == pV2 || pW == pV3)
		//	{
		//		final_edges.push_back(pE);
		//	}
		//}
		//if (new_loop.size() < 7)
		//{
		//	for (auto pE : m_boundary_edges)
		//	{
		//		int pV = m_pMesh->edge_vertex(pE, 0)->idx();
		//		int pW = m_pMesh->edge_vertex(pE, 1)->idx();
		//		if (pV == oG1 || pV == oG2 || pV == oG3 || pW == oG1 || pW == oG2 || pW == oG3)
		//		{
		//			//pE->sharp() = true;
		//			final_edges.push_back(pE);

		//		}
		//	}
		//	for (auto pE : m_boundary_edges)
		//	{
		//		int pV = m_pMesh->edge_vertex(pE, 0)->idx();
		//		int pW = m_pMesh->edge_vertex(pE, 1)->idx();
		//		std::vector<int>::iterator i1 = std::find(oG_loop.begin(), oG_loop.end(), pV);
		//		std::vector<int>::iterator i2 = std::find(oG_loop.begin(), oG_loop.end(), pW);
		//		if (i1 != oG_loop.end() && i2 != oG_loop.end())
		//		{
		//			if (std::distance(oG_loop.begin(), i1) - std::distance(oG_loop.begin(), i2) == -1
		//				|| std::distance(oG_loop.begin(), i1) - std::distance(oG_loop.begin(), i2) == 1
		//				|| std::distance(oG_loop.begin(), i1) - std::distance(oG_loop.begin(), i2) == oG_loop.size() - 1
		//				|| std::distance(oG_loop.begin(), i1) - std::distance(oG_loop.begin(), i2) == -1 * oG_loop.size() + 1)
		//			{
		//				final_edges.push_back(pE);
		//			}
		//		}
		//	}
		//}
		std::cout << "new loop has size: " << new_loop.size() << " while we started with " << num_vertices << " vertices\n";
		if (new_loop.size() > num_vertices)
		{
			for (auto pE : m_boundary_edges)
			{
				int pV = m_pMesh->edge_vertex(pE, 0)->idx();
				int pW = m_pMesh->edge_vertex(pE, 1)->idx();
				std::vector<int>::iterator i1 = std::find(new_loop.begin(), new_loop.end(), pV);
				std::vector<int>::iterator i2 = std::find(new_loop.begin(), new_loop.end(), pW);
				if (i1 != new_loop.end() && i2 != new_loop.end())
				{
					if (std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == -1
						|| std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == 1
						|| std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == new_loop.size() - 1
						|| std::distance(new_loop.begin(), i1) - std::distance(new_loop.begin(), i2) == -1 * new_loop.size() + 1)
					{
						final_edges.push_back(pE);
					}
				}
			}
		}
		num_vertices *= 2;
		//std::cout << "the length of one random edge is  " << (m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->point() - m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->point()).norm() << "\n";
		//std::cout << "this should be the samve as above " << graph[m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->idx()][m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->idx()] << "\n";
		//std::cout << "also should be the samve as above " << graph[m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->idx()][m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->idx()] << "\n";
		//std::cout << "the first point was " << m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->point().print() << "\n";
		//std::cout << "the second point is " << m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->point().print() << "\n";
		
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

int CHandleTunnelLoop::minDistance(std::vector<double> dist, std::vector<bool> sptSet)
{
	// Initialize min value 
	double min = DBL_MAX;
	int min_index;

	for (int v = 0; v < m_boundary_vertices.size() + 1; v++)
		if (sptSet[v] == false && dist[v] <= min)
			min = dist[v], min_index = v;

	return min_index;
}

std::vector<int> CHandleTunnelLoop::dijkstra2(int src, int dest)
{
	// The output array. dist[i] 
	// will hold the shortest 
	// distance from src to i 
	std::vector<double> dist(m_boundary_vertices.size()+1);
	//double dist[V];

	// sptSet[i] will true if vertex 
	// i is included / in shortest 
	// path tree or shortest distance  
	// from src to i is finalized 
	//bool sptSet[V];
	std::vector<bool> sptSet(m_boundary_vertices.size() + 1);

	// Parent array to store 
	// shortest path tree 
	//int parent[V];
	std::vector<int> parent(m_boundary_vertices.size() + 1);

	// Initialize all distances as  
	// INFINITE and stpSet[] as false 
	for (int i = 0; i < m_boundary_vertices.size() + 1; i++)
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
	for (int count = 0; count < m_boundary_vertices.size(); count++)
	{
		//std::cout << "at " << count << "\n";
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
		for (int v = 0; v < m_boundary_vertices.size(); v++)
		{
			// Update dist[v] only if is 
			// not in sptSet, there is 
			// an edge from u to v, and  
			// total weight of path from 
			// src to v through u is smaller 
			// than current value of 
			// dist[v] 
			if (!sptSet[v] && graph[u][v] && dist[u] + graph[u][v] < dist[v])
			{
				parent[v] = u;
				dist[v] = dist[u] + graph[u][v];
			}
		}
	}
	new_dist += dist[dest];
	// print the constructed 
	// distance array 
	int loc = dest;
	std::vector<int> new_verts;
	while (loc != src)
	{
		loc = parent[loc];
		new_verts.insert(new_verts.begin(), loc);
	}
	std::cout << "new_verts had " << new_verts.size() << "vertices\n";
	new_loop.insert(new_loop.end(), new_verts.begin(), new_verts.end());
	return new_verts;
}














std::vector<int> CHandleTunnelLoop::dijkstra(int s, int t)
{
	std::vector<double> d;
	std::vector<int> p;
	int n = adj.size();
	d.assign(n, 10000000.0);
	p.assign(n, -1);

	d[s] = 0;
	std::set<std::pair<double, int>> q;
	q.insert({ 0.0, s });
	while (!q.empty()) {
		int v = q.begin()->second;
		if (v == t)
		{
			break;
		}
		q.erase(q.begin());

		for (auto edge : adj[v]) {
			int to = edge.first;
			double len = edge.second;

			if (d[v] + len < d[to]) 
			{
				q.erase({ d[to], to });
				d[to] = d[v] + len;
				p[to] = v;
				q.insert({ d[to], to });
			}
		}
	}
	new_dist += d[t];
	// print the constructed 
	// distance array 
	int loc = t;
	std::vector<int> new_verts;
	while (loc != s)
	{
		loc = p[loc];
		new_verts.insert(new_verts.begin(), loc);
	}
	std::cout << "new_verts had " << new_verts.size() << "vertices\n";
	new_loop.insert(new_loop.end(), new_verts.begin(), new_verts.end());
	return new_verts;
}

















void CHandleTunnelLoop::display()
{
	for (auto pE : m_boundary_edges)
	{
		pE->sharp() = false;
	}
	for (auto pE : final_edges)
	{
		pE->sharp() = true;
	}
}
void CHandleTunnelLoop::display_before(int which)
{
	which = which % before_edges.size();
	std::cout << "displaying " << which << "\n";
	for (auto pE : m_boundary_edges)
	{
		pE->sharp() = false;
	}
	for (auto pE : before_edges[which])
	{
		pE->sharp() = true;
	}
}
}
