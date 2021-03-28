#include "HandleTunnelLoop.h"



namespace DartLib
{



	/*!
	 *	constructor of the CHandleTunnelLoop
	 */
	CHandleTunnelLoop::CHandleTunnelLoop() : m_pMesh(NULL) {}

	CHandleTunnelLoop::CHandleTunnelLoop(M* pMesh) { set_mesh(pMesh); }
	void CHandleTunnelLoop::set_name(std::string name)
	{
		file_name = name;
	}
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
		m_boundary_faces.clear();
		m_boundary_edges.clear();
		m_boundary_vertices.clear();
		for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
		{
			M::CFace* pF = *fiter;
			pF->idx() = NULL;
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
				pE->idx() = NULL;
				m_boundary_edges.insert(pE);
			}
		}

		for (auto pE : m_boundary_edges)
		{
			M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
			pV->idx() = NULL;
			pW->idx() = NULL;
			m_boundary_vertices.insert(pV);
			m_boundary_vertices.insert(pW);
		}
		std::cout << "there are  " << m_boundary_vertices.size() << " vertices, " << m_boundary_edges.size() << " edges, and " << m_boundary_faces.size() << " faces\n";
		int euler_number = m_boundary_vertices.size() + m_boundary_faces.size() - m_boundary_edges.size();
		m_genus = (2 - euler_number) / 2;
		std::cout << "Genus of the Boundary Mesh is " << m_genus << std::endl;
	}

	/*!
	 *	extract interior volume, define the filtration
	 */
	void CHandleTunnelLoop::_extract_interior_volume()
	{
		m_inner_faces.clear();
		m_inner_edges.clear();
		m_inner_vertices.clear();
		int vid = 1;
		std::map<int, double> m;
		std::vector < std::pair<int, double>> m2;
		idx_verts.clear();
		idx_verts.push_back(NULL);
		pointVertex.clear();
		idx_all_verts.clear();
		graph.clear();
		adj.clear();
		for (auto pV : m_boundary_vertices)
		{
			pV->idx() = vid++;
			CPoint& p = pV->point();
			idx_verts.push_back(pV);
			pointVertex[pV->point().print()] = pV->idx();
			idx_all_verts.push_back(pV);
			graph.push_back(m);
			adj.push_back(m2);
		}
		graph.push_back(m);
		adj.push_back(m2);
		

		for (M::VertexIterator viter(m_pMesh); !viter.end(); viter++)
		{
			M::CVertex* pV = *viter;
			if (pV->idx() > 0)
				continue;

			pV->idx() = vid++;
			idx_verts.push_back(pV);
			pointVertex[pV->point().print()] = pV->idx();
			idx_all_verts.push_back(pV);
			adj.push_back(m2);
			m_inner_vertices.insert(pV);
		}

		int eid = 1;
		std::vector<M::CFace*> empty_faces;
		edges_faces.clear();
		idx_edges.clear();
		pointsEdge.clear();
		edges_faces.push_back(empty_faces);
		idx_edges.push_back(NULL);
		for (auto pE : m_boundary_edges)
		{
			pE->idx() = eid;
			eid++;
			M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
			pointsEdge[pV->point().print() + pW->point().print()] = pE->idx();
			pointsEdge[pW->point().print() + pV->point().print()] = pE->idx();
			edges_faces.push_back(empty_faces);
			idx_edges.push_back(pE);
		}
		/*for (auto pE : m_boundary_edges)
		{
			M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
			pointsEdge[pV->point().print() + pW->point().print()] = pE->idx();
			pointsEdge[pW->point().print() + pV->point().print()] = pE->idx();
			graph[pV->idx()][pW->idx()] = (pW->point() - pV->point()).norm();
			graph[pW->idx()][pV->idx()] = (pV->point() - pW->point()).norm();
			adj[pV->idx()].push_back(std::make_pair(pW->idx(), (pW->point() - pV->point()).norm()));
			adj[pW->idx()].push_back(std::make_pair(pV->idx(), (pV->point() - pW->point()).norm()));
		}*/

		for (M::EdgeIterator eiter(m_pMesh); !eiter.end(); eiter++)
		{
			M::CEdge* pE = *eiter;
			if (pE->idx() > 0)
				continue;

			pE->idx() = eid;
			eid++;
			idx_edges.push_back(pE);
			edges_faces.push_back(empty_faces);
			M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
			pointsEdge[pV->point().print() + pW->point().print()] = pE->idx();
			pointsEdge[pW->point().print() + pV->point().print()] = pE->idx();
			m_inner_edges.insert(pE);
			adj[pV->idx()].push_back(std::make_pair(pW->idx(), (pW->point() - pV->point()).norm()));
			adj[pW->idx()].push_back(std::make_pair(pV->idx(), (pV->point() - pW->point()).norm()));
		}

		int fid = 1;
		idx_faces.clear();
		idx_faces.push_back(NULL);
		for (auto pF : m_boundary_faces)
		{
			pF->idx() = fid++;
			idx_faces.push_back(pF);
		}
		face_exist.clear();
		for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
		{
			M::CFace* pF = *fiter;
			std::vector<int> vertices_of_face;
			vertices_of_face.clear();
			for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
			{
				M::CVertex* pV = *fviter;
				vertices_of_face.push_back(pV->idx());

			}


			std::sort(vertices_of_face.begin(), vertices_of_face.end());
			face_exist.push_back(vertices_of_face);
			if (pF->idx() > 0)
			{
				for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
				{
					M::CEdge* pE = *feiter;
					edges_faces[pE->idx()].push_back(pF);
				}
				continue;
			}
			if (boundary_shorten == false)
			{
				for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
				{
					M::CEdge* pE = *feiter;
					edges_faces[pE->idx()].push_back(pF);
				}
			}
			pF->idx() = fid++;
			idx_faces.push_back(pF);
			m_inner_faces.insert(pF);
		}
	}
	/*!
	*   pair the interior simplices 
	*/
	void CHandleTunnelLoop::interior_volume_pair()
	{
		if (!exterior_volume)
		{
			return;
		}
		clock_t b = clock();
		_pair(m_inner_vertices);
		_pair(m_inner_edges);
		_pair(m_inner_faces);

		std::cout << "\nAfter pairing the Interior Volume :" << std::endl;
		std::vector<M::CEdge*> handle_loops;
		std::vector<M::CEdge*> tunnel_loops;
		// these are all edges that are generators but haven't been killed by the boundary
		for (auto eiter = m_generators.begin(); eiter != m_generators.end(); eiter++)
		{
			M::CEdge* pE = *eiter;
			if (pE->generator() && pE->pair() == NULL)
			{
				//generator not killed = tunnel loop
				std::cout << "Generator Edge " << pE->idx() << std::endl;
				//_mark_loop(pE);
				if (!exterior_tunnel)
				{
					tunnel_loops.push_back(pE);//----------------------------------------------turn back on to get tunnel loops from the interior volume---------------

				}
			}
			else if (pE->generator() && pE->pair() != NULL)
			{
				// generator that has been killed = handle loop
				std::cout << "Handle Loop Edge " << pE->idx() << std::endl;
				if (exterior_tunnel)
				{
					handle_loops.push_back(pE); //----------------------------------------------------------------------turn back on-------------------------------------
				}//pE->sharp() = true;
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
			current_generator = m_handle_gens[i];
			_mark_loop(handle_loop_edges[i]);
			
			//_mark_loop(handle_loops[i], handle_loops[i]->pair());
			//_mark_loop(handle_loops[i]);
			handletunnel_edges.push_back(handle_loops[i]);
			//M::CFace* pF = handle_loops[i]->pair();
			//_mark_loop(pF);
		}
		for (size_t i = 0; i < tunnel_loops.size(); i++)
		{
			_mark_loop(tunnel_loops[i]);
			handletunnel_edges.push_back(tunnel_loops[i]);
			//M::CFace* pF = handle_loops[i]->pair();
			//_mark_loop(pF);
		}


	}

	/*!
	*   pair simplices on the boundary surface
	*/
	void CHandleTunnelLoop::boundary_surface_pair()
	{
		if (!exterior_volume)
		{
			return;
		}
		_pair(m_boundary_vertices);
		_pair(m_boundary_edges);
		_pair(m_boundary_faces);
		
		std::cout << "After Pairing the boundary surface: " << std::endl;
		for (auto eiter = m_boundary_edges.begin(); eiter != m_boundary_edges.end(); eiter++)
		{
			M::CEdge* pE = *eiter;
			if (pE->generator() && pE->pair() == NULL)
			{
				//std::cout << "Generator Edge that has not been killed " << pE->idx() << std::endl;
				m_generators.insert(pE);
				for (auto generator_loop : generated_edge_loops)
				{
					if (generator_loop.first == pE)
					{
						for (auto pE2 : m_boundary_edges)
						{
							pE2->sharp() = false;
							pE2->green() = false;
						}
						for (auto pE2 : m_inner_edges)
						{
							pE2->sharp() = false;
							pE2->green() = false;
						}
						loop_edges.clear();
						for (M::CEdge* edg : generator_loop.second)
						{
							if (std::find(loop_edges.begin(), loop_edges.end(), edg) == loop_edges.end())
							{
								loop_edges.push_back(edg);
								edg->sharp() = true;
							}
						}
						//std::cout << "pruning\n";
						//prune();
						unkilled_generated_edge_loops.push_back({ pE, loop_edges});
						break;
					}
				}
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
		std::cout << "we started with " << edges.size() << " edges\n";
		int generator_edges = 0;
		int killer_edges = 0;
		std::cout << "starting edge pairing";
		double assign = 0;
		double add_pairing = 0;
		double head_pairing = 0;
		int largest_number = 0;
		int largest_edges_used = 0;
		int edges_used = 0;
		int counting = 0;
		std::vector<int> edges_indices_list;
		for (auto eiter = edges.begin(); eiter != edges.end(); eiter++)
		{
			M::CEdge* pE = *eiter;
			edges_indices_list.push_back(pE->idx());
		}
		std::sort(edges_indices_list.begin(), edges_indices_list.end());
		for (int edge_index : edges_indices_list)
		{
			M::CEdge* pE = idx_edges[edge_index];
			//std::cout << edge_index << " ";
			std::set<M::CEdge*> generated_loop;
			counting += 1;
			if (counting % 1000 == 0)
			{
				std::cout << ".";
			}
			generated_loop.insert(pE);
			if (edges_used > largest_edges_used)
			{
				largest_edges_used = edges_used;
			}
			edges_used = 0;
			int number = 0;
			inSet.clear();
			M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
			inSet.insert(pV->idx());
			number += 1;
			inSet.insert(pW->idx());
			number += 1;
			pV = idx_verts[*inSet.rbegin()];
			while (number > 0 && pV->pair() != NULL)
			{
				edges_used += 1;
				if (number > largest_number)
				{
					largest_number = number;
				}
				M::CEdge* pE2 = pV->pair();
				for (int vert : edgesPair[pE2])
				{
					if (inSet.find(vert) != inSet.end())
					{
						inSet.erase(vert);
						number -= 1;
					}
					else
					{
						inSet.insert(vert);
						number += 1;
					}
				}
				if (generated_loop.find(pE2) != generated_loop.end())
				{
					generated_loop.erase(pE2);
				}
				else
				{
					generated_loop.insert(pE2);
				}
				/*
				M::CVertex* pV2 = m_pMesh->edge_vertex(pE2, 0);
				M::CVertex* pW2 = m_pMesh->edge_vertex(pE2, 1);

				if (inSet.find(pV2->idx()) != inSet.end())
				{
					inSet.erase(pV2->idx());
					number -= 1;
				}
				else
				{
					inSet.insert(pV2->idx());
					number += 1;
				}
				if (inSet.find(pW2->idx()) != inSet.end())
				{
					inSet.erase(pW2->idx());
					number -= 1;
				}
				else
				{
					inSet.insert(pW2->idx());
					number += 1;
				}*/
				if (*inSet.rbegin())
				{
					pV = idx_verts[*inSet.rbegin()];
				}
			}
			if (number > 0)
			{
				killer_edges += 1;
				pV->pair() = pE;
				// insert what pE kills;
				edgesPair.insert({ pE, inSet });
				pE->generator() = false;
			}
			else
			{
				if (number < 0)
				{
					std::cout << "number is less than zero???\n";
				}
				if (number != inSet.size())
				{
					std::cout << "they are a different size, for whatever reason.\n";
				}
				generator_edges += 1;
				pE->generator() = true;
				generated_edge_loops.push_back({ pE, generated_loop });
			}

		}
		std::cout << "\nfinished with edges, there are " << killer_edges << " killers and " << generator_edges << " generator edges.\n";

	};

	/*!
	 *	pair faces
	 */
	void CHandleTunnelLoop::_pair(std::set<M::CFace*>& faces)
	{
		std::cout << "we started with " << faces.size() << " faces\n";
		int killer_faces = 0;
		int generator_faces = 0;
		double assign = 0;
		double add_time = 0;
		double head_time = 0;
		int counting = 0;
		std::vector<int> faces_indices_list;
		for (auto fiter = faces.begin(); fiter != faces.end(); fiter++)
		{
			M::CFace* pF = *fiter;
			faces_indices_list.push_back(pF->idx());
		}
		
		std::sort(faces_indices_list.begin(), faces_indices_list.end());
		for (int face_index : faces_indices_list)
		{
			M::CFace* pF = idx_faces[face_index];
			counting += 1;
			if (counting % 10000 == 0)
			{
				std::cout << "-";
			}
			
			int number = 0;
			int gnumber = 0;
			inSet.clear();
			inSetGens.clear();
			//Cycle<M::CEdge, Compare<M::CEdge>> ecycle;
			for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
			{
				M::CEdge* pEd = *feiter;
				inSet.insert(pEd->idx());
				number += 1;
				if (pEd->generator())
				{
					inSetGens.insert(pEd->idx());
					gnumber += 1;
				}
			}

			//std::cout << "We started with " <<  number << " " << gnumber << "\n";
			if (gnumber == 0)
			{
				std::cout << "oh no! gnumber is 0!\n";
			}
		
			M::CEdge* pE = idx_edges[*inSetGens.rbegin()];
			std::set<int> first_boundary_only;
			bool found = false;
			while(number > 0 && pE != NULL && pE->pair() != NULL)
			{
				M::CFace* pF2 = pE->pair();
				if (false/*std::find(killer_faces_list.begin(), killer_faces_list.end(), pF2) != killer_faces_list.end()*/)
				{
					inSet.erase(pE->idx());
				}
				else
				{
					for (int edge1 : facesPair[pF2])
					{
						if (inSet.find(edge1) != inSet.end())
						{
							inSet.erase(edge1);
							number -= 1;
						}
						else
						{
							inSet.insert(edge1);
							number += 1;
						}
						if (idx_edges[edge1]->generator())
						{
							if (inSetGens.find(edge1) != inSetGens.end())
							{
								inSetGens.erase(edge1);
								gnumber -= 1;
							}
							else
							{
								inSetGens.insert(edge1);
								gnumber += 1;
							}
						}
					}
				}
				/*for (M::FaceEdgeIterator feiter2(pF2); !feiter2.end(); ++feiter2)
				{
					M::CEdge* pE2 = *feiter2;

					if (inSet.find(pE2->idx()) != inSet.end())
					{
						inSet.erase(pE2->idx());
						number -= 1;
					}
					else
					{
						inSet.insert(pE2->idx());
						number += 1;
					}
					if (pE2->generator())
					{
						if (inSetGens.find(pE2->idx()) != inSetGens.end())
						{
							inSetGens.erase(pE2->idx());
							gnumber -= 1;
						}
						else
						{
							inSetGens.insert(pE2->idx());
							gnumber += 1;
						}
					}
				}*/
				/*bool boundary_only = true;
				if (found == false)
				{
					for (int edgeIn : inSet)
					{
						if (edgeIn > m_boundary_edges.size())
						{
							boundary_only = false;
							break;
						}
					}
				}
				if (boundary_only)
				{
					first_boundary_only = inSet;
					found = true;
				}*/

				if (gnumber > 0)
				{
					pE = idx_edges[*inSetGens.rbegin()];
				}
				else
				{
					pE = NULL;
				}
			
			
			}
			if (number > 0)
			{
				killer_faces += 1;
				pE->pair() = pF;
				pairing_information.push_back({ pE, pF });
				pairing_loop.push_back(inSet);
				pF->generator() = false;
				facesPair.insert({ pF, inSet });
				if (fastFacePairing)
				{
					if (m_generators.size() != 0)
					{
						if (std::find(m_generators.begin(), m_generators.end(), pE) != m_generators.end())
						{
							paired_generators += 1;
							m_handle_gens.push_back(pE);
							std::cout << "|";
							handle_loop_edges.push_back(/*first_boundary_only*/inSet);
							killer_faces_list.push_back(pF);
							if (paired_generators == m_genus)
							{
								std::cout << "ended quicker!\n";
								return;
							}
						}

					}
				}
				
			}
			else
			{
				generator_faces += 1;
				pF->generator() = true;
				if (!fastFacePairing)
				{
					if (m_generators.size() != 0)
					{
						if (std::find(m_generators.begin(), m_generators.end(), pE) != m_generators.end())
						{
							paired_generators += 1;

							if (paired_generators == m_genus)
							{
								std::cout << "ended quicker2!\n";
								return;
							}
						}

					}
				}
			}
		}
		std::cout << "finished, there are " << killer_faces << " killers and " << generator_faces << " generator faces.\n";
		std::cout << "time taken to add edges was " << add_time << " seconds while head time was " << head_time << " seconds."
			<< "time taken to assign boolean to false was " << assign <<  " seconds \n";
		/*
		int numb = 0;
		for (auto fiter = faces.begin(); fiter != faces.end(); fiter++)
		{
			M::CFace* pF = *fiter;
			numb += 1;
			if (numb % 1000 == 0)
			{
				std::cout << "-";
			}

			Cycle<M::CEdge, Compare<M::CEdge>> ecycle;

			for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
			{
				M::CEdge* pE = *feiter;
				ecycle.add(pE);
			}

			M::CEdge* phead = ecycle.head();
			while (!ecycle.empty() && phead->pair() != NULL)
			{
				M::CFace* _pF = phead->pair();
				// ecycle.print();
				for (M::FaceEdgeIterator feiter(_pF); !feiter.end(); ++feiter)
				{
					M::CEdge* pE = *feiter;
					ecycle.add(pE);
				}
				phead = ecycle.head();
			}

			if (!ecycle.empty())
			{
				// pe is a killer
				pF->generator() = false;
				phead->pair() = pF;

				if (m_generators.size() != m_genus * 2)
					continue;
				if (m_generators.find(phead) == m_generators.end())
					continue;
				int paired_generators = 0;
				for (auto eiter = m_generators.begin(); eiter != m_generators.end(); eiter++)
				{
					M::CEdge* pE = *eiter;
					if (pE->pair() != NULL)
						paired_generators++;
				}
				if (paired_generators == m_genus)
					return;
			}
			else
			{
				pF->generator() = true;
			}
		}*/
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
	void CHandleTunnelLoop::_mark_loop(M::CEdge* edge, M::CFace* face)
	{
		std::cout << "The loop started with " << edge->idx() << " " << face->idx() << "\n";
		loop_edges.clear();
		current_loop_edges.clear();
		loop_vertices.clear();
		std::set<M::CFace*> section;
		section.clear();

		if (m_inner_faces.find(face) != m_inner_faces.end())
		{
			std::cout << "added the original face\n";
			section.insert(face);
		}

		Cycle<M::CEdge, Compare<M::CEdge>> ecycle;
		for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
		{
			M::CEdge* pE = *feiter;
			ecycle.add(pE);
		}

		M::CEdge* phead = ecycle.head();
		while (!ecycle.empty() && phead != NULL && phead->pair() != NULL)
		{
			M::CFace* pF = phead->pair();

			if (m_inner_faces.find(pF) != m_inner_faces.end())
				section.insert(pF);
			for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
			{
				M::CEdge* pE = *feiter;
				ecycle.add(pE);
			}
			phead = ecycle.head();

		if (phead == edge)
			{
				break;
			}
		}


		/*int number = 0;
		int gnumber = 0;
		inSet.clear();
		inSetGens.clear();
		//Cycle<M::CEdge, Compare<M::CEdge>> ecycle;
		for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
		{
			M::CEdge* pEd = *feiter;
			inSet.insert(pEd->idx());
			number += 1;
			if (pEd->generator())
			{
				inSetGens.insert(pEd->idx());
				gnumber += 1;
			}
		}
		//std::cout << "We started with " <<  number << " " << gnumber << "\n";
		if (gnumber == 0)
		{
			std::cout << "oh no! gnumber is 0!\n";
		}

		M::CEdge* pE = idx_edges[*inSetGens.rbegin()];

		while (number > 0 && pE != NULL && pE->pair() != NULL)
		{
			M::CFace* pF2 = pE->pair();
			for (M::FaceEdgeIterator feiter2(pF2); !feiter2.end(); ++feiter2)
			{
				M::CEdge* pE2 = *feiter2;
				if (m_inner_faces.find(pF2) != m_inner_faces.end())
					section.insert(pF2);
				if (inSet.find(pE2->idx()) != inSet.end())
				{
					inSet.erase(pE2->idx());
					number -= 1;
				}
				else
				{
					inSet.insert(pE2->idx());
					number += 1;
				}
				if (pE2->generator())
				{
					if (inSetGens.find(pE2->idx()) != inSetGens.end())
					{
						inSetGens.erase(pE2->idx());
						gnumber -= 1;
					}
					else
					{
						inSetGens.insert(pE2->idx());
						gnumber += 1;
					}
				}

			}
			if (gnumber > 0)
			{
				pE = idx_edges[*inSetGens.rbegin()];
			}
			else
			{
				break;
				pE = NULL;
			}
		}*/











		std::cout << "There are " << section.size() << " faces" << std::endl;
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;

		}
		for (auto fiter = section.begin(); fiter != section.end(); fiter++)
		{
			M::CFace* pF = *fiter;
			for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
			{
				M::CEdge* pE = *feiter;
				// if (m_edges.find(pwe) != m_edges.end())
				//loop_edges.push_back(pE);
				//pE->sharp() = true;
				
				if (m_boundary_edges.find(pE) != m_boundary_edges.end())
				{
					if (std::find(loop_edges.begin(), loop_edges.end(), pE) == loop_edges.end())
					{
						loop_edges.push_back(pE);
						pE->sharp() = true;
					}
				}
			}
		}
		if (false)
		{
			loop_edges.clear();
			for (auto pE : m_boundary_edges)
			{
				pE->sharp() = false;
			}
			for (int ei : facesPair[face])
			{
				loop_edges.push_back(idx_edges[ei]);
				idx_edges[ei]->sharp() = true;
			}
			before_prune_edges.push_back(loop_edges);
		}
		if (false)
		{
			loop_edges.clear();
			for (auto pE : m_boundary_edges)
			{
				pE->sharp() = false;
			}
		}
		
		prune();
		
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		before_edges.push_back(loop_edges);
		
		std::cout << "begining shortening.\n";
		shorten();
	};

	void CHandleTunnelLoop::_mark_loop(std::set<int> hLoop)
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		current_loop_edges.clear();
		loop_vertices.clear();
		loop_edges.clear();
		for (int ei : hLoop)
		{
			idx_edges[ei]->sharp();
			loop_edges.push_back(idx_edges[ei]);
		}
		prune();
		before_edges.push_back(loop_edges);
		//return; //comment this out to get the shortened.
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		

		std::cout << "begining shortening.\n";
		//shorten();
	}
	void CHandleTunnelLoop::_mark_loop(M::CEdge* pE)
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		std::vector<M::CEdge*> one_loop;
		one_loop.push_back(pE);
		loop_edges.clear();
		loop_vertices.clear();
		loop_edges.push_back(pE);
		pE->sharp() = true;
		inSet.clear();
		int number = 0;
		//Cycle<M::CVertex, Compare<M::CVertex>> vcycle;
		//start from youngest positive vertex (all vertices are positive)
		M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
		M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
		number += 2;
		inSet.insert(pV->idx());
		inSet.insert(pW->idx());
		pV = idx_verts[*inSet.rbegin()];
		while (number > 0 && pV->pair())
		{
			M::CEdge* pE2 = pV->pair();
			one_loop.push_back(pE2);
			loop_edges.push_back(pE2);
			pE2->sharp() = true;

			M::CVertex* pV2 = m_pMesh->edge_vertex(pE2, 0);
			M::CVertex* pW2 = m_pMesh->edge_vertex(pE2, 1);
			if (inSet.find(pV2->idx()) != inSet.end())
			{
				inSet.erase(pV2->idx());
				number -= 1;
			}
			else
			{
				inSet.insert(pV2->idx());
				number += 1;
			}
			if (inSet.find(pW2->idx()) != inSet.end())
			{
				inSet.erase(pW2->idx());
				number -= 1;
			}
			else
			{
				inSet.insert(pW2->idx());
				number += 1;
			}
			if (number > 0)
			{
				pV = idx_verts[*inSet.rbegin()];
			}
		}
		if (number > 0)
		{
			std::cout << "this is wrong, the edge should be a generator.\n";
		}
		/*pE->sharp() = true;
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
			one_loop.push_back(pE2);
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
			std::cout << "this is right\n";
			pE->generator() = true;
		}*/
		before_prune_edges.push_back(one_loop);
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


	void CHandleTunnelLoop::setExterior()
	{
		exterior_volume = true;

		exterior_tunnel = true;
		boundary_shorten = true;
	}
	void CHandleTunnelLoop::write_tets(const std::string& output)
	{
		std::cout << "starting to write the tetrahedra\n";
		std::vector<std::set<int>> allowed_sets;
		std::vector<CPoint> center_of_masss;
		std::vector<std::vector<CPoint>> loop_vertices_scaleds;
		std::map<int, std::vector<std::vector<int>>> vert_tets;
		std::vector<std::vector<int>> new_tets2;
		std::map<int, bool> vertex_used;
		std::map<int, bool> bound_point;
		std::set<int> used_vertices_list;
		std::vector<double> average_distances;
		std::vector<double> largest_distances;
		double scale_factor = 1;
		if (m_boundary_edges.size() < 100000)
		{
			std::cout << "we turned the scale factor to 1.5\n";
			scale_factor = 1.5;
		}
		for (int good_final_index = 0; good_final_index < good_final_vertices.size(); good_final_index++)
		{

			loop_vertices = good_final_vertices[good_final_index];
			center_of_mass[0] = 0.0;
			center_of_mass[1] = 0.0;
			center_of_mass[2] = 0.0;
			for (M::CVertex* vertex : loop_vertices)
			{
				center_of_mass += vertex->point();
			}
			center_of_mass = center_of_mass / double(loop_vertices.size());
			center_of_masss.push_back(center_of_mass);
			std::vector<CPoint> loop_vertices_scaled;
			for (M::CVertex* vertex : loop_vertices)
			{
				CPoint scaled_point = center_of_mass + (vertex->point() - center_of_mass) * scale_factor;
				loop_vertices_scaled.push_back(scaled_point);
			}
			loop_vertices_scaleds.push_back(loop_vertices_scaled);
			double distance;
			double average_distance = 0;
			double largest_distance = 0;
			for (M::CVertex* v : loop_vertices)
			{
				distance = (v->point() - center_of_mass).norm();
				average_distance += distance;
				if (distance > largest_distance)
				{
					largest_distance = distance;
				}

			}
			average_distance /= loop_vertices.size();
			average_distances.push_back(average_distance);
			largest_distances.push_back(largest_distance);
			double actual_average = average_distance;
		}
		std::cout << "computed the scaled loops\n";
		//loop through tets in _O, add to _2_I
		std::fstream is(file_name.substr(0, file_name.size() - 6) + "_O.t", std::fstream::in);
		//std::cout << "the exterior file is named " << file_name.substr(0, file_name.size() - 6) + "_O.t\n";
		char buffer[MAX_LINE];

		verts_O.clear();
		std::map<int, bool> allowed;



		std::map<std::string, bool> int_used;
			
		for (M::CVertex* bV : m_boundary_vertices)
		{
			int_used[bV->point().print2()] = true;
		}
			
		int CH_number = 0;
		std::set<int> allowed_set;
		std::set<int> on_loop;
		std::cout << "there were " << good_final_vertices.size() << " loops\n";
		while (!is.eof())
		{
			is.getline(buffer, MAX_LINE);
			std::string line(buffer);
			line = strutil::trim(line);
			strutil::Tokenizer stokenizer(line, " \r\n");

			stokenizer.nextToken();
			std::string token = stokenizer.getToken();
			
			if (token == "Vertex")
			{
				stokenizer.nextToken();
				int vindex = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				double p1 = std::stod(stokenizer.getToken());
				stokenizer.nextToken();
				double p2 = std::stod(stokenizer.getToken());
				stokenizer.nextToken();
				double p3 = std::stod(stokenizer.getToken());
				CPoint mypoint(p1, p2, p3);
				vertex_used[vindex] = false;
				bool al = false;
				if (int_used[mypoint.print2()])
				{
					//std::cout << "this is true\n";
					bound_point[vindex] = true;
				}
				verts_O.insert({ vindex, mypoint });
				std::vector<std::vector<int>> emp;
				vert_tets[vindex] = emp;
				// convex hull idea
				for (int good_final_index = 0; good_final_index < good_final_vertices.size(); good_final_index++)
				{
					center_of_mass = center_of_masss[good_final_index];
					std::vector<CPoint> loop_vertices_scaled = loop_vertices_scaleds[good_final_index];
					loop_vertices = good_final_vertices[good_final_index];
					double largest_distance = largest_distances[good_final_index];
					double average_distance = average_distances[good_final_index];
					if ((mypoint - center_of_mass).norm() < largest_distance * 1.5)
					{
						std::vector<CPoint> vectors_list;
						for (M::CVertex* v : loop_vertices)
						{
							if (v->point().print2() == mypoint.print2())
							{
								al = true;
								if (loop_vertices.size() <= 5)
								{
									on_loop.insert(vindex);
								}
								break;
							}
						}
						for (CPoint v : loop_vertices_scaled)
						{
							vectors_list.push_back((v - mypoint) / (v - mypoint).norm());
						}
						if (al == false)
						{
							al = true;
							CH_number += 1;
							bool finish = false;
							for (int i = 0; i < vectors_list.size(); i++)
							{
								if (finish)
								{
									break;
								}
								for (int j = i + 1; j < vectors_list.size(); j++)
								{
									if (finish)
									{
										break;
									}
									for (double sign : {-1.0, 1.0})
									{
										CPoint CrPr = (vectors_list[i] ^ vectors_list[j]) / (vectors_list[i] ^ vectors_list[j]).norm() * sign;
										bool same_hemi = true;
										for (int k = 0; k < vectors_list.size(); k++)
										{
											if (k == i || k == j)
											{
												continue;
											}
											double theta = acos(vectors_list[k] * CrPr) * 180.0 / 3.1415926;
											if (theta >= 90.0) // lower the number to allow more points
											{
												same_hemi = false;
												break;
											}
										}
										if (same_hemi)
										{
											al = false;
											CH_number -= 1;
											finish = true;

											break;
										}
									}
								}
							}
						}
					}
					if (al == true)
					{
						allowed_set.insert(vindex);
						break;
					}
					//allowed.insert({ vindex, al });
				}
			}
			if (token == "Tet")
			{
				allowed_sets.push_back(allowed_set);
				break;
			}

		}
		is.close();
		std::cout << "starting to find all the tets\n";
		std::fstream is3(file_name.substr(0, file_name.size() - 6) + "_O.t", std::fstream::in);
		//std::cout << "the exterior file is named " << file_name.substr(0, file_name.size() - 6) + "_O.t\n";
		char buffer3[MAX_LINE];
		while (!is3.eof())
		{
			is3.getline(buffer3, MAX_LINE);
			std::string line(buffer3);
			line = strutil::trim(line);
			strutil::Tokenizer stokenizer(line, " \r\n");

			stokenizer.nextToken();
			std::string token = stokenizer.getToken();
			//std::cout << "a\n";
			if (token == "Tet")
			{
				stokenizer.nextToken();
				int tindex = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v1 = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v2 = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v3 = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v4 = std::stoi(stokenizer.getToken());
				vert_tets[v1].push_back({ v1,v2,v3,v4 });
				vert_tets[v2].push_back({ v1,v2,v3,v4 });
				vert_tets[v3].push_back({ v1,v2,v3,v4 });
				vert_tets[v4].push_back({ v1,v2,v3,v4 });
				for (int good_final_index = 0; good_final_index < good_final_vertices.size(); good_final_index++)
				{
					//std::cout << good_final_index;
					//std::set<int> allowed_set = allowed_sets[good_final_index];
					loop_vertices = good_final_vertices[good_final_index];

					int inside = 0;
					int in_loop = 0;
					if (allowed_set.find(v1) != allowed_set.end())
					{
						inside += 1;
					}
					if (allowed_set.find(v2) != allowed_set.end())
					{
						inside += 1;
					}
					if (allowed_set.find(v3) != allowed_set.end())
					{
						inside += 1;
					}
					if (allowed_set.find(v4) != allowed_set.end())
					{
						inside += 1;
					}
					if (on_loop.find(v1) != on_loop.end())
					{
						in_loop += 1;
					}
					if (on_loop.find(v2) != on_loop.end())
					{
						in_loop += 1;
					}
					if (on_loop.find(v3) != on_loop.end())
					{
						in_loop += 1;
					}
					if (on_loop.find(v4) != on_loop.end())
					{
						in_loop += 1;
					}
					//std::cout << "inside is " << inside;
					if (inside >= 4 || in_loop >= 3 || (inside >= 2 && m_boundary_edges.size() < 100000))
					{
						std::vector<int> new_tet;
						new_tet.push_back(v1);
						new_tet.push_back(v2);
						new_tet.push_back(v3);
						new_tet.push_back(v4);
						new_tets2.push_back(new_tet);
						vertex_used[v1] = true;
						vertex_used[v2] = true;
						vertex_used[v3] = true;
						vertex_used[v4] = true;
						used_vertices_list.insert(v1);
						used_vertices_list.insert(v2);
						used_vertices_list.insert(v3);
						used_vertices_list.insert(v4);
						//std::cout << "found a correct tet!\n";
						break;

					}
					
				}
				all_tets.push_back({ v1, v2, v3, v4 });
			}
		}
		is3.close();
		//std::cout << "there were " << CH_number << " points in the convex hull of this loop\n";
		std::cout << "filling in the tet gaps\n";
		bool fill_gaps = true;
		if (fill_gaps)
		{
			for (int vert : used_vertices_list) // use a map from point to tet, only check tets that touch a point that was used.
			{
				for (auto tet : vert_tets[vert])
				{
					int num_used = 0;
					int bound_points = 0;
					for (int i : tet)
					{
						if (bound_point[i])
						{
							bound_points += 1;
						}
						if (vertex_used[i])
						{
							num_used += 1;
						}
					}
					if (num_used == 4)
					{

						std::vector<int> new_tet;
						for (int vi : tet)
						{
							new_tet.push_back(vi);
						}
						if (std::find(new_tets2.begin(), new_tets2.end(), new_tet) == new_tets2.end())
						{
							//std::cout << "filled one in\n";
							new_tets2.push_back(new_tet);
						}
					}
					else if (num_used >= 2 && bound_points + num_used >= 4)
					{
						std::vector<int> new_tet;
						for (int vi : tet)
						{
							new_tet.push_back(vi);
						}
						if (std::find(new_tets2.begin(), new_tets2.end(), new_tet) == new_tets2.end())
						{
							//std::cout << "fixed a gap with the boundary\n";
							for (int vi : tet)
							{
								vertex_used[vi] = true;
								//used_vertices_list.insert(vi);
							}
							new_tets2.push_back(new_tet);
						}
					}
				}
			}

			std::cout << "there were " << new_tets2.size() << " new tets to be added\n";
			if (new_tets2.size() < 10000000000)// automatically true now // lower after testing is done
			{
				for (auto te : new_tets2)
				{
					new_tets.push_back(te);
				}
			}
			else
			{
				std::cout << "we didn't add the new tets of this component, because there were " << new_tets2.size() << " tets\n";
			}
			//new_tets2.clear();

		}

		// go through all tets one more time
		std::cout << "in total, we had " << new_tets.size() << " new tets in this loop\n";

		std::cout << "beginning to add the tets\n";
		

		std::fstream is2(output, std::fstream::in);
		char buffer2[MAX_LINE];
		old_tets.clear();
		std::unordered_map<std::string, int> verts_I;
		int tet_index = 0;
		int vert_index = 0;
		std::vector<std::string> vertex_lines;
		std::vector<std::string> tet_lines;
		std::vector<std::vector<int>> already_in;
		while (!is2.eof())
		{
			is2.getline(buffer2, MAX_LINE);
			std::string line(buffer2);
			line = strutil::trim(line);
			strutil::Tokenizer stokenizer(line, " \r\n");

			stokenizer.nextToken();
			std::string token = stokenizer.getToken();

			if (token == "Vertex")
			{
				stokenizer.nextToken();
				int vindex = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				double p1 = std::stod(stokenizer.getToken());
				stokenizer.nextToken();
				double p2 = std::stod(stokenizer.getToken());
				stokenizer.nextToken();
				double p3 = std::stod(stokenizer.getToken());
				CPoint mypoint(p1, p2, p3);
				verts_I.insert({ mypoint.print2(), vindex });
				vert_index += 1;
				vertex_lines.push_back(line);
			}
			else if (token == "Tet")
			{
				stokenizer.nextToken();
				int tindex = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v1 = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v2 = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v3 = std::stoi(stokenizer.getToken());
				stokenizer.nextToken();
				int v4 = std::stoi(stokenizer.getToken());
				std::vector<int> numbers;
				numbers.push_back(v1);
				numbers.push_back(v2);
				numbers.push_back(v3);
				numbers.push_back(v4);
				std::sort(numbers.begin(), numbers.end());
				already_in.push_back(numbers);
				tet_index += 1;
				tet_lines.push_back(line);
			}
		}

		//new_tets.clear(); // to generate the pre-surgery mesh.
		is2.close();
		std::fstream _os(output, std::fstream::out);

		if (_os.fail())
		{
			fprintf(stderr, "Error is opening file %s\n", output);
			return;
		}
		std::cout << "beginning to write the new verts and tets\n";
		for (std::string line : vertex_lines)
		{
			_os << line << "\n";
		}
		for (auto new_tet : new_tets)
		{
			for (int vert : new_tet)
			{
				CPoint point = verts_O[vert];
				if (verts_I.find(point.print2()) == verts_I.end())
				{
					vert_index++;
					verts_I.insert({ point.print2(), vert_index });
					_os << "Vertex " << vert_index << " " << point[0] << " " << point[1] << " " << point[2] << "\n";
				}
			}

		}
		for (std::string line : tet_lines)
		{
			_os << line << "\n";
		}

		for (auto new_tet : new_tets)
		{
			std::vector<int> numbers;
			for (int vert : new_tet)
			{
				CPoint point = verts_O[vert];
				numbers.push_back(verts_I[point.print2()]);
			}
			std::sort(numbers.begin(), numbers.end());
			if (std::find(already_in.begin(), already_in.end(), numbers) == already_in.end())
			{
				already_in.push_back(numbers);
				tet_index += 1;
				_os << "Tet " << tet_index;
				for (int vert : new_tet)
				{
					CPoint point = verts_O[vert];
					_os << " " << verts_I[point.print2()];
				}
				_os << "\n";
			}
			else
			{
				//std::cout << "we tried to add a tet that was already in there!--------------------~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~======================\n";
			}

		}

		_os.close();
	}
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
	void CHandleTunnelLoop::write_boundary()
	{
		std::fstream _os(file_name.substr(0, file_name.size() - 6) + ".obj", std::fstream::out);
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
			_os << "v " << p[0] << " " << p[1] << " " << p[2] << "\n";
		}

		for (auto pF : surface)
		{
			_os << "f";
			for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
			{
				M::CVertex* pV = *fviter;
				_os << " " << pV->idx();
			}
			_os << "\n";
		}
		_os.close();
	}
	void CHandleTunnelLoop::add_tunnel_old(std::string line)
	{
		int count = 0;
		std::istringstream iss(line);
		double u1, u2, u3;
		double w1, w2, w3;
		double v1, v2, v3;
		std::vector<M::CEdge*> middle_edge;
		std::vector<M::CVertex*> middle_vert;
		while (iss >> u1)
		{
			iss >> u2;
			iss >> u3;
			iss >> w1;
			iss >> w2;
			iss >> w3;
			iss >> v1;
			iss >> v2;
			iss >> v3;
			CPoint hU(u1, u2, u3);
			CPoint hV(v1, v2, v3);
			CPoint hW(w1, w2, w3);
			//middle_vert.push_back(idx_verts[pointVertex[hU.print()]]);
			//for (auto pV : m_boundary_vertices)
			//{
			//	if (pV->point().is_same(hU))
			//	{
			//		middle_vert.push_back(pV);
			//		break;
			//	}
			//}
			
			//std::cout << hV.print() + hW.print() << "\n";
			//std::cout << pointsEdge[hV.print() + hW.print()] << "\n";
			if (idx_edges[pointsEdge[hV.print() + hW.print()]] == NULL)
			{
				std::cout << "this one was null, before we even started!!@#$%^&*()!@#$%^&*()!@#$%^&*()\n";
				//return;
			}
			else
			{
				middle_edge.push_back(idx_edges[pointsEdge[hV.print() + hW.print()]]);
				count += 1;
				//std::cout << "edge number " << count << " is " << idx_edges[pointsEdge[hV.print() + hW.print()]] << "\n";
				
			}
			/*for (auto pE : m_boundary_edges)
			{
				M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
				M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
				CPoint tV = pV->point();
				CPoint tW = pW->point();
				if ((tV.is_same(hV) && tW.is_same(hW)) || (tV.is_same(hW) && tW.is_same(hV)))
				{
					middle_edge.push_back(pE);
					break;
				}
			}*/
		}
		
		middle_vertices.push_back(middle_vert);
		middle_edges.push_back(middle_edge);
		return;
	}
	void CHandleTunnelLoop::add_shortened_tunnel(std::string line)
	{
		int count = 0;
		std::istringstream iss(line);
		double u1, u2, u3;
		double v1, v2, v3;
		std::vector<M::CEdge*> middle_edge;
		while (iss >> u1)
		{
			iss >> u2;
			iss >> u3;
			iss >> v1;
			iss >> v2;
			iss >> v3;
			CPoint hU(u1, u2, u3);
			CPoint hV(v1, v2, v3);
			if (idx_edges[pointsEdge[hU.print() + hV.print()]] == NULL)
			{
				std::cout << "this one was null\n";
				//return;
			}
			else
			{
				middle_edge.push_back(idx_edges[pointsEdge[hU.print() + hV.print()]]);
				count += 1;
				//std::cout << "edge number " << count << " is " << idx_edges[pointsEdge[hV.print() + hW.print()]] << "\n";

			}
		}
		after_edges.push_back(middle_edge);
		std::cout << "we added the edge_loop\n";
		return;
	}
	void CHandleTunnelLoop::add_tunnel(std::string line)
	{
		int count = 0;
		std::istringstream iss(line);
		double u1, u2, u3;
		double v1, v2, v3;
		std::vector<M::CEdge*> middle_edge;
		while (iss >> u1)
		{
			iss >> u2;
			iss >> u3;
			iss >> v1;
			iss >> v2;
			iss >> v3;
			CPoint hU(u1, u2, u3);
			CPoint hV(v1, v2, v3);
			
			if (idx_edges[pointsEdge[hU.print() + hV.print()]] == NULL)
			{
				std::cout << "this one was null\n";
				//return;
			}
			else
			{
				count += 1;
				middle_edge.push_back(idx_edges[pointsEdge[hU.print() + hV.print()]]);
				//std::cout << "edge number " << count << " is " << idx_edges[pointsEdge[hV.print() + hW.print()]] << "\n";

			}
		}
		middle_edges.push_back(middle_edge);
		std::cout << "we added the edge_loop\n";
		return;
	}
	void CHandleTunnelLoop::write_tunnels(const std::string& output)
	{
		std::fstream _os(output, std::fstream::out);

		if (_os.fail())
		{
			fprintf(stderr, "Error is opening file %s\n", output);
			return;
		}
		std::vector<int> traversal_order;
		for (int i = 0; i < before_edges.size(); i++)
		{
			traversal_order.push_back(0);
		}
		for (int i = 0; i < before_edges.size(); i++)
		{
			int rank = 0;
			for (int j = 0; j <before_edges.size(); j++)
			{
				if (before_edges[i].size() > before_edges[j].size() || (before_edges[i].size() == before_edges[j].size() && i > j))
				{
					rank++;
				}
			}
			traversal_order[rank] = i;
		}
		
		for (int i : traversal_order)
		{
			std::vector<M::CEdge*> before_edge = before_edges[i];
			std::cout << "writing unshortened tunnel loop, size is " << before_edge.size() << "\n";
			M::CVertex* pV = m_pMesh->edge_vertex(m_handle_gens[i], 0);
			M::CVertex* pW = m_pMesh->edge_vertex(m_handle_gens[i], 1);
			_os << pV->point().print2() << " " << pW->point().print2() << " ";
			for (int j = 0; j < before_edge.size(); j ++)
			{
				M::CEdge* edge = before_edge[j];
				if (edge->idx() == m_handle_gens[i]->idx())
				{
					continue;
				}
				M::CVertex* pV = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* pW = m_pMesh->edge_vertex(edge, 1);
				_os << pV->point().print2() << " " << pW->point().print2() << " ";
				
			}
			_os << "\n";
		}
	}
	void CHandleTunnelLoop::write_shortened_tunnels(const std::string& output)
	{
	
		if (first_time == true)
		{
			first_time = false;
			std::ofstream _os(output, std::ofstream::trunc);
			if (_os.fail())
			{
				fprintf(stderr, "Error is opening file %s\n", output);
				return;
			}
			std::vector<int> traversal_order;
			for (int i = 0; i < after_edges.size(); i++)
			{
				int j = 0;
				while (j < traversal_order.size())
				{
					if (after_edges[i].size() > after_edges[j].size())
					{
						j++;
					}
					else
					{
						break;
					}
				}
				if (j == traversal_order.size())
				{
					traversal_order.push_back(i);
				}
				else
				{
					traversal_order.insert(traversal_order.begin() + j, i);
				}
			}

			for (int i : traversal_order)
			{
				std::vector<M::CEdge*> after_edge = after_edges[i];
				for (int j = 0; j < after_edge.size(); j++)
				{
					M::CEdge* edge = after_edge[j];
					M::CVertex* pV = m_pMesh->edge_vertex(edge, 0);
					M::CVertex* pW = m_pMesh->edge_vertex(edge, 1);
					_os << pV->point().print2() << " " << pW->point().print2() << " ";

				}
				_os << "\n";
			}
			_os.close();
			after_edges.clear();
		}
		else
		{
			std::ofstream _os(output, std::ofstream::app);
			if (_os.fail())
			{
				fprintf(stderr, "Error is opening file %s\n", output);
				return;
			}
			std::vector<int> traversal_order;
			for (int i = 0; i < after_edges.size(); i++)
			{
				int j = 0;
				while (j < traversal_order.size())
				{
					if (after_edges[i].size() > after_edges[j].size())
					{
						j++;
					}
					else
					{
						break;
					}
				}
				if (j == traversal_order.size())
				{
					traversal_order.push_back(i);
				}
				else
				{
					traversal_order.insert(traversal_order.begin() + j, i);
				}
			}

			for (int i : traversal_order)
			{
				std::vector<M::CEdge*> after_edge = after_edges[i];
				for (int j = 0; j < after_edge.size(); j++)
				{
					M::CEdge* edge = after_edge[j];
					M::CVertex* pV = m_pMesh->edge_vertex(edge, 0);
					M::CVertex* pW = m_pMesh->edge_vertex(edge, 1);
					_os << pV->point().print2() << " " << pW->point().print2() << " ";

				}
				_os << "\n";
			}
			_os.close();
			after_edges.clear();
		}


	}
	void CHandleTunnelLoop::write_after_obj(const std::string& output)
	{
		std::fstream _os(output, std::fstream::out);

		if (_os.fail())
		{
			fprintf(stderr, "Error is opening file %s\n", output);
			return;
		}

		//M::CBoundary boundary(m_pMesh);
		//const auto& surface = boundary.boundary_surface();

		/*std::set<M::CVertex*> vSet;
		for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
		{
			M::CFace* pF = *fiter;
			for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
			{
				M::CVertex* pV = *fviter;
				vSet.insert(pV);
			}
		}*/
		for (auto pV : idx_all_verts)
		{
			CPoint& p = pV->point();
			_os << "v " << p[0] << " " << p[1] << " " << p[2] << "\n";
		}
		/*for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
		{
			M::CFace* pF = *fiter;*/
			/*for (auto pF: m_boundary_faces)
			{
				_os << "f";
				for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
				{
					M::CVertex* pV = *fviter;
					_os << " " << pV->idx();
				}
				_os << "\n";
			}*/
		for (auto final_v : final_vertices)
		{
			/*int v = final_v[0];
			for (int i = 2; i < final_v.size(); i++)
			{
				_os << "f " << v << " " << final_v[i - 1] << " " << final_v[i] << "\n";
			}*/
			int larger = final_v.size() - 1;
			int smaller = 0;
			int next = smaller + 1;
			while (true)
			{
				if (next == larger)
				{
					break;
				}
				_os << "f " << final_v[smaller] << " " << final_v[larger] << " " << final_v[next] << "\n";
				smaller = next;
				next = larger - 1;
				if (next == smaller)
				{
					break;
				}
				_os << "f " << final_v[smaller] << " " << final_v[larger] << " " << final_v[next] << "\n";
				larger = next;
				next = smaller + 1;
			}
			_os << "\n\n";
		}
	}
	void CHandleTunnelLoop::write_good_after_obj(const std::string& output)
	{
		std::fstream _os(output, std::fstream::out);

		if (_os.fail())
		{
			fprintf(stderr, "Error is opening file %s\n", output);
			return;
		}

		//M::CBoundary boundary(m_pMesh);
		//const auto& surface = boundary.boundary_surface();

		/*std::set<M::CVertex*> vSet;
		for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
		{
			M::CFace* pF = *fiter;
			for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
			{
				M::CVertex* pV = *fviter;
				vSet.insert(pV);
			}
		}*/
		for (auto pV : idx_all_verts)
		{
			CPoint& p = pV->point();
			_os << "v " << p[0] << " " << p[1] << " " << p[2] << "\n";
		}
		/*for (M::FaceIterator fiter(m_pMesh); !fiter.end(); fiter++)
		{
			M::CFace* pF = *fiter;*/
			/*for (auto pF: m_boundary_faces)
			{
				_os << "f";
				for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
				{
					M::CVertex* pV = *fviter;
					_os << " " << pV->idx();
				}
				_os << "\n";
			}*/
		for (auto final_v : good_final_vertices)
		{
			/*int v = final_v[0];
			for (int i = 2; i < final_v.size(); i++)
			{
				_os << "f " << v << " " << final_v[i - 1] << " " << final_v[i] << "\n";
			}*/
			int larger = final_v.size() - 1;
			int smaller = 0;
			int next = smaller + 1;
			while (true)
			{
				if (next == larger)
				{
					break;
				}
				_os << "f " << final_v[smaller] << " " << final_v[larger] << " " << final_v[next] << "\n";
				smaller = next;
				next = larger - 1;
				if (next == smaller)
				{
					break;
				}
				_os << "f " << final_v[smaller] << " " << final_v[larger] << " " << final_v[next] << "\n";
				larger = next;
				next = smaller + 1;
			}
			_os << "\n\n";
		}
	}
	void CHandleTunnelLoop::write_before_obj(const std::string& output)
	{
		std::cout << "there were " << before_vertices.size() << " middle loops\n";
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
			_os << "v " << p[0] << " " << p[1] << " " << p[2] << "\n";
		}
		for (auto before_v : before_vertices)
		{
			/*int v = final_v[0];
			for (int i = 2; i < final_v.size(); i++)
			{
				_os << "f " << v << " " << final_v[i - 1] << " " << final_v[i] << "\n";
			}*/
			int larger = before_v.size() - 1;
			int smaller = 0;
			int next = smaller + 1;
			while (true)
			{
				if (next == larger)
				{
					break;
				}
				_os << "f " << before_v[smaller] << " " << before_v[larger] << " " << before_v[next] << "\n";
				smaller = next;
				next = larger - 1;
				if (next == smaller)
				{
					break;
				}
				_os << "f " << before_v[smaller] << " " << before_v[larger] << " " << before_v[next] << "\n";
				larger = next;
				next = smaller + 1;
			}
			_os << "\n\n";
		}
	}

	void CHandleTunnelLoop::write_before_ply(const std::string& output)
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
		_os << "ply\nformat ascii 1.0\n";
		_os << "element vertex " << vSet.size() << "\n";
		_os << "property float x\nproperty float y\nproperty float z\n";
		_os << "element face " << before_vertices.size() << "\n";
		_os << "property list uchar int vertex_index\nend_header\n";
		for (auto pV : vSet)
		{
			CPoint& p = pV->point();
			_os << p[0] << " " << p[1] << " " << p[2] << "\n";
		}

		for (auto before_v : before_vertices)
		{
			_os << before_v.size();
			for (int i : before_v)
			{
				_os << " " << i - 1;
			}
			_os << "\n";
		}
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



	void CHandleTunnelLoop::_shorten3()
	{
		/*std::vector<M::CVertex*> old_loop_vertices = loop_vertices;
		int start_vertices = 3 * loop_vertices.size() / 4 + 1;
		std::vector<int> search_verts;
		old_dist = DBL_MAX;
		new_dist = 0;
		int num_vertices = start_vertices;
		before_edges_search_size.push_back(start_vertices + 1);
		while (true new_dist == 0 || new_loop.size() < 4 || (new_loop.size() < num_vertices && num_vertices <= 6))
		{
			start_vertices += 1;
			num_vertices = start_vertices;
			std::cout << "retry==================================, searching with " << num_vertices << " vertices\n";
			search_verts.clear();
			new_loop.clear();
			old_dist = DBL_MAX;
			new_dist = 0;
			if (num_vertices > loop_vertices.size() || loop_vertices.size() == 4)
			{
				std::cout << "*************************************************************************** too many vertices were searched for ****************************************";
				std::cout << "num_vertices is " << num_vertices << " and there are " << loop_vertices.size() << " vertices in the loop.\n";
				for (auto vertex : loop_vertices)
				{
					new_loop.push_back(vertex->idx());
				}
				std::vector<M::CEdge*> f_edges;

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
							f_edges.push_back(pE);
						}
					}
				}
				for (auto pE : m_inner_edges)
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
							f_edges.push_back(pE);
						}
					}
				}
				final_vertices.push_back(new_loop);
				final_edges.push_back(f_edges);
				break;
			}

			for (int i = 0; i < num_vertices; i++)
			{
				search_verts.push_back(old_loop_vertices[(i * old_loop_vertices.size()) / num_vertices]->idx());
			}
			for (int i = 0; i < num_vertices; i++)
			{
				std::vector<int> path = dijkstra(search_verts[i % num_vertices], search_verts[(i + 1) % num_vertices]);
			}
			std::cout << "searching with " << search_verts.size() << " verts\n";
			std::cout << "we have " << new_loop.size() << " verts in the loop\n";

			while (new_loop.size() > num_vertices)
			{
				if (num_vertices >= 5)
				{
					num_vertices  = (num_vertices * 4) / 5;
				}
				search_verts.clear();
				old_dist = new_dist;
				new_dist = 0;
				for (int i = 0; i < num_vertices; i++)
				{
					search_verts.push_back(new_loop[(((i + 1/2)* new_loop.size()) / num_vertices) % new_loop.size()]);
				}
				new_loop.clear();
				for (int i = 0; i < num_vertices; i++)
				{
					std::vector<int> path = dijkstra(search_verts[i % num_vertices], search_verts[(i + 1) % num_vertices]);
				}
				if (old_dist <= new_dist || new_loop.size() <= num_vertices )
				{
					break;

				}
				std::cout << "old length is " << old_dist << " while the new distance is, " << new_dist << "\n";

				std::cout << "there are " << new_loop.size() << " vertices\n\n";

			}
			std::cout << "final distance was " << new_dist << "\n";

			remove_dup(new_loop);
			std::cout << "new loop has size: " << new_loop.size() << " while we started with " << num_vertices << " vertices\n";

			if (new_loop.size() > num_vertices && new_loop.size() >= 4)
			{
				std::vector<M::CEdge*> f_edges;
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
							f_edges.push_back(pE);
						}
					}
				}
				for (auto pE : m_inner_edges)
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
							f_edges.push_back(pE);
						}
					}
				}
				final_vertices.push_back(new_loop);
				final_edges.push_back(f_edges);
				break;
			}

			//std::cout << "the length of one random edge is  " << (m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->point() - m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->point()).norm() << "\n";
			//std::cout << "this should be the samve as above " << graph[m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->idx()][m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->idx()] << "\n";
			//std::cout << "also should be the samve as above " << graph[m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->idx()][m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->idx()] << "\n";
			//std::cout << "the first point was " << m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 0)->point().print() << "\n";
			//std::cout << "the second point is " << m_pMesh->edge_vertex(final_edges[final_edges.size() - 1], 1)->point().print() << "\n";

		}
		*/
		
		bool correct = false;
		int num_vertices;
		if (loop_vertices.size() > 600)
		{
			num_vertices = loop_vertices.size() / 150; //temporary
		}
		else if (loop_vertices.size() > 300)
		{
			num_vertices = loop_vertices.size() / 75; //temporary
		}
		else if (loop_vertices.size() > 200)
		{
			num_vertices = loop_vertices.size() / 50;
		}
		else if (loop_vertices.size() > 150)
		{
			num_vertices = loop_vertices.size() / 5;
		}
		else if (loop_vertices.size() > 100)
		{
			num_vertices = loop_vertices.size() / 3;
		}
		else if (loop_vertices.size() > 50)
		{
			num_vertices = loop_vertices.size() / 2;
		}
		else if (loop_vertices.size() > 10)
		{
			num_vertices = 2 * loop_vertices.size() / 3;
		}
		else
		{
			num_vertices = 3 * loop_vertices.size() / 4;
		}
		int starting_vertices = num_vertices;
		std::vector<int> starting_loop;
		for (auto vertex : loop_vertices)
		{
			new_loop.push_back(vertex->idx());
			starting_loop.push_back(vertex->idx());
		}
		old_dist = 0;
		// find the current distance of the starting loop
		for (int i = 0; i < loop_vertices.size() - 1; i++)
		{
			for (auto p : adj[loop_vertices[i]->idx()])
			{
				if (p.first == loop_vertices[i + 1]->idx())
				{
					old_dist += p.second;
					break;
				}
			}
		}
		for (auto p : adj[loop_vertices[loop_vertices.size() - 1]->idx()])
		{
			if (p.first == loop_vertices[0]->idx())
			{
				old_dist += p.second;
				break;
			}
		}
		double starting_distance = old_dist;

		double best_final = 100000.0;
		std::vector<int> best_final_loop;
		for (int counter = 0; counter < starting_loop.size(); counter++)
		{
			MVERT = 4;
			old_dist = starting_distance;
			new_dist = 0;
			starting_loop.push_back(starting_loop[0]);
			starting_loop.erase(starting_loop.begin());
			new_loop = starting_loop;
			std::vector<int> search_verts;
			int shift = 0;
			before_edges_search_size.push_back(num_vertices);
			std::vector<int> old_loop = new_loop;
			std::cout << "counter is " << counter << "\n";
			num_vertices = starting_vertices;
			double best = old_dist;
			std::vector<int> best_loop = old_loop;
			while (num_vertices >= MVERT)
			{
				bool dec = true;
				shift = 0;

				old_loop = new_loop;
				best_loop = old_loop;
				//int best_idx = -1;
				best = old_dist;
				//std::cout << "old loop has size " << old_loop.size() << "\n";
				int shift_amount = old_loop.size() - 2 - ((num_vertices - 3) * (old_loop.size() - 2) / (num_vertices - 2));
				//std::cout << "old loop has size " << old_loop.size() << " shift_amount is " << shift_amount << " while we search with " << num_vertices << " vertices\n";
				while (shift < shift_amount)
				{
					new_loop = old_loop;
					search_verts.clear();
					search_verts.push_back(new_loop[0]);
					search_verts.push_back(new_loop[1]);
					for (int i = 0; i < (num_vertices - 2); i++)
					{
						search_verts.push_back(new_loop[((i * (new_loop.size() - 2)) / (num_vertices - 2) + shift + 2) % new_loop.size()]);
					}
					new_loop.clear();
					ignore_these.clear();
					new_dist = 0;
					for (int i = 0; i < num_vertices; i++)
					{
						std::vector<int> path = dijkstra(search_verts[i % num_vertices], search_verts[(i + 1) % num_vertices]);
					}
					std::vector<int> temp = remove_dup(new_loop);
					/*if (new_loop.size() <= 8)
					{
						std::cout << "\nused the following to search: ";
						for (int inte : search_verts)
						{
							std::cout << inte << " ";
						}
						std::cout << "\ngot the distance " << new_dist << "\n";
						std::cout << "and we found the loop ";
						for (int inte : new_loop)
						{
							std::cout << inte << " ";
						}
						std::cout << "\n\n";
					}*/
					if (temp != new_loop)
					{
						/*if (new_loop.size() <= 8)
						{
							std::cout << "two loops\n";
							for (int inte : new_loop)
							{
								std::cout << inte << " ";
							}
							std::cout << "\n";
							for (int inte2 : temp)
							{
								std::cout << inte2 << " ";
							}
							std::cout << "\nshift " << shift << "\n";
						}*/

						new_dist = 0;
						new_loop = old_loop;

						shift += 1;
						continue;
					}
					//std::cout << "last2\n";
					std::vector<int> ol = old_loop;
					std::vector<int> nl = new_loop;
					std::sort(ol.begin(), ol.end());
					std::sort(nl.begin(), nl.end());
					//std::cout << "last3\n";
					if (new_dist < best /*|| ol != nl*/)
					{
						if (new_loop.size() <= 10)
						{
							/*std::cout << "new loop is ";
							for (int inte : new_loop)
							{
								std::cout << inte << " ";
							}
							std::cout << "\n";*/
						}
						best = new_dist;
						//best_idx = shift;
						best_loop = new_loop;
					}
					new_dist = 0;
					new_loop = old_loop;
					shift += 1;
					//std::cout << "last4 " << num_vertices <<"\n"; 
					/*if (old_dist - new_dist >= 0.0 && ol != nl)
					{
						old_dist = new_dist;
						new_dist = 0;
						if (num_vertices == 3)
						{
							dec = false;
						}
						break;
					}
					else
					{
						new_dist = 0;
						new_loop = old_loop;
					}*/
				}
				if (best_loop.size() <= 6)
				{
					MVERT = 3;
				}
				if (best < old_dist)
				{
					old_dist = best;
					old_loop = best_loop;
					new_dist = old_dist;
					new_loop = old_loop;
				}
				else
				{
					/*if (num_vertices == 3)
					{
						dec = false;
					}*/
					/*if (dec)
					{*/
					num_vertices = std::min(int(new_loop.size()), num_vertices - 1);
					//}
				}

			}

			//std::cout << "new loop has size " << new_loop.size() << "\n";
			if (new_loop.size() <= MVERT)
			{
				//std::cout << "resetted it\n";
				new_loop = starting_loop;
				continue;
			}
			else if (new_loop.size() <= 20)
			{
				int n = new_loop.size();
				int counter = 0;
				std::vector<int> every_four;
				for (int i = 0; i < n; i++)
				{
					every_four.clear();
					for (int j = 0; j < 4; j++)
					{
						every_four.push_back(new_loop[(i + j) % n]);
					}
					for (int k = 0; k < 4; k++) // which one is left out
					{
						std::vector<int> three_verts;
						for (int l = 0; l < 4; l++)
						{
							if (l != k)
							{
								three_verts.push_back(every_four[l]);
							}
						}
						std::sort(three_verts.begin(), three_verts.end());
						if (std::find(face_exist.begin(), face_exist.end(), three_verts) != face_exist.end())
						{
							counter += 1;
						}
					}
				}
				if (counter > n / 2)
				{
					//std::cout << "resetted it due to having faces\n";
					new_loop = starting_loop;
					continue;
				}
				else if (counter > n / 3 && n > 10)
				{
					//std::cout << "resetted it due to having faces\n";
					new_loop = starting_loop;
					continue;
				}
			}

				correct = true;

			//std::cout << "----------------------------------------------------------------------------------------------------length is " << best << "\n";
			if (best < best_final)
			{
				/*std::cout << "final answer is\n ";
				for (int inte : new_loop)
				{
					std::cout << inte << " ";
				}
				std::cout << "\n";
				for (int inte : best_loop)
				{
					std::cout << inte << " ";
				}
				std::cout << "\n";*/
				best_final = best;
				best_final_loop = best_loop;
			}
			
		}
		new_loop = best_final_loop;
		search_start_verts.push_back({ new_loop[0], new_loop[1] });
		/*if (!correct)
		{
			fails += 1;
			std::cout << "failed\n";
			new_loop = starting_loop;
		}*/
		std::vector<M::CEdge*> f_edges;
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
					f_edges.push_back(pE);
				}
			}
		}
		for (auto pE : m_inner_edges)
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
					f_edges.push_back(pE);
				}
			}
		}
		std::cout << "------------------------------------------------- " << f_edges.size() << " edges in this answer\n";

		if (new_loop == starting_loop)
		{
			final_vertices.push_back(new_loop);
			final_edges.push_back(f_edges);
		}
		else
		{
			//good_final_vertices.push_back(new_loop);
			good_final_edges.push_back(f_edges);
		}


	}
	void CHandleTunnelLoop::_shorten2()
	{
		bool printS = false;
		single_to_double.clear();
		double_to_single.clear();
		fall_back = current_loop_edges;
		center_of_mass[0] = 0.0;
		center_of_mass[1] = 0.0;
		center_of_mass[2] = 0.0;
		for (M::CVertex* v : loop_vertices)
		{
			//std::cout << v->idx() << " " << v->point().print() << " ";
			//std::cout << v->idx() << " " ;
			//std::cout << v->point().print() << "\n";
			center_of_mass += v->point();
			//std::cout << "center of mass is " << center_of_mass.print() << "\n";
		}
		std::cout << "we started with " << loop_vertices.size() << "vertices\n";
		center_of_mass /= double(loop_vertices.size());
		std::cout << "center of mass was " << center_of_mass.print() << "\n";
		//std::cout << center_of_mass.print() << " ";
		int tester = 0;
		bool failed_previously = false;
		bool correct = false;
		bool change_happened = false;

		while (tester < 100) //50000
		{
			change_happened = false;
			tester += 1;
			//std::cout << "the index is " << index;
			//std::cout << " vert size, edge size " << loop_vertices.size() << " " << current_loop_edges.size() << "\n";
			//^ allows repeating moves
			while (true)
			{
				for (auto e : current_loop_edges)
				{
					if (e == NULL)
					{
						std::cout << "we just got a null edge now!====================================================================\n";
					}
				}
				green_edges.clear();
				if (index >= current_loop_edges.size())
				{
					index = 0;
					break;
				}
				if (index < 0)
				{
					index = (index + current_loop_edges.size()) % current_loop_edges.size();
				}
				current_edge_to_green = current_loop_edges[index];
				green_edges.push_back(current_edge_to_green);
				//std::cout << "\n\n";
				if (_repeats((index + current_loop_edges.size() - 2) % current_loop_edges.size()))
				{
					if (printS)
					{
						std::cout << "removed double spike in past\n";
					}
					//std::cout << "we removed a double spike in previous edge!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
					index -= 2;
					change_happened = true;
					continue;
				}
				if (_repeats(index))
				{
					if (printS)
					{
						std::cout << "removed double spike\n";
					}
					//std::cout << "we removed a double spike!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
					change_happened = true;
					continue;
				}
				if (_repeats((index + 1) % current_loop_edges.size()))
				{
					if (printS)
					{
						std::cout << "removed double spike in future\n";
					}
					//std::cout << "we removed a double spike in the future!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
					change_happened = true;
					continue;
				}
				if (_fill_gaps(index))
				{
					index += 2;
					if (printS)
					{
						std::cout << "we filled a gap\n";
					}
					change_happened = true;
					//std::cout << "new index is " << index << ", out of " << loop_vertices.size() << "\n";
					continue;
				}
				if (_repeats2((index + 1) % current_loop_edges.size()))
				{
					if (printS)
					{
						std::cout << "first spike from repeats 2\n";
					}
					//std::cout << "we removed a spike in the future!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
					change_happened = true;
					continue;
				}
				if (_repeats2(index))
				{
					if (printS)
					{
						std::cout << "spike from repeats2\n";
					}
					//std::cout << "we removed a spike now!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
					change_happened = true;
					continue;
				}
				if (_triple0(index))
				{
					change_happened = true;
					if (printS)
					{
						std::cout << "triple0 removed\n";
					}
					continue;
				}
				if (_triple1(index))
				{
					change_happened = true;
					if (printS)
					{
						std::cout << "triple1 removed\n";
					}
					index -= 1;
					continue;
				}
				if (_triple2(index))
				{
					change_happened = true;
					if (printS)
					{
						std::cout << "triple2 removed\n";
					}
					index -= 2;
					continue;
				}
				if (_double(index))
				{
					if (printS)
					{
						std::cout << "double\n";
					}
					change_happened = true;
					_change_double();
					continue;
				}
				if (_single(index))
				{
					change_happened = true;
					index += 1;
					if (skip_singles)
					{
						//std::cout << "single got skipped\n";
						skip_singles = false;
						continue;
					}
					if (printS)
					{
						std::cout << "single\n";
					}

					_change_single();

					skip_singles = true;
					continue;
				}
				index += 1;
			}
			if (change_happened == false)
			{
				correct = true;
				//std::cout << "ended naturally, after " << tester << " tries.\n";
				break;
			}

			/*best_edge_o1s.clear();
			best_edge_o2s.clear();
			best_face_os.clear();
			idx_best_edge_o1s.clear();
			idx_best_edge_o2s.clear();
			best_edge_os.clear();
			idx_best_edge_os.clear();
			tester++;
			//check all pairs of consecutive edges if they share a face.
			// check doubles;
			//std::cout << "the center of mass is1 " << center_of_mass.print() << "\n";
			bool keep_go = _delete_triple();

			if (keep_go)
			{
				//std::cout << "deleted a face\n";
				display_loop(current_loop_edges);
				continue;
			}

			//std::cout << "the center of mass is2 " << center_of_mass.print() << "\n";
			double best_doub = _check_double();

			//change one single
			double best_sing = _check_single();
			//std::cout << "the distances are: " << best_doub << " and " << best_sing << "\n";
			//std::cout << "the center of mass is3 " << center_of_mass.print() << "\n\n";
			// choose the worse that still helps!???

			/*if ((best_doub <= 0.0 && best_sing <= 0.0) != (best_face_os.size() == 0))
			{
				std::cout << "SOMETHING TERRIBLY WRONG HAS HAPPENED WITH THE FACES!===========================================\n";
			}*/
			/*
			if (best_doub <= 0.0 && best_sing <= 0.0)
			{
				correct = true;
				std::cout << "done right";
				break;
				std::cout << "we were here--------------------------------------------------------------------------------------------------------------------\n";
				// TRY SHORTENING ALL THE DOUBLES, AND TRY AGAIN.
				// FAIL TWICE = EXIT!
				// first try to use a double to shorten the loop, if that doesn't work, then break;
				std::cout << "gave it some help! THIS SERVES AS A JUMP START\n";
				single_to_double.clear();
				double_to_single.clear();
				failed_previously = true;
				bool go_on = _shorten_double();
				// shorten all the doubles
				/*while (go_on)
				{
					go_on = _shorten_double();
					std::cout << "shortened the loop as needed.||";
				}*/
				/*
				continue;
			}
			if (best_doub > 0 /*= best_sing || (best_sing - best_doub) < best_doub / 10.0/*best_sing*//*)
			{
				//std::cout << "d";
				_change_double();
			}
			else if (best_sing > 0 /*best_doub*//*)
			{
				//std::cout << "s";
				_change_single();
			}
			*/
			//std::cout << "center of mass is, after changing: " << center_of_mass.print() << "\n";
			//testing
			/*else if (best_doub < best_sing && best_doub > 0.0)
			{
				_change_double();
			}
			else if (best_sing < best_doub && best_sing > 0.0)
			{
				_change_single();
			}
			else if (best_doub > 0.0)
			{
				_change_double();
			}
			else if (best_sing > 0.0)
			{
				_change_single();
			}
			//end testing



			int chosen_one = rand() % best_face_os.size();
			if (chosen_one >= best_edge_o1s.size())
			{
				best_edge_o = best_edge_os[chosen_one - best_edge_o1s.size()];
				best_face_o = best_face_os[chosen_one];
				idx_best_edge_o = idx_best_edge_os[chosen_one - best_edge_o1s.size()];
				_change_single();
			}
			else
			{
				best_edge_o1 = best_edge_o1s[chosen_one];
				best_edge_o2 = best_edge_o2s[chosen_one];
				best_face_o = best_face_os[chosen_one];
				idx_best_edge_o1 = idx_best_edge_o1s[chosen_one];
				idx_best_edge_o2 = idx_best_edge_o2s[chosen_one];
				_change_double();
			}
			*/

		}
		//std::cout << "new center of mass is: " << center_of_mass.print() << "\n";
		//display_loop(current_loop_edges);

		std::vector<int> lv;
		for (M::CVertex* v : loop_vertices)
		{
			lv.push_back(v->idx());
		}
		std::cout << "center of mass is " << center_of_mass.print() << "\n";

		if (correct)
		{
			good_final_vertices.push_back(loop_vertices);
			good_final_edges.push_back(current_loop_edges);
		}
		else
		{
			final_vertices.push_back(lv);
			final_edges.push_back(current_loop_edges);
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
							loop_edges.erase(std::remove(loop_edges.begin(), loop_edges.end(), edges[i]), loop_edges.end());
						}
						else if (!edges[i]->sharp())
						{
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
				/*if ((to != t) && (std::find(ignore_these.begin(), ignore_these.end(), to) != ignore_these.end()))
				{
					continue;
				}*/
						
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
			//ignore_these.push_back(loc);
		}
		new_loop.insert(new_loop.end(), new_verts.begin(), new_verts.end());
		return new_verts;
	}

	std::vector<int> CHandleTunnelLoop::remove_dup(std::vector<int> v)
	{
		std::vector<int> n_vect;
		for (int i : v)
		{
			if (std::find(n_vect.begin(), n_vect.end(), i) != n_vect.end())
			{
				continue;
			}
			else
			{
				n_vect.push_back(i);
			}
		}
		return n_vect;
	}

	void CHandleTunnelLoop::display_all_before_prune()
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (auto f_edges : before_prune_edges)
		{
			for (auto pE : f_edges)
			{
				pE->sharp() = true;
			}
		}
	}
	void CHandleTunnelLoop::display_all_before()
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		std::cout << "there are " << before_edges.size() << " starting loops\n";
		std::cout << "there are " << before_edges_search_size.size() << " starting search loops\n";
		for (auto f_edges : before_edges)
		{
			for (auto pE : f_edges)
			{
				pE->sharp() = true;
			}
		}
	}
	void CHandleTunnelLoop::display_all_unshortened()
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (auto f_edges : unshortened_edges)
		{
			for (auto pE : f_edges)
			{
				pE->sharp() = true;
			}
		}
	}
	void CHandleTunnelLoop::display_all_middle()
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		std::cout << "there are " << middle_edges.size() << " middle loops\n";
		for (auto f_edges : middle_edges)
		{
			std::cout << "this middle_loop has size " << f_edges.size() << "\n";
			for (auto pE : f_edges)
			{
				pE->sharp() = true;
			}
		}
	}
	void CHandleTunnelLoop::display_all_after()
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (auto f_edges : after_edges)
		{
			for (auto pE : f_edges)
			{
				pE->sharp() = true;
			}
		}
	}
	void CHandleTunnelLoop::display_before(int which)
	{
		which = which % before_edges.size();
		std::cout << "displaying " << which << " of " << before_edges.size() << " before loops. It has size " << before_edges[which].size() << "\n";
		//std::cout << "This loop was shortened starting with " << before_edges_search_size[which] << "\n";
		//std::cout << before_edges[which].size() << " started with this many edges\n";
		//green_edges.clear();
		//for (M::CEdge* ed : m_handle_gens)
		//{
		//	if (std::find(before_edges[which].begin(), before_edges[which].end(), ed) != before_edges[which].end())
		//	{
		//		green_edges.push_back(ed);
		//	}
		//}
		//std::cout << green_edges.size();
		display_loop(before_edges/*middle_edges*/[which]);
		/*for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : before_edges[which])
		{
			pE->sharp() = true;
		}*/
	}
	void CHandleTunnelLoop::display_unshortened(int which)
	{
		which = which % unshortened_edges.size();
		display_loop(unshortened_edges[which]);
	}
	void CHandleTunnelLoop::display_middle(int which)
	{
		which = which % middle_edges.size();
		std::cout << "displaying " << which << " of " << middle_edges.size() << " middle loops. It has size " << middle_edges[which].size() << "\n";
		
		display_loop(middle_edges[which]);
	}
	void CHandleTunnelLoop::display_tested(int which)
	{
		which = which % tested_edges.size();
		std::cout << "displaying " << which << " of " << tested_edges.size() << " tested loops. It has size " << tested_edges[which].size() << "\n";
		display_loop(tested_edges[which]);
	}
	void CHandleTunnelLoop::display_after(int which)
	{
		which = which % after_edges.size();
		std::cout << after_edges[which].size() << " ended with this many edges\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : after_edges[which])
		{
			pE->sharp() = true;
		}
		std::cout << "\n";
	}



	void CHandleTunnelLoop::show_original()
	{
		std::cout << "original edges has " << handletunnel_edges.size() << " edges\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : handletunnel_edges)
		{
			pE->sharp() = true;
		}
	}
	void CHandleTunnelLoop::show_starting(int which)
	{
		which = which % search_start_verts.size();
		display_loop(search_start_verts[which]);
	
		/*for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : search_start_edges)
		{
			pE->sharp() = true;
		}*/
	}
	void CHandleTunnelLoop::display_individual(int which_edge)
	{
		which_edge = which_edge % current_loop_edges.size();
		std::cout << "which_edge is " << which_edge << "\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		current_loop_edges[which_edge]->sharp() = true;
	}
	void CHandleTunnelLoop::display_pair(int w)
	{
		w = w % pairing_information.size();
		std::cout << "displaying pair " << w << "\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		pairing_information[w].first->green() = true;
		for (M::FaceEdgeIterator feiter(pairing_information[w].second); !feiter.end(); ++feiter)
		{
			M::CEdge* pE = *feiter;
			//pE->sharp() = true;
		}
		for (int i : pairing_loop[w])
		{
			idx_edges[i]->sharp() = true;
		}
	}

	void CHandleTunnelLoop::display_correct_loop(int w)
	{
		
		w = w % middle_edges.size();
		std::cout << "Displaying " << w << " of " << middle_edges.size() << " cleaned loops. It has size " << middle_edges[w].size() << " edges\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		for (auto edgei : middle_edges[w])
		{
			edgei->sharp() = true;
		}
	}
	void CHandleTunnelLoop::display_generated_loop(int w)
	{
		w = w % unkilled_generated_edge_loops.size();
		std::cout << "displaying generated loop " << w << " with size " << unkilled_generated_edge_loops[w].second.size() << "\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		unkilled_generated_edge_loops[w].first->green() = true;
		for (M::CEdge* ed : unkilled_generated_edge_loops[w].second)
		{
			ed->sharp() = true;
		}
	}
	void CHandleTunnelLoop::display_loop(std::vector<int> loop)
	{
		std::cout << "this loop has " << loop.size() << " vertices\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		std::vector<M::CEdge*> loop_e;
		for (auto pE : m_boundary_edges)
		{
			int pV = m_pMesh->edge_vertex(pE, 0)->idx();
			int pW = m_pMesh->edge_vertex(pE, 1)->idx();
			std::vector<int>::iterator i1 = std::find(loop.begin(), loop.end(), pV);
			std::vector<int>::iterator i2 = std::find(loop.begin(), loop.end(), pW);
			if (i1 != loop.end() && i2 != loop.end())
			{
				if (std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == loop.size() - 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1 * loop.size() + 1)
				{
					loop_e.push_back(pE);
				}
			}
		}
		for (auto pE : m_inner_edges)
		{
			int pV = m_pMesh->edge_vertex(pE, 0)->idx();
			int pW = m_pMesh->edge_vertex(pE, 1)->idx();
			std::vector<int>::iterator i1 = std::find(loop.begin(), loop.end(), pV);
			std::vector<int>::iterator i2 = std::find(loop.begin(), loop.end(), pW);
			if (i1 != loop.end() && i2 != loop.end())
			{
				if (std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == loop.size() - 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1 * loop.size() + 1)
				{
					loop_e.push_back(pE);
				}
			}
		}
		double d = 0;
		for (auto pE : loop_e)
		{
			M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
			d += (pW->point() - pV->point()).norm();
			pE->sharp() = true;
		}
		std::cout << "loop length is " << d << "\n";
	}
	void CHandleTunnelLoop::display_edges(std::vector<int> loop)
	{
		std::cout << "this loop has " << loop.size() << " edges\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (int i : loop)
		{
			idx_edges[i]->sharp() = true;
		}
	}
	void CHandleTunnelLoop::display_loop(std::vector<M::CVertex*> l)
	{
		std::vector<int> loop;
		for (auto i : l)
		{
			loop.push_back(i->idx());
		}
		std::cout << "this loop has " << loop.size() << " vertices\n";
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		std::vector<M::CEdge*> loop_e;
		for (auto pE : m_boundary_edges)
		{
			int pV = m_pMesh->edge_vertex(pE, 0)->idx();
			int pW = m_pMesh->edge_vertex(pE, 1)->idx();
			std::vector<int>::iterator i1 = std::find(loop.begin(), loop.end(), pV);
			std::vector<int>::iterator i2 = std::find(loop.begin(), loop.end(), pW);
			if (i1 != loop.end() && i2 != loop.end())
			{
				if (std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == loop.size() - 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1 * loop.size() + 1)
				{
					loop_e.push_back(pE);
				}
			}
		}
		for (auto pE : m_inner_edges)
		{
			int pV = m_pMesh->edge_vertex(pE, 0)->idx();
			int pW = m_pMesh->edge_vertex(pE, 1)->idx();
			std::vector<int>::iterator i1 = std::find(loop.begin(), loop.end(), pV);
			std::vector<int>::iterator i2 = std::find(loop.begin(), loop.end(), pW);
			if (i1 != loop.end() && i2 != loop.end())
			{
				if (std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == loop.size() - 1
					|| std::distance(loop.begin(), i1) - std::distance(loop.begin(), i2) == -1 * loop.size() + 1)
				{
					loop_e.push_back(pE);
				}
			}
		}
		double d = 0;
		for (auto pE : loop_e)
		{
			M::CVertex* pV = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(pE, 1);
			d += (pW->point() - pV->point()).norm();
			pE->sharp() = true;
		}
		std::cout << "loop length is " << d << "\n";
	}
	void CHandleTunnelLoop::display_loop(std::vector<M::CEdge*> loop)
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
			pE->green() = false;
		}
		for (M::CEdge* ed : loop)
		{
			ed->sharp() = true;
		}
		for (M::CEdge* ed : green_edges)
		{
			ed->green() = true;
		}
		//current_edge_to_green->green() = true;
	}
	void CHandleTunnelLoop::display_single_loop()
	{
		for (auto pE : m_boundary_edges)
		{
			pE->sharp() = false;
		}
		for (auto pE : m_inner_edges)
		{
			pE->sharp() = false;
		}
		for (M::CEdge* ed : single_loop)
		{
			ed->sharp() = true;
		}
	}
	void CHandleTunnelLoop::next_shorten_step()
	{
		_shorten();
	}
	void CHandleTunnelLoop::go_back()
	{
		display_loop(fall_back);
	}
	void CHandleTunnelLoop::go_forward()
	{
		display_loop(loop_vertices);
	}
	void CHandleTunnelLoop::remove_unconnected()
	{
		std::vector<M::CEdge*> visited_edges;
		std::map<M::CVertex*, std::vector<M::CEdge*>> vertex_edges;
		std::vector<M::CVertex*> vertices_counter;
		vertices_counter.clear();
		for (auto edge : loop_edges)
		{
			M::CVertex* pV = m_pMesh->edge_vertex(edge, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(edge, 1);
			vertices_counter.push_back(pV);
			vertices_counter.push_back(pW);
			if (vertex_edges.find(pV) == vertex_edges.end())
			{
				vertex_edges.insert({ pV, {edge} });
			}
			else
			{
				vertex_edges[pV].push_back(edge);
			}
			if (vertex_edges.find(pW) == vertex_edges.end())
			{
				vertex_edges.insert({ pW, {edge} });
			}
			else
			{
				vertex_edges[pW].push_back(edge);
			}
		}
		M::CVertex* current_vertex = vertices_counter[0];
		int badIdx1 = 0;
		loop_vertices.push_back(current_vertex);
		M::CEdge* current_edge = vertex_edges[current_vertex][0];
		M::CVertex* next_vertex = current_vertex == m_pMesh->edge_vertex(current_edge, 0)
			? m_pMesh->edge_vertex(current_edge, 1) : m_pMesh->edge_vertex(current_edge, 0);
		current_loop_edges.push_back(current_edge);
		visited_edges.push_back(current_edge);
		while (true)
		{
			//std::cout << "the sizes are: " << loop_vertices.size() << " " << current_loop_edges.size() << "\n";
			/*std::cout << "\n";
			for (auto ver : loop_vertices)
			{
				std::cout << " " << ver->idx();
			}
			std::cout << "\n";*/
			if (std::find(loop_vertices.begin(), loop_vertices.end(), next_vertex) == loop_vertices.end())
			{
				loop_vertices.push_back(next_vertex);
				for (M::CEdge* edge : vertex_edges[next_vertex])
				{
					if (std::find(visited_edges.begin(), visited_edges.end(), edge) == visited_edges.end())
					{
						current_edge = edge;
						visited_edges.push_back(current_edge);
						current_loop_edges.push_back(current_edge);
						next_vertex = next_vertex == m_pMesh->edge_vertex(current_edge, 0)
							? m_pMesh->edge_vertex(current_edge, 1) : m_pMesh->edge_vertex(current_edge, 0);
						break;
					}
				}
			}
			else if (std::find(loop_vertices.begin(), loop_vertices.end(), next_vertex) != loop_vertices.end())
			{
				// we've visited this vertex already, check if null homologous
				//std::cout << "potential loop found! We found " << next_vertex->idx();
				int firstOccIndex = std::find(loop_vertices.begin(), loop_vertices.end(), next_vertex) - loop_vertices.begin();
				std::vector<M::CEdge*> edge_loop(current_loop_edges.begin() + firstOccIndex, current_loop_edges.end());
				if (_null_homologous(edge_loop))
				{
					// delete the edges up to the last triple that still has unvisited edges;
					M::CEdge* e;
					M::CVertex* v;
					while (true)
					{
						if (current_loop_edges.size() == 0)
						{
							std::cout << "current loop edges has size 0===================================\n";
							return;
						}
						e = current_loop_edges.back();
						current_loop_edges.pop_back();
						v = loop_vertices.back();
						loop_vertices.pop_back();
						//std::cout << "edge, vertex removed: " << e->idx() << " " << v->idx();
						bool found = false;
						for (M::CEdge* edge : vertex_edges[v])
						{
							if (std::find(visited_edges.begin(), visited_edges.end(), edge) == visited_edges.end())
							{
								loop_vertices.push_back(v);
								current_edge = edge;
								visited_edges.push_back(current_edge);
								current_loop_edges.push_back(current_edge);
								next_vertex = v == m_pMesh->edge_vertex(current_edge, 0)
									? m_pMesh->edge_vertex(current_edge, 1) : m_pMesh->edge_vertex(current_edge, 0);
								found = true;
								break;
							}
						}
						if (found)
						{
							break;
						}

					}
				}
				else
				{
					// this is correct!
					std::cout << "this is correct! \n";
					current_loop_edges = edge_loop;
					std::vector<M::CVertex*> vertices_loop(loop_vertices.begin() + firstOccIndex, loop_vertices.end());
					loop_vertices = vertices_loop;
					break;

				}
			}
		}
	}
	void CHandleTunnelLoop::start_shorten2()
	{
		for (int i = 0; i < middle_edges.size(); i++)
		{
			loop_vertices = middle_vertices[i];
			current_loop_edges = middle_edges[i];
			_shorten();
		}
	}
	void CHandleTunnelLoop::start_shorten()
	{

		
		loop_edges = middle_edges[0];
		std::cout << "the middle loop has size " << loop_edges.size() << "\n";
		current_loop_edges = middle_edges[0];
		middle_edges.clear();
		shorten();
		//new_tets.clear();

	}
	void CHandleTunnelLoop::shorten()
	{
		single_loop.clear();
		for (M::CEdge* edge : loop_edges)
		{
			single_loop.insert(edge);
		}
		int num_components = -1;
		int num_iterations = 500;
		trip = false;
		if (loop_edges.size() > 200)
		{
			num_iterations = 1000;
		}
		for (int iterations = 0; iterations < num_iterations; iterations++)
		{
			 _shorten_single();
		}
		trip = false;
		num_components = _shorten_single_final();

		if (num_components > 1)
		{
			std::cout << "========================================we accidentally ended with: " << num_components << " components\n";
		}

		if (false)
		{
			clock_t start = clock();
			bool cont = true;
			std::map<M::CVertex*, std::vector<M::CEdge*>> vertex_edges;
			vertex_edges.clear();
			std::vector<M::CVertex*> vertices_counter;
			vertices_counter.clear();
			for (auto edge : loop_edges)
			{
				M::CVertex* pV = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* pW = m_pMesh->edge_vertex(edge, 1);
				vertices_counter.push_back(pV);
				vertices_counter.push_back(pW);
				if (vertex_edges.find(pV) == vertex_edges.end())
				{
					vertex_edges.insert({ pV, {edge} });
				}
				else
				{
					vertex_edges[pV].push_back(edge);
				}
				if (vertex_edges.find(pW) == vertex_edges.end())
				{
					vertex_edges.insert({ pW, {edge} });
				}
				else
				{
					vertex_edges[pW].push_back(edge);
				}
			}
			std::vector<M::CVertex*> bad_vertices;
			bad_vertices.clear();
			for (M::CVertex* v : vertices_counter)
			{
				if (std::count(vertices_counter.begin(), vertices_counter.end(), v) >= 3)
				{
					if (std::find(bad_vertices.begin(), bad_vertices.end(), v) == bad_vertices.end())
					{
						bad_vertices.push_back(v);
					}
				}
			}
			loop_vertices.clear();
			current_loop_edges.clear();
			bool notDone = true;
			tested_edges.clear();
			tested_vertices.clear();
			std::vector<M::CEdge*> visited_edges;
			//std::cout << "these are the bad vertices: ";
			/*for (M::CVertex* bad_v : bad_vertices)
			{
				std::cout << bad_v->idx() << " ";
			}*/
			//M::CVertex* current_vertex = /*bad_vertices*/ vertices_counter[0];
			/*
			std::cout << "\n";
			for (auto edge : vertex_edges[current_vertex])
			{
				std::cout << "edge: " << edge->idx() << "\n";
			}*/
			int badIdx1 = 0;

			//M::CEdge* current_edge = vertex_edges[current_vertex][0];
			M::CEdge* current_edge = loop_edges[0];
			current_loop_edges.push_back(current_edge);
			visited_edges.push_back(current_edge);
			std::cout << "We started with edge " << current_edge->idx() << "\n";
			M::CVertex* current_vertex = m_pMesh->edge_vertex(current_edge, 0);
			loop_vertices.push_back(current_vertex);
			M::CVertex* next_vertex = m_pMesh->edge_vertex(current_edge, 1);
			//M::CVertex* next_vertex = current_vertex == m_pMesh->edge_vertex(current_edge, 0) ? m_pMesh->edge_vertex(current_edge, 1) : m_pMesh->edge_vertex(current_edge, 0);


			while (notDone)
			{
				//std::cout << "the sizes are: " << loop_vertices.size() << " " << current_loop_edges.size() << "\n";
				/*std::cout << "\n";
				for (auto ver : loop_vertices)
				{
					std::cout << " " << ver->idx();
				}
				std::cout << "\n";*/
				if (std::find(loop_vertices.begin(), loop_vertices.end(), next_vertex) == loop_vertices.end())
				{
					loop_vertices.push_back(next_vertex);
					for (M::CEdge* edge : vertex_edges[next_vertex])
					{
						if (std::find(visited_edges.begin(), visited_edges.end(), edge) == visited_edges.end())
						{
							current_edge = edge;
							visited_edges.push_back(current_edge);
							current_loop_edges.push_back(current_edge);
							next_vertex = next_vertex == m_pMesh->edge_vertex(current_edge, 0)
								? m_pMesh->edge_vertex(current_edge, 1) : m_pMesh->edge_vertex(current_edge, 0);
							break;
						}
					}
				}
				else if (std::find(loop_vertices.begin(), loop_vertices.end(), next_vertex) != loop_vertices.end())
				{
					// we've visited this vertex already, check if null homologous
					//std::cout << "potential loop found! We found " << next_vertex->idx();
					int firstOccIndex = std::find(loop_vertices.begin(), loop_vertices.end(), next_vertex) - loop_vertices.begin();
					std::vector<M::CEdge*> edge_loop(current_loop_edges.begin() + firstOccIndex, current_loop_edges.end());
					//tested_edges.push_back(edge_loop);
					std::vector<M::CVertex*> vertices_loop(loop_vertices.begin() + firstOccIndex, loop_vertices.end());
					//tested_vertices.push_back(vertices_loop);
					if (_shorten(edge_loop))//_null_homologous(edge_loop)
					{
						// delete the edges up to the last triple that still has unvisited edges;
						M::CEdge* e;
						M::CVertex* v;
						while (true)
						{
							if (current_loop_edges.size() == 0)
							{
								notDone = false;
								//std::cout << "current loop edges has size 0============================================================================================\n";
								for (M::CEdge* edge : loop_edges)
								{
									if (std::find(visited_edges.begin(), visited_edges.end(), edge) == visited_edges.end())
									{
										//std::cout << "trying again with a different starting edge.\n";
										current_edge = edge;
										current_loop_edges.push_back(current_edge);
										visited_edges.push_back(current_edge);
										current_vertex = m_pMesh->edge_vertex(current_edge, 0);
										loop_vertices.push_back(current_vertex);
										next_vertex = m_pMesh->edge_vertex(current_edge, 1);
										std::cout << "found a new start.\n";
										notDone = true;
										break;
									}
								}


								break;
							}
							e = current_loop_edges.back();
							current_loop_edges.pop_back();
							v = loop_vertices.back();
							loop_vertices.pop_back();
							//std::cout << "edge, vertex removed: " << e->idx() << " " << v->idx();
							bool found = false;
							for (M::CEdge* edge : vertex_edges[v])
							{
								if (std::find(visited_edges.begin(), visited_edges.end(), edge) == visited_edges.end())
								{
									loop_vertices.push_back(v);
									current_edge = edge;
									visited_edges.push_back(current_edge);
									current_loop_edges.push_back(current_edge);
									next_vertex = v == m_pMesh->edge_vertex(current_edge, 0)
										? m_pMesh->edge_vertex(current_edge, 1) : m_pMesh->edge_vertex(current_edge, 0);
									found = true;
									break;
								}
							}
							if (found)
							{
								break;
							}

						}
					}
					else
					{
						unshortened_edges.push_back(edge_loop);
						// this is correct!
						std::cout << "_________________________________________________________----this is correct! The cleaned loop had size " << edge_loop.size() << "\n";
						current_loop_edges = edge_loop;
						break;

					}
				}
			}

			if (exterior_volume == false)
			{
				std::cout << "finished one cleaning.\n";
				return;
			}
			middle_edges.push_back(current_loop_edges);
			middle_vertices.push_back(loop_vertices);
			std::vector<int> before_v;
			for (auto i : loop_vertices)
			{
				before_v.push_back(i->idx());
			}
			before_vertices.push_back(before_v);
			return;
			_shorten();
			//display_loop(current_loop_edges);
			clock_t end = clock();
			std::cout << "\nshorten time took " << double(end - start) / CLOCKS_PER_SEC << "==============\n";
		}
	}

	void CHandleTunnelLoop::find_connected_components()
	{
		for (auto loop : middle_edges)
		{
			std::vector<std::set<M::CEdge*>> components;
			std::unordered_map<int, std::vector<std::pair<M::CVertex*, M::CEdge*>>> gr;
			std::vector<std::pair<M::CVertex*, M::CEdge*>> empty_vect;
			std::set<M::CEdge*> visited_edges;
			std::set<M::CVertex*> visited_vertices;
			for (M::CEdge* edge : loop)
			{
				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				std::pair<M::CVertex*, M::CEdge*> p1(v1, edge);
				std::pair<M::CVertex*, M::CEdge*> p2(v2, edge);
				if (gr.find(v1->idx()) == gr.end())
				{
					gr[v1->idx()] = empty_vect;
				}
				if (gr.find(v2->idx()) == gr.end())
				{
					gr[v2->idx()] = empty_vect;
				}
				gr[v1->idx()].push_back(p2);
				gr[v2->idx()].push_back(p1);
			}
			for (M::CEdge* edge : loop)
			{
				if (visited_edges.find(edge) != visited_edges.end())
				{
					continue;
				}

				std::set<M::CEdge*> component;
				std::queue<M::CVertex*> q;
				q.push(m_pMesh->edge_vertex(edge, 0));
				while (!q.empty())
				{
					M::CVertex* vert = q.front();
					q.pop();
					visited_vertices.insert(vert);
					for (std::pair<M::CVertex*, M::CEdge*> p : gr[vert->idx()])
					{
						if (visited_edges.find(p.second) == visited_edges.end())
						{
							visited_edges.insert(p.second);
							component.insert(p.second);
							if (visited_vertices.find(p.first) == visited_vertices.end())
							{
								q.push(p.first);
							}
						}
					}
				}
				components.push_back(component);

			}
			std::set<M::CEdge*> largest_component;
			for (auto component : components)
			{
				if (component.size() > largest_component.size())
				{
					largest_component = component;
				}
			}
			//in the largest component, add faces with all 3 edges present.
			std::set<M::CFace*> insertFaces;
			for (M::CEdge* edge : largest_component)
			{
				for (M::CFace* face : edges_faces[edge->idx()])
				{
					bool all_in = true;
					for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
					{
						M::CEdge* face_edge = *feiter;
						if (largest_component.find(face_edge) == largest_component.end())
						{
							all_in = false;
							break;
						}
					}
					if (all_in)
					{
						if (insertFaces.find(face) == insertFaces.end())
						{
							insertFaces.insert(face);
						}
					}
				}
			}
			for (M::CFace* face : insertFaces)
			{
				for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
				{
					M::CEdge* edge = *feiter;
					if (largest_component.find(edge) == largest_component.end())
					{
						largest_component.insert(edge);
						std::cout << "this should be impossible\n";
					}
					else
					{
						largest_component.erase(edge);
					}
				}
			}
			std::vector<M::CEdge*> unshortened_loop;
			for (auto edge : largest_component)
			{
				unshortened_loop.push_back(edge);
			}
			unshortened_edges.push_back(unshortened_loop);
		}
	}
	void CHandleTunnelLoop::shorten_demo(int which)
	{
		which = which % middle_edges.size();
		if (which != shortening_index)
		{
			shortening_index = which;
			single_loop.clear();
			for (auto edge : middle_edges[which])
			{
				single_loop.insert(edge);
			}
		}
		_shorten_single();
		display_single_loop();
	}
	int CHandleTunnelLoop::_shorten_single()
	{
		std::set<M::CEdge*> loop;
		for (M::CEdge* edge : single_loop)
		{
			if (loop.find(edge) == loop.end())
			{
				loop.insert(edge);
			}
		}

		std::set<M::CFace*> insertFaces;
		std::set<M::CEdge*> prev_loop = loop;
		std::vector<std::set<M::CEdge*>> components;
		// pick the best face for each edge
		if (trip == false)
		{
			// find individual connected components, and have each  component shorten towards it's own center of mass.
			
			
			std::unordered_map<int, std::vector<std::pair<M::CVertex*, M::CEdge*>>> gr;
			std::vector<std::pair<M::CVertex*, M::CEdge*>> empty_vect;
			std::set<M::CEdge*> visited_edges;
			std::set<M::CVertex*> visited_vertices;
			for (M::CEdge* edge : loop)
			{
				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				std::pair<M::CVertex*, M::CEdge*> p1(v1, edge);
				std::pair<M::CVertex*, M::CEdge*> p2(v2, edge);
				if (gr.find(v1->idx()) == gr.end())
				{
					gr[v1->idx()] = empty_vect;
				}
				if (gr.find(v2->idx()) == gr.end())
				{
					gr[v2->idx()] = empty_vect;
				}
				gr[v1->idx()].push_back( p2 );
				gr[v2->idx()].push_back( p1 );
			}
			for (M::CEdge* edge : loop)
			{
				if (visited_edges.find(edge) != visited_edges.end())
				{
					continue;
				}
				
				std::set<M::CEdge*> component;
				std::queue<M::CVertex*> q;
				q.push(m_pMesh->edge_vertex(edge, 0));
				while (!q.empty())
				{
					M::CVertex* vert = q.front();
					q.pop();
					visited_vertices.insert(vert);
					for (std::pair<M::CVertex*, M::CEdge*> p : gr[vert->idx()])
					{
						if (visited_edges.find(p.second) == visited_edges.end())
						{
							visited_edges.insert(p.second);
							component.insert(p.second);
							if (visited_vertices.find(p.first) == visited_vertices.end())
							{
								q.push(p.first);
							}
						}
					}
				}
				components.push_back(component);

			}
			//std::cout << "we have " << components.size() << " components\n";
			for (std::set<M::CEdge*> component : components)
			{
				CPoint total_mass(0, 0, 0);
				center_of_mass[0] = 0.0;
				center_of_mass[1] = 0.0;
				center_of_mass[2] = 0.0;

				for (M::CEdge* edge : component)
				{

					M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
					M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
					total_mass += (v1->point() + v2->point()) / 2.0;

				}

				center_of_mass = total_mass / double(component.size());
				//std::cout << component.size() << " has a center of mass of " <<  center_of_mass.print() << "\n";
				std::set<M::CEdge*> skip_edges;
				if (component.size() < 3)
				{
					for (M::CEdge* edge : component)
					{
						loop.erase(edge);
					}
				}
				else
				{
					for (M::CEdge* edge : component)
					{
						/*if (skip_edges.find(edge) != skip_edges.end())
						{
							continue;
						}*/
						double closestDist = 9999;
						M::CFace* bestFace = NULL;
						for (M::CFace* face : edges_faces[edge->idx()])
						{
							//std::cout << "checking if we recently used the face\n";
							/*if (recently_used.find(face) != recently_used.end())
							{
								std::cout << "we did recently use the face\n";
								continue;
							}*/
							CPoint faceCOM(0, 0, 0);
							for (M::FaceVertexIterator fviter(face); !fviter.end(); ++fviter)
							{
								M::CVertex* vertex = *fviter;
								faceCOM += vertex->point();
							}
							faceCOM /= 3.0;

							if ((faceCOM - center_of_mass).norm() < closestDist)
							{
								closestDist = (faceCOM - center_of_mass).norm();
								bestFace = face;
							}
						}
						if (insertFaces.find(bestFace) == insertFaces.end())
						{
							insertFaces.insert(bestFace);
							for (M::FaceEdgeIterator feiter(bestFace); !feiter.end(); ++feiter)
							{
								M::CEdge* e = *feiter;
								/*if (skip_edges.find(e) == skip_edges.end())
								{
									skip_edges.insert(e);
								}*/
							}
						}
					}
				}
			}
			trip = true;
		}
		else if (trip == true)
		{
			for (M::CEdge* edge : loop)
			{
				for (M::CFace* face : edges_faces[edge->idx()])
				{
					bool all_in = true;
					for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
					{
						M::CEdge* face_edge = *feiter;
						if (loop.find(face_edge) == loop.end())
						{
							all_in = false;
							break;
						}
					}
					if (all_in)
					{
						if (insertFaces.find(face) == insertFaces.end())
						{
							insertFaces.insert(face);
						}
					}
				}
			}
			trip = false;
		}
		recently_used = insertFaces;
		// insert each of the faces, adding and removing edges
		for (M::CFace* face : insertFaces)
		{
			for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
			{
				M::CEdge* edge = *feiter;
				if (loop.find(edge) == loop.end())
				{
					loop.insert(edge);
					M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
					M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
					//total_mass += (v1->point() + v2->point()) / 2.0;
				}
				else
				{
					loop.erase(edge);
					M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
					M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
					//total_mass -= (v1->point() + v2->point()) / 2.0;
				}
			}
		}
		// before finishing, choose the best that is the lowest average distance to center of mass.
		single_loop = loop;
		return components.size();
		
	}
	int CHandleTunnelLoop::_shorten_single_final()
	{
		std::set<M::CEdge*> loop;
		for (M::CEdge* edge : single_loop)
		{
			if (loop.find(edge) == loop.end())
			{
				loop.insert(edge);
			}
		}

		std::set<M::CFace*> insertFaces;
		std::set<M::CEdge*> prev_loop = loop;
		std::vector<std::set<M::CEdge*>> components;
		// pick the best face for each edge


		std::unordered_map<int, std::vector<std::pair<M::CVertex*, M::CEdge*>>> gr;
		std::vector<std::pair<M::CVertex*, M::CEdge*>> empty_vect;
		std::set<M::CEdge*> visited_edges;
		std::set<M::CVertex*> visited_vertices;
		for (M::CEdge* edge : loop)
		{
			M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
			M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
			std::pair<M::CVertex*, M::CEdge*> p1(v1, edge);
			std::pair<M::CVertex*, M::CEdge*> p2(v2, edge);
			if (gr.find(v1->idx()) == gr.end())
			{
				gr[v1->idx()] = empty_vect;
			}
			if (gr.find(v2->idx()) == gr.end())
			{
				gr[v2->idx()] = empty_vect;
			}
			gr[v1->idx()].push_back(p2);
			gr[v2->idx()].push_back(p1);
		}
		for (M::CEdge* edge : loop)
		{
			if (visited_edges.find(edge) != visited_edges.end())
			{
				continue;
			}

			std::set<M::CEdge*> component;
			std::queue<M::CVertex*> q;
			q.push(m_pMesh->edge_vertex(edge, 0));
			while (!q.empty())
			{
				M::CVertex* vert = q.front();
				q.pop();
				visited_vertices.insert(vert);
				for (std::pair<M::CVertex*, M::CEdge*> p : gr[vert->idx()])
				{
					if (visited_edges.find(p.second) == visited_edges.end())
					{
						visited_edges.insert(p.second);
						component.insert(p.second);
						if (visited_vertices.find(p.first) == visited_vertices.end())
						{
							q.push(p.first);
						}
					}
				}
			}
			components.push_back(component);

		}
		//std::cout << "we have " << components.size() << " components\n";
		for (std::set<M::CEdge*> component : components)
		{
			CPoint total_mass(0, 0, 0);
			center_of_mass[0] = 0.0;
			center_of_mass[1] = 0.0;
			center_of_mass[2] = 0.0;

			for (M::CEdge* edge : component)
			{

				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				total_mass += (v1->point() + v2->point()) / 2.0;

			}

			center_of_mass = total_mass / double(component.size());
			if (component.size() < 3)
			{
				for (M::CEdge* edge : component)
				{
					loop.erase(edge);
				}
			}
			else
			{
				//std::cout << component.size() << " has a center of mass of " <<  center_of_mass.print() << "\n";
				for (M::CEdge* edge : component)
				{
					double closestDist = 9999;
					M::CFace* bestFace = NULL;
					for (M::CFace* face : edges_faces[edge->idx()])
					{
						CPoint faceCOM(0, 0, 0);
						for (M::FaceVertexIterator fviter(face); !fviter.end(); ++fviter)
						{
							M::CVertex* vertex = *fviter;
							faceCOM += vertex->point();
						}
						faceCOM /= 3.0;

						if ((faceCOM - center_of_mass).norm() < closestDist)
						{
							closestDist = (faceCOM - center_of_mass).norm();
							bestFace = face;
						}
					}
					if (insertFaces.find(bestFace) == insertFaces.end())
					{
						insertFaces.insert(bestFace);
					}
				}
			}
		}

		for (M::CFace* face : insertFaces)
		{
			for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
			{
				M::CEdge* edge = *feiter;
				if (loop.find(edge) == loop.end())
				{
					loop.insert(edge);
					M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
					M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
					//total_mass += (v1->point() + v2->point()) / 2.0;
				}
				else
				{
					loop.erase(edge);
					M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
					M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
					//total_mass -= (v1->point() + v2->point()) / 2.0;
				}
			}
		}
		
		// find components
		gr.clear();
		empty_vect.clear();
		visited_edges.clear();
		visited_vertices.clear();
		components.clear();
		for (M::CEdge* edge : loop)
		{
			M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
			M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
			std::pair<M::CVertex*, M::CEdge*> p1(v1, edge);
			std::pair<M::CVertex*, M::CEdge*> p2(v2, edge);
			if (gr.find(v1->idx()) == gr.end())
			{
				gr[v1->idx()] = empty_vect;
			}
			if (gr.find(v2->idx()) == gr.end())
			{
				gr[v2->idx()] = empty_vect;
			}
			gr[v1->idx()].push_back(p2);
			gr[v2->idx()].push_back(p1);
		}
		for (M::CEdge* edge : loop)
		{
			if (visited_edges.find(edge) != visited_edges.end())
			{
				continue;
			}

			std::set<M::CEdge*> component;
			std::queue<M::CVertex*> q;
			q.push(m_pMesh->edge_vertex(edge, 0));
			while (!q.empty())
			{
				M::CVertex* vert = q.front();
				q.pop();
				visited_vertices.insert(vert);
				for (std::pair<M::CVertex*, M::CEdge*> p : gr[vert->idx()])
				{
					if (visited_edges.find(p.second) == visited_edges.end())
					{
						visited_edges.insert(p.second);
						component.insert(p.second);
						if (visited_vertices.find(p.first) == visited_vertices.end())
						{
							q.push(p.first);
						}
					}
				}
			}
			components.push_back(component);

		}



		bool remove_triangles = true;
		if (remove_triangles)
		{
			insertFaces.clear();
			for (M::CEdge* edge : loop)
			{
				for (M::CFace* face : edges_faces[edge->idx()])
				{
					bool all_in = true;
					for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
					{
						M::CEdge* face_edge = *feiter;
						if (loop.find(face_edge) == loop.end())
						{
							all_in = false;
							break;
						}
					}
					if (all_in)
					{
						if (insertFaces.find(face) == insertFaces.end())
						{
							insertFaces.insert(face);
						}
					}
				}
			}
			for (M::CFace* face : insertFaces)
			{
				for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
				{
					M::CEdge* edge = *feiter;
					if (loop.find(edge) == loop.end())
					{
						loop.insert(edge);
						M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
						M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
						//total_mass += (v1->point() + v2->point()) / 2.0;
					}
					else
					{
						loop.erase(edge);
						M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
						M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
						//total_mass -= (v1->point() + v2->point()) / 2.0;
					}
				}
			}
		}
		// find components
		gr.clear();
		empty_vect.clear();
		visited_edges.clear();
		visited_vertices.clear();
		std::vector<std::set<M::CEdge*>> ncomponents;
		for (M::CEdge* edge : loop)
		{
			M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
			M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
			std::pair<M::CVertex*, M::CEdge*> p1(v1, edge);
			std::pair<M::CVertex*, M::CEdge*> p2(v2, edge);
			if (gr.find(v1->idx()) == gr.end())
			{
				gr[v1->idx()] = empty_vect;
			}
			if (gr.find(v2->idx()) == gr.end())
			{
				gr[v2->idx()] = empty_vect;
			}
			gr[v1->idx()].push_back(p2);
			gr[v2->idx()].push_back(p1);
		}
		for (M::CEdge* edge : loop)
		{
			if (visited_edges.find(edge) != visited_edges.end())
			{
				continue;
			}

			std::set<M::CEdge*> component;
			std::queue<M::CVertex*> q;
			q.push(m_pMesh->edge_vertex(edge, 0));
			while (!q.empty())
			{
				M::CVertex* vert = q.front();
				q.pop();
				visited_vertices.insert(vert);
				for (std::pair<M::CVertex*, M::CEdge*> p : gr[vert->idx()])
				{
					if (visited_edges.find(p.second) == visited_edges.end())
					{
						visited_edges.insert(p.second);
						component.insert(p.second);
						if (visited_vertices.find(p.first) == visited_vertices.end())
						{
							q.push(p.first);
						}
					}
				}
			}
			ncomponents.push_back(component);

		}


		single_loop = loop;
		if (ncomponents.size() >= 2)
		{
			single_loop = components[0];
		}
		std::vector<int> lv;
		for (auto component : components)
		{
			bool only_interior = true;
			lv.clear();
			current_loop_edges.clear();
			loop_vertices.clear();
			double length = 0;
			for (M::CEdge* edge : component)
			{
				if (only_interior && m_boundary_edges.find(edge) != m_boundary_edges.end())
				{
					only_interior = false;
				}
				current_loop_edges.push_back(edge);
				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				if (std::find(lv.begin(), lv.end(), v1->idx()) == lv.end())
				{
					lv.push_back(v1->idx());
					loop_vertices.push_back(v1);
				}
				if (std::find(lv.begin(), lv.end(), v2->idx()) == lv.end())
				{
					lv.push_back(v2->idx());
					loop_vertices.push_back(v2);
				}
				length += (v1->point() - v2->point()).norm();
			}
			if (only_interior)
			{
				std::cout << "we skipped this component because it was only in the interior\n";
				continue;
			}
			if (component.size() > 50 && m_boundary_faces.size() > 100000)
			{
				std::cout << "~~~~~~~this component was too long, so it was skipped\n";
				continue;
			}
			good_final_vertices.push_back(loop_vertices);
			good_final_edges.push_back(current_loop_edges);
		}
		lv.clear();
		for (auto component : ncomponents)
		{
			bool only_interior = true;
			lv.clear();
			current_loop_edges.clear();
			loop_vertices.clear();
			for (M::CEdge* edge : component)
			{
				if (only_interior && m_boundary_edges.find(edge) != m_boundary_edges.end())
				{
					only_interior = false;
				}
				current_loop_edges.push_back(edge);
				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				if (std::find(lv.begin(), lv.end(), v1->idx()) == lv.end())
				{
					lv.push_back(v1->idx());
					loop_vertices.push_back(v1);
				}
				if (std::find(lv.begin(), lv.end(), v2->idx()) == lv.end())
				{
					lv.push_back(v2->idx());
					loop_vertices.push_back(v2);
				}
			}
			if (only_interior)
			{
				std::cout << "we skipped this component because it was only in the interior\n";
				continue;
			}
			after_edges.push_back(current_loop_edges);
		}
		return ncomponents.size();

	}


	bool CHandleTunnelLoop::_shorten(std::vector<M::CEdge*> ed_loop)
	{
		CPoint total_mass(0, 0, 0);
		center_of_mass[0] = 0.0;
		center_of_mass[1] = 0.0;
		center_of_mass[2] = 0.0;
		std::set<M::CEdge*> loop;
		for (M::CEdge* edge : ed_loop)
		{
			if (loop.find(edge) == loop.end())
			{
				loop.insert(edge);
				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				total_mass += (v1->point() + v2->point()) / 2.0;
			}
			else
			{
				std::cout << "we had one edge twice!\n";
			}
		}
		//std::cout << "edge loop size" << current_loop_edges.size() << " " << loop.size() << "\n";
		center_of_mass = total_mass / double(loop.size());
		double total_distance = 0;
		for (M::CEdge* edge : loop)
		{
			M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
			M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
			total_distance += ((v1->point() + v2->point()) / 2.0 - center_of_mass).norm();
		}
		double average_distance = total_distance / loop.size();
		//std::cout << "center of mass was " << center_of_mass.print() << "\n";
		int iterations = ed_loop.size() * ed_loop.size() / 4;
		if (ed_loop.size() > 50)
		{
			iterations *= 2;
		}
		if (ed_loop.size() > 100)
		{
			iterations *= 3;
		}
		if (ed_loop.size() > 200)
		{
			iterations *= 5;
		}
		if (ed_loop.size() > 500)
		{
			iterations *= 8;
		}
		int init_iters = iterations;
		std::set<M::CFace*> insertFaces;
		while (iterations > 0)
		{
			iterations -= 1;
			insertFaces.clear();
			std::set<M::CEdge*> prev_loop = loop;
			for (M::CEdge* edge : loop)
			{
				double closestDist = 9999;
				M::CFace* bestFace = NULL;
				//std::cout << "\n\n";
				for (M::CFace* face : edges_faces[edge->idx()])
				{
					CPoint faceCOM(0,0,0);
					for (M::FaceVertexIterator fviter(face); !fviter.end(); ++fviter)
					{
						M::CVertex* vertex = *fviter;
						faceCOM += vertex->point();
					}
					faceCOM /= 3.0;
					//std::cout << "the face's center of mass is " << faceCOM.print() << "\n";

					if ((faceCOM - center_of_mass).norm() < closestDist)
					{
						closestDist = (faceCOM - center_of_mass).norm();
						//std::cout << "we got a new distance, it is " << closestDist << "\n";
						bestFace = face;
					}
				}
				if (insertFaces.find(bestFace) == insertFaces.end())
				{
					
					insertFaces.insert(bestFace);
					if (m_inner_faces.find(bestFace) != m_inner_faces.end())
					{
						//std::cout << "new interior face\n";
					}
					else if (m_boundary_faces.find(bestFace) != m_boundary_faces.end())
					{
						//std::cout << "new boundary face\n";
					}
					else
					{
						std::cout << "an error occured\n";
					}
				}
				else
				{
					//insertFaces.insert(bestFace);
					//std::cout << "we tried to insert the same face\n";
				}
			}
			//std::cout << "the size of the loop and insertFaces are " << loop.size() << " " << insertFaces.size() << "\n";
			for (M::CFace* face : insertFaces)
			{
				//std::cout << "|";
				for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
				{
					M::CEdge* edge = *feiter;
					if (loop.find(edge) == loop.end())
					{
						loop.insert(edge);
						//std::cout << "insert";
						M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
						M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
						total_mass += (v1->point() + v2->point()) / 2.0;
					}
					else
					{
						loop.erase(edge);
						//std::cout << "remove";
						M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
						M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
						total_mass -= (v1->point() + v2->point()) / 2.0;
					}
				}
			}
			if (loop.size() == 0)
			{
				return true;
			}
			//std::cout << "the new size of the loop is now " << loop.size() << "\n";
			center_of_mass = total_mass / double(loop.size());
			// get the average distance
			double total_distance = 0;
			for (M::CEdge* edge : loop)
			{
				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				total_distance += ((v1->point() + v2->point()) / 2.0 -  center_of_mass).norm();
			}
			double new_average_distance = total_distance / loop.size();
			if (new_average_distance > 1.05 * average_distance)
			{
				loop = prev_loop;
				std::cout << "ended with " << iterations << " iterations left, when ";
				break;
			}

			//std::cout << "---------------------------------------------------------the new center of mass is: " << center_of_mass.print() << "\n";
		}
		std::vector<int> lv;
		current_loop_edges.clear();
		loop_vertices.clear();
		for (M::CEdge* edge : loop)
		{
			current_loop_edges.push_back(edge);
			M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
			M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
			if (std::find(lv.begin(), lv.end(), v1->idx()) == lv.end())
			{
				lv.push_back(v1->idx());
				loop_vertices.push_back(v1);
			}
			if (std::find(lv.begin(), lv.end(), v2->idx()) == lv.end())
			{
				lv.push_back(v2->idx());
				loop_vertices.push_back(v2);
			}
		}
		std::cout << "we tried with " << init_iters << " iterations\n";
		std::cout << "the loop shortened from " << ed_loop.size() << " edges to " << current_loop_edges.size() << " edges\n";
		std::cout << "final center of mass is " << center_of_mass.print() << "\n";
		good_final_vertices.push_back(loop_vertices);
		good_final_edges.push_back(current_loop_edges);
		after_edges.push_back(current_loop_edges);
	
		return false;
	}
	bool CHandleTunnelLoop::_shorten()
	{
		CPoint total_mass(0, 0, 0);
		center_of_mass[0] = 0.0;
		center_of_mass[1] = 0.0;
		center_of_mass[2] = 0.0;
		std::set<M::CEdge*> loop;
		for (M::CEdge* edge : current_loop_edges)
		{
			if (loop.find(edge) == loop.end())
			{
				loop.insert(edge);
				M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
				total_mass += (v1->point() + v2->point()) / 2.0;
			}
			else
			{
				std::cout << "we had one edge twice!\n";
			}
		}
		std::cout << "edge loop size" << current_loop_edges.size() << " " << loop.size() << "\n";
		center_of_mass = total_mass / double(loop.size());
		std::cout << "center of mass was " << center_of_mass.print() << "\n";
		int iterations = 500;
		std::set<M::CFace*> insertFaces;
		while (iterations > 0)
		{
			iterations -= 1;
			insertFaces.clear();
			for (M::CEdge* edge : loop)
			{
				double closestDist = 9999;
				M::CFace* bestFace = NULL;
				//std::cout << "\n\n";
				for (M::CFace* face : edges_faces[edge->idx()])
				{
					CPoint faceCOM(0, 0, 0);
					for (M::FaceVertexIterator fviter(face); !fviter.end(); ++fviter)
					{
						M::CVertex* vertex = *fviter;
						faceCOM += vertex->point();
					}
					faceCOM /= 3.0;
					//std::cout << "the face's center of mass is " << faceCOM.print() << "\n";

					if ((faceCOM - center_of_mass).norm() < closestDist)
					{
						closestDist = (faceCOM - center_of_mass).norm();
						//std::cout << "we got a new distance, it is " << closestDist << "\n";
						bestFace = face;
					}
				}
				if (insertFaces.find(bestFace) == insertFaces.end())
				{

					insertFaces.insert(bestFace);
					if (m_inner_faces.find(bestFace) != m_inner_faces.end())
					{
						//std::cout << "new interior face\n";
					}
					else if (m_boundary_faces.find(bestFace) != m_boundary_faces.end())
					{
						//std::cout << "new boundary face\n";
					}
					else
					{
						std::cout << "an error occured\n";
					}
				}
				else
				{
					//insertFaces.insert(bestFace);
					//std::cout << "we tried to insert the same face\n";
				}
			}
			//std::cout << "the size of the loop and insertFaces are " << loop.size() << " " << insertFaces.size() << "\n";
			for (M::CFace* face : insertFaces)
			{
				//std::cout << "|";
				for (M::FaceEdgeIterator feiter(face); !feiter.end(); ++feiter)
				{
					M::CEdge* edge = *feiter;
					if (loop.find(edge) == loop.end())
					{
						loop.insert(edge);
						//std::cout << "insert";
						M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
						M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
						total_mass += (v1->point() + v2->point()) / 2.0;
					}
					else
					{
						loop.erase(edge);
						//std::cout << "remove";
						M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
						M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
						total_mass -= (v1->point() + v2->point()) / 2.0;
					}
				}
			}
			if (loop.size() == 0)
			{
				return true;
			}
			//std::cout << "the new size of the loop is now " << loop.size() << "\n";
			center_of_mass = total_mass / double(loop.size());
			//std::cout << "---------------------------------------------------------the new center of mass is: " << center_of_mass.print() << "\n";
		}
		std::vector<int> lv;
		current_loop_edges.clear();
		loop_vertices.clear();
		// TODO: pick the shortest loop(by average distance to center of mass)
		for (M::CEdge* edge : loop)
		{
			current_loop_edges.push_back(edge);
			M::CVertex* v1 = m_pMesh->edge_vertex(edge, 0);
			M::CVertex* v2 = m_pMesh->edge_vertex(edge, 1);
			if (std::find(lv.begin(), lv.end(), v1->idx()) == lv.end())
			{
				lv.push_back(v1->idx());
				loop_vertices.push_back(v1);
			}
			if (std::find(lv.begin(), lv.end(), v2->idx()) == lv.end())
			{
				lv.push_back(v2->idx());
				loop_vertices.push_back(v2);
			}
		}
		std::cout << "final center of mass is " << center_of_mass.print() << "\n";
		good_final_vertices.push_back(loop_vertices);
		good_final_edges.push_back(current_loop_edges);

		return false;
	}
	bool CHandleTunnelLoop::_repeats(int i)
	{
		// spike goes up and down
		M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
		M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
		if (edg1 == edg2)
		{
			// these 2 can all be removed, along with verts
			int i1 = (i + 0) % current_loop_edges.size();
			int i2 = (i + 1) % current_loop_edges.size();
			std::vector<int> removal_order = { i1, i2 };
			std::sort(removal_order.begin(), removal_order.end(), std::greater<int>());
			center_of_mass *= loop_vertices.size();
			for (int ind : removal_order)
			{
				center_of_mass -= loop_vertices[ind]->point();
			}
			for (int ind : removal_order)
			{
				current_loop_edges.erase(current_loop_edges.begin() + ind);
				loop_vertices.erase(loop_vertices.begin() + ind);
			}
			center_of_mass /= double(loop_vertices.size());
			return true;
		}
		return false;

	}
	bool CHandleTunnelLoop::_repeats2(int i)
	{
		// spike goes up but doesn't return, or there is a gap
		M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
		M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
		M::CEdge* edg3 = current_loop_edges[(i + current_loop_edges.size() - 1) % current_loop_edges.size()];
		int result = _intersection(edg1, edg2, edg3);
		if (result == 0)
		{
			int i1 = (i + 0) % current_loop_edges.size();
			std::vector<int> removal_order = { i1 };
			std::sort(removal_order.begin(), removal_order.end(), std::greater<int>());
			center_of_mass *= loop_vertices.size();
			for (int ind : removal_order)
			{
				center_of_mass -= loop_vertices[ind]->point();
			}
			for (int ind : removal_order)
			{
				current_loop_edges.erase(current_loop_edges.begin() + ind);
				loop_vertices.erase(loop_vertices.begin() + ind);
			}
			center_of_mass /= double(loop_vertices.size());
			return true;
		}
		return false;
	}
	bool CHandleTunnelLoop::_fill_gaps(int i)
	{
		// spike goes up but doesn't return, or there is a gap
		M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
		M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
		int result = _intersection(edg1, edg2);
		
		if (result == 0)
		{
			M::CVertex* pV1 = m_pMesh->edge_vertex(edg1, 0);
			M::CVertex* pV2 = m_pMesh->edge_vertex(edg1, 1);
			M::CVertex* pW1 = m_pMesh->edge_vertex(edg2, 0);
			M::CVertex* pW2 = m_pMesh->edge_vertex(edg2, 1);
			M::CEdge* add_edge = NULL;

			//std::cout << "the 4 vertices are " << pV1->idx() << " " << pV2->idx() << " " << pW1->idx() << " " << pW2->idx() << "\n";
			//std::cout << "the vertices there have indices: " << loop_vertices[(i + loop_vertices.size() - 1) % loop_vertices.size()]->idx() << " " << loop_vertices[i]->idx() << " " << loop_vertices[(i + 1) % loop_vertices.size()]->idx() << "\n";
			M::CVertex* other_vert = NULL;
			/*std::vector < M::CVertex*> four_vertices = { pV1, pV2, pW1, pW2 };
			for (M::CVertex* fvert : four_vertices)
			{
				if (std::find(loop_vertices.begin() + i, loop_vertices.begin() + i + 1, fvert) == loop_vertices.end())
				{
					std::cout << "found the other vertex\n";
					other_vert = fvert;
				}
			}*/
			if (pW1 == loop_vertices[(i + 1) % loop_vertices.size()])
			{
				other_vert = pW2;
			}
			else if (pW2 == loop_vertices[(i + 1) % loop_vertices.size()])
			{
				other_vert = pW1;
			}
			else
			{
				//std::cout << "my idea for gap filing is broken.\n";
				return false;
			}
			if (other_vert == NULL)
			{
				//std::cout << "somthing is wrong here, other_vert is null. this should never happen\n";
				return false;
			}
			// find the edge between othervert and loop_vert[i]
			for (M::VertexEdgeIterator veiter(m_pMesh, other_vert); !veiter.end(); ++veiter)
			{
				M::CEdge* pE = *veiter;
				M::CVertex* v1 = m_pMesh->edge_vertex(pE, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(pE, 1);
				if (v1 == other_vert)
				{
					if (v2 == loop_vertices[i] /*|| v2 == loop_vertices[(i + 1) % loop_vertices.size()]*/)
					{
						if (pE != edg1 && pE != edg2)
						{
							add_edge = pE;
						}
					}
				}
				else if (v2 == other_vert)
				{
					if (v1 == loop_vertices[i] /*|| v1 == loop_vertices[(i + 1) % loop_vertices.size()]*/)
					{
						if (pE != edg1 && pE != edg2)
						{
							add_edge = pE;
						}
					}
				}
				else
				{
					//std::cout << "v1 and v2 are both not the vertex. this should never happpen.\n";
					return false;
				}
			}
			if (add_edge == NULL)
			{
				//std::cout << "my concept for what should be happening is incorrect.\n";
				return false;
			}
			/*for (M::VertexEdgeIterator veiter(m_pMesh, pV1); !veiter.end(); ++veiter)
			{
				M::CEdge* pE = *veiter;
				M::CVertex* v1 = m_pMesh->edge_vertex(pE, 0);
				M::CVertex* v2 = m_pMesh->edge_vertex(pE, 1);
				other = v1 == pV1 ? v2 : v1;
				if (other == pW1 || other == pW2)
				{
					if (std::find(loop_vertices.begin(), loop_vertices.end(), other) != loop_vertices.end())
					{
						//std::cout << "it was actually pV1!!\n";
						other = pV1;
					}
					add_edge = pE;
					break;
				}
			}
			if (add_edge == NULL)
			{
				for (M::VertexEdgeIterator veiter(m_pMesh, pV2); !veiter.end(); ++veiter)
				{
					M::CEdge* pE = *veiter;
					M::CVertex* v1 = m_pMesh->edge_vertex(pE, 0);
					M::CVertex* v2 = m_pMesh->edge_vertex(pE, 1);
					other = v1 == pV2 ? v2 : v1;
					if (other == pW1 || other == pW2)
					{
						if (std::find(loop_vertices.begin(), loop_vertices.end(), other) != loop_vertices.end())
						{
							//std::cout << "it was actually pV2!!\n";
							other = pV2;
						}
						add_edge = pE;
						break;
					}
				}
			}
			//if (std::find(loop_vertices.begin(), loop_vertices.end(), other) != loop_vertices.end())
			//{
				//std::cout << "this was not at all what I expected! Wrong now!\n";
			//}
			if (add_edge == NULL)
			{
				//std::cout << "edge to be added is null!??\n";
				return false;
			}*/
			center_of_mass *= loop_vertices.size();
			center_of_mass += other_vert->point();
			center_of_mass /= double(loop_vertices.size() + 1);
			//std::cout << "the vertex and edge are: " << other_vert->idx() << " " << add_edge->idx() << "\n";
			if (i == current_loop_edges.size() - 1)
			{
				current_loop_edges.push_back(add_edge);
				loop_vertices.push_back(other_vert);
			}
			else
			{
				current_loop_edges.insert(current_loop_edges.begin() + i, add_edge);
				loop_vertices.insert(loop_vertices.begin() + i, other_vert);
			}
			std::cout << "we've fixed the gap!\n";
			return true;
		}
		return false;
	}
	bool CHandleTunnelLoop::_triple0(int i)
	{
		M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
		M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
		M::CEdge* edg3 = current_loop_edges[(i + 2) % current_loop_edges.size()];

		std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
		std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
		std::vector<M::CFace*> vec3 = edges_faces[edg3->idx()];
		_intersection(vec1, vec2);
		if (face_intersection.size() == 1)
		{
			_intersection(vec1, vec3);
			if (face_intersection.size() == 1)
			{
				_intersection(vec2, vec3);
				if (face_intersection.size() == 1)
				{
					if (edg1 == edg2 || edg1 == edg3 || edg2 == edg3)
					{
						//std::cout << "duplicated edges put inside the triples?? this should have been removed earlier...\n";
						return false;
					}
					// these 3 can all be removed, along with verts
					int i1 = (i + 0) % current_loop_edges.size();
					int i2 = (i + 1) % current_loop_edges.size();
					int i3 = (i + 2) % current_loop_edges.size();
					std::vector<int> removal_order = { i1, i2, i3 };
					std::sort(removal_order.begin(), removal_order.end(), std::greater<int>());
					center_of_mass *= loop_vertices.size();
					for (int ind : removal_order)
					{
						center_of_mass -= loop_vertices[ind]->point();
					}
					for (int ind : removal_order)
					{
						current_loop_edges.erase(current_loop_edges.begin() + ind);
						loop_vertices.erase(loop_vertices.begin() + ind);
					}
					center_of_mass /= double(loop_vertices.size());
					return true;
				}
			}
		}
		return false;
	}
	bool CHandleTunnelLoop::_triple1(int i)
	{
		i = ((i + current_loop_edges.size()) - 1) % current_loop_edges.size();
		M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
		M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
		M::CEdge* edg3 = current_loop_edges[(i + 2) % current_loop_edges.size()];

		std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
		std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
		std::vector<M::CFace*> vec3 = edges_faces[edg3->idx()];
		_intersection(vec1, vec2);
		if (face_intersection.size() == 1)
		{
			_intersection(vec1, vec3);
			if (face_intersection.size() == 1)
			{
				_intersection(vec2, vec3);
				if (face_intersection.size() == 1)
				{
					// these 3 can all be removed, along with verts
					int i1 = (i + 0) % current_loop_edges.size();
					int i2 = (i + 1) % current_loop_edges.size();
					int i3 = (i + 2) % current_loop_edges.size();
					std::vector<int> removal_order = { i1, i2, i3 };
					std::sort(removal_order.begin(), removal_order.end(), std::greater<int>());
					center_of_mass *= loop_vertices.size();
					for (int ind : removal_order)
					{
						center_of_mass -= loop_vertices[ind]->point();
					}
					for (int ind : removal_order)
					{
						current_loop_edges.erase(current_loop_edges.begin() + ind);
						loop_vertices.erase(loop_vertices.begin() + ind);
					}
					center_of_mass /= double(loop_vertices.size());
					return true;
				}
			}
		}
		return false;
	}
	bool CHandleTunnelLoop::_triple2(int i)
	{
		i = ((i + current_loop_edges.size()) - 2) % current_loop_edges.size();
		M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
		M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
		M::CEdge* edg3 = current_loop_edges[(i + 2) % current_loop_edges.size()];

		std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
		std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
		std::vector<M::CFace*> vec3 = edges_faces[edg3->idx()];
		_intersection(vec1, vec2);
		if (face_intersection.size() == 1)
		{
			_intersection(vec1, vec3);
			if (face_intersection.size() == 1)
			{
				_intersection(vec2, vec3);
				if (face_intersection.size() == 1)
				{
					// these 3 can all be removed, along with verts
					int i1 = (i + 0) % current_loop_edges.size();
					int i2 = (i + 1) % current_loop_edges.size();
					int i3 = (i + 2) % current_loop_edges.size();
					std::vector<int> removal_order = { i1, i2, i3 };
					std::sort(removal_order.begin(), removal_order.end(), std::greater<int>());
					center_of_mass *= loop_vertices.size();
					for (int ind : removal_order)
					{
						center_of_mass -= loop_vertices[ind]->point();
					}
					for (int ind : removal_order)
					{
						current_loop_edges.erase(current_loop_edges.begin() + ind);
						loop_vertices.erase(loop_vertices.begin() + ind);
					}
					center_of_mass /= double(loop_vertices.size());
					return true;
				}
			}
		}
		return false;
	}
	bool CHandleTunnelLoop::_double(int i)
	{
		M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
		M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
		M::CEdge* edg3;
		std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
		std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
		_intersection(vec1, vec2);
		if (face_intersection.size() == 1)
		{
			M::CFace* fac1 = face_intersection[0];
			for (M::FaceEdgeIterator feiter(fac1); !feiter.end(); ++feiter)
			{
				M::CEdge* pE = *feiter;
				if (pE != edg1 && pE != edg2)
				{
					edg3 = pE;
					break;
				}
			}
			// find center of edg3
			M::CVertex* pV = m_pMesh->edge_vertex(edg3, 0);
			M::CVertex* pW = m_pMesh->edge_vertex(edg3, 1);
			//p1 is the closer vertex
			CPoint p1;
			// p1 is just the average of the two
			p1 = (pV->point() + pW->point()) /= 2.0;
			// find shared vertex of edg1 and edg2
			M::CVertex* pV1 = m_pMesh->edge_vertex(edg1, 0);
			M::CVertex* pW1 = m_pMesh->edge_vertex(edg1, 1);
			M::CVertex* pV2 = m_pMesh->edge_vertex(edg2, 0);
			M::CVertex* pW2 = m_pMesh->edge_vertex(edg2, 1);
			M::CVertex* shared_v;

			if (pV1 == pV2 || pV1 == pW2)
			{
				shared_v = pV1;
			}
			else if (pW1 == pV2 || pW1 == pW2)
			{
				shared_v = pW1;
			}
			CPoint p3 = shared_v->point();
			CPoint mid1 = (shared_v->point() + pV->point()) / 2.0;
			CPoint mid2 = (shared_v->point() + pW->point()) / 2.0;
			CPoint COM = center_of_mass;
			CPoint p2 = ((mid1 - COM).norm() < (mid2 - COM).norm()) ? mid1 : mid2;

			//farthest midpoint - farthest vertex of new edge
			CPoint p4 = (pV->point() - center_of_mass).norm() > (pW->point() - center_of_mass).norm() ? pV->point() : pW->point();
			CPoint p5 = ((mid1 - center_of_mass).norm() > (mid2 - center_of_mass).norm()) ? mid1 : mid2;
			p4 = (pV->point() + pW->point()) / 2.0;
			p5 = shared_v->point();
			//shared point - midpoint of new edge
			double improvement = (p5 - COM).norm() - (p4 - COM).norm();
			//if (improvement > 0)
			//std::cout << "double\n";
			if (boundary_shorten == false)
			{
				if (_different_side(pV, pW, center_of_mass, shared_v))
				{
					best_edge_o1 = edg1;
					best_edge_o2 = edg2;
					best_face_o = fac1;
					idx_best_edge_o1 = i;
					idx_best_edge_o2 = (i + 1) % current_loop_edges.size();
					return true;
				}
			}
			else if (boundary_shorten == true)
			{
				if (improvement > 0)
				{
					best_edge_o1 = edg1;
					best_edge_o2 = edg2;
					best_face_o = fac1;
					idx_best_edge_o1 = i;
					idx_best_edge_o2 = (i + 1) % current_loop_edges.size();
					return true;
				}
			}
		}
		return false;
	}
	bool CHandleTunnelLoop::_single(int i) 
	{
		bool ret_true = false;
		M::CEdge* pE = current_loop_edges[i];
		M::CVertex* pV1 = m_pMesh->edge_vertex(pE, 0);
		M::CVertex* pV2 = m_pMesh->edge_vertex(pE, 1);

		CPoint further_point = (pV1->point() - center_of_mass).norm() > (pV2->point() - center_of_mass).norm() ? pV1->point() : pV2->point();
		CPoint closer_point = (pV1->point() - center_of_mass).norm() < (pV2->point() - center_of_mass).norm() ? pV1->point() : pV2->point();
		CPoint mid_orig = (pV1->point() + pV2->point()) / 2.0;
		double distance = (mid_orig - center_of_mass).norm();
		double orig_mid_distance = (mid_orig - center_of_mass).norm();
		double further_point_dist = (further_point - center_of_mass).norm();
		for (M::CFace* pF : edges_faces[pE->idx()])
		{
			bool bad = false;
			for (std::pair<M::CFace*, std::pair<M::CEdge*, CPoint>> pair2 : single_to_double)
			{
				if (pF == pair2.first && pE == pair2.second.first && (center_of_mass - pair2.second.second).norm() < 0.00001)
				{
					bad = true;
					break;
				}
			}
			if (bad)
			{
				continue;
			}
			M::CEdge* other_e1 = NULL;
			M::CEdge* other_e2 = NULL;
			bad = false;
			for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
			{
				M::CEdge* pE2 = *feiter;
				if (pE2 == current_loop_edges[(i + 1) % current_loop_edges.size()] || pE2 == current_loop_edges[(i + current_loop_edges.size() - 1) % current_loop_edges.size()])
				{
					bad = true;
					break;
				}
				if (pE2 != pE && other_e1 == NULL)
				{
					other_e1 = pE2;
				}
				else if (pE2 != pE && other_e2 == NULL)
				{
					other_e2 = pE2;
				}
			}
			if (bad)
			{
				// double but points wrong way
				continue;
			}
			// get the 3rd vertex
			M::CVertex* pV3;
			for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
			{
				M::CVertex* pV = *fviter;
				if (pV != pV1 && pV != pV2)
				{
					pV3 = pV;
					//found the third, exit.
					break;
				}

			}
			/* allow repeated vertices
			if (std::find(loop_vertices.begin(), loop_vertices.end(), pV3) != loop_vertices.end())
			{
				continue;
			}*/
			//compare midpoint of closer new edge to midpoint of original edge
			CPoint mid_closer = (closer_point + pV3->point()) / 2.0;
			CPoint mid_further = (further_point + pV3->point()) / 2.0;
			//compare to new vertex
			mid_closer = pV3->point();
			//std::cout << "single\n";
			if (boundary_shorten == false)
			{
				if (_different_side(pV1, pV2, center_of_mass, pV3))
				{
					continue;
				}
			}
			//if ((mid_closer - center_of_mass).norm() < distance)
			//if ((mid_closer - center_of_mass).norm() < orig_mid_distance && (mid_further - center_of_mass).norm() < further_point_dist)
			if (boundary_shorten == false)
			{
				if ((pV3->point() - center_of_mass).norm() < further_point_dist)
				{
					//distance = (mid_closer - center_of_mass).norm();
					//orig_mid_distance = (mid_closer - center_of_mass).norm();
					//further_point_dist = (mid_further - center_of_mass).norm();
					further_point_dist = (pV3->point() - center_of_mass).norm();
					best_face_o = pF;
					best_edge_o = pE;
					idx_best_edge_o = i;
					ret_true = true;
				}
			}
			else if (boundary_shorten == true)
			{
				if ((pV3->point() - center_of_mass).norm() < further_point_dist)
				{
					//distance = (mid_closer - center_of_mass).norm();
					//orig_mid_distance = (mid_closer - center_of_mass).norm();
					//further_point_dist = (mid_further - center_of_mass).norm();
					further_point_dist = (pV3->point() - center_of_mass).norm();
					best_face_o = pF;
					best_edge_o = pE;
					idx_best_edge_o = i;
					ret_true = true;
				}
			}
		}
		if (ret_true)
		{
			return true;
		}
		return false;
	}
	bool CHandleTunnelLoop::_delete_triple()
	{
		for (int i = 0; i < current_loop_edges.size(); i++)
		{
			M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
			M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
			M::CEdge* edg3 = current_loop_edges[(i + 2) % current_loop_edges.size()];

			std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
			std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
			std::vector<M::CFace*> vec3 = edges_faces[edg3->idx()];
			_intersection(vec1, vec2);
			if (face_intersection.size() == 1)
			{
				_intersection(vec1, vec3);
				if (face_intersection.size() == 1)
				{
					_intersection(vec2, vec3);
					if (face_intersection.size() == 1)
					{
						
						// these 3 can all be removed, along with verts
						int i1 = (i + 0) % current_loop_edges.size();
						int i2 = (i + 1) % current_loop_edges.size();
						int i3 = (i + 2) % current_loop_edges.size();
						std::vector<int> removal_order = { i1, i2, i3 };
						std::sort(removal_order.begin(), removal_order.end(), std::greater<int>());
						center_of_mass *= loop_vertices.size();
						for (int ind : removal_order)
						{
							center_of_mass -= loop_vertices[ind]->point();
						}
						for (int ind : removal_order)
						{
							current_loop_edges.erase(current_loop_edges.begin() + ind);
							loop_vertices.erase(loop_vertices.begin() + ind);
						}
						center_of_mass /= double(loop_vertices.size());
						return true;
					}
				}
			}
			
		}
		return false;
	}
	double CHandleTunnelLoop::_check_double()
	{
		best_improve_o = 0.0;
		equal_dist_imp = 0.0;
		for (int i = 0; i < current_loop_edges.size(); i++)
		{
			M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
			M::CEdge* edg2 = current_loop_edges[(i+1) % current_loop_edges.size()];
			M::CEdge* edg3;
			std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
			std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
			_intersection(vec1, vec2);
			if (face_intersection.size() == 1)
			{
				M::CFace* fac1 = face_intersection[0];
				for (M::FaceEdgeIterator feiter(fac1); !feiter.end(); ++feiter)
				{
					M::CEdge* pE = *feiter;
					if (pE != edg1 && pE != edg2)
					{
						edg3 = pE;
						break;
					}
				}
				// find center of edg3
				M::CVertex* pV = m_pMesh->edge_vertex(edg3, 0);
				M::CVertex* pW = m_pMesh->edge_vertex(edg3, 1);
				//p1 is the closer vertex
				CPoint p1;
				/*if ((pV->point() - center_of_mass).norm() < (pW->point() - center_of_mass).norm())
				{
					p1 = pV->point();
				}
				else
				{
					p1 = pW->point();
				}*/
				// p1 is just the average of the two
				p1 = (pV->point() + pW->point()) /= 2.0;
				// find shared vertex of edg1 and edg2
				M::CVertex* pV1 = m_pMesh->edge_vertex(edg1, 0);
				M::CVertex* pW1 = m_pMesh->edge_vertex(edg1, 1);
				M::CVertex* pV2 = m_pMesh->edge_vertex(edg2, 0);
				M::CVertex* pW2 = m_pMesh->edge_vertex(edg2, 1);
				M::CVertex* shared_v;
				
				if (pV1 == pV2 || pV1 == pW2)
				{
					shared_v = pV1;
				}
				else if (pW1 == pV2 || pW1 == pW2)
				{
					shared_v = pW1;
				}
				else
				{
					std::cout << "ERROR: edges share a face but don't share a vertex-----------------------";
				}
				CPoint p3 = shared_v->point();
				CPoint mid1 = (shared_v->point() + pV->point()) / 2.0;
				CPoint mid2 = (shared_v->point() + pW->point()) / 2.0;
				// new center of mass used for calculations.
				CPoint COM = center_of_mass;
				/*COM *= loop_vertices.size();
				COM -= p3;
				COM /= double(loop_vertices.size() - 1);*/
				CPoint p2 = ((mid1 - COM).norm() < (mid2 - COM).norm()) ? mid1 : mid2;

				/*bool bad = false;
				for (std::pair<M::CFace*, M::CEdge*> pair2 : single_to_double)
				{
					if (fac1 == pair2.first && edg3 == pair2.second)
					{
						bad = true;
						break;
					}
				}
				if (bad)
				{
					continue;
				}
				std::vector<M::CEdge*> vec1;
				std::vector<M::CEdge*> vec2;
				vec1.push_back(edg1);
				vec1.push_back(edg2);
				vec2.push_back(edg2);
				vec2.push_back(edg1);
				bad = false;
				for (std::pair<M::CFace*, std::vector<M::CEdge*>> pair2 : double_to_single)
				{
					if (fac1 == pair2.first)
					{
						if (vec1 == pair2.second || vec2 == pair2.second)
						{
							bad = true;
							break;
						}
					}
				}
				if (bad)
				{
					continue;
				}*/
				//farthest midpoint - farthest vertex of new edge
				CPoint p4 = (pV->point() - center_of_mass).norm() > (pW->point() - center_of_mass).norm() ? pV->point() : pW->point();
				CPoint p5 = ((mid1 - center_of_mass).norm() > (mid2 - center_of_mass).norm()) ? mid1 : mid2;
				//std::cout << "this double is: " << (p5 - center_of_mass).norm() << " is the old length\n";
				//std::cout << "this double is: " << (p4 - center_of_mass).norm() << " is the new length\n";
				//std::cout << "the vertices are: " << pV->idx() << " " << shared_v->idx() << " " << pW->idx() << "\n";
				//std::cout << "the vertices are: " << pV->point().print() << " " << shared_v->point().print() << " " << pW->point().print() << "\n";
				//std::cout << "the center of mass is: " << center_of_mass.print() << "\n";
				p4 = (pV->point() + pW->point()) / 2.0;
				p5 = shared_v->point();
				//shared point - midpoint of new edge
				double improvement = (p5 - COM).norm() - (p4 - COM).norm();
				double distance = (p3 - center_of_mass).norm();
				if (improvement > 0 && distance >= best_improve_o )
				{
					//std::cout << "there was an improvement in the doubles!? The indices of the verts are: "
					//	<< pV->idx() << " " << shared_v->idx() << " " << pW->idx() << "\n";
					if (distance == best_improve_o)
					{
						if (improvement > equal_dist_imp) 
						{
							equal_dist_imp = improvement;
						}
						else
						{
							continue;
						}
					}
					best_improve_o = distance; // improvement instead of distance?
					best_edge_o1 = edg1;
					best_edge_o2 = edg2;
					best_face_o = fac1;
					idx_best_edge_o1 = i;
					idx_best_edge_o2 = (i + 1) % current_loop_edges.size();
				}
				/*if (improvement > 0.0)
				{
					if (last_step_s != NULL && shared_v == last_step_s)
					{
						std::cout << "ended because of last step\n";
						continue;
					}
					best_improve_o = improvement;
					best_edge_o1s.push_back(edg1);
					best_edge_o2s.push_back(edg2);
					best_face_os.push_back(fac1);
					idx_best_edge_o1s.push_back(i);
					idx_best_edge_o2s.push_back((i + 1) % current_loop_edges.size());
				}*/
				
			}
		}
		return best_improve_o;
	}
	double CHandleTunnelLoop::_check_single()
	{
		//pick the best single edge to split into 2 edges (for each edge pick best face/new vertex, compare the bests)
		best_improve_o = 0.0;
		equal_dist_imp = 0.0;
		for (int i = 0; i < current_loop_edges.size(); i++)
		{
			M::CEdge* pE = current_loop_edges[i];
			M::CVertex* pV1 = m_pMesh->edge_vertex(pE, 0);
			M::CVertex* pV2 = m_pMesh->edge_vertex(pE, 1);
			
			CPoint further_point = (pV1->point() - center_of_mass).norm() > (pV2->point() - center_of_mass).norm() ? pV1->point() : pV2->point();
			CPoint closer_point = (pV1->point() - center_of_mass).norm() < (pV2->point() - center_of_mass).norm() ? pV1->point() : pV2->point();
			CPoint mid_orig = (pV1->point() + pV2->point()) / 2.0;
			double orig_distance = (mid_orig - center_of_mass).norm();
			double distance = orig_distance;
			if (orig_distance < best_improve_o)
			{
				continue;
			}
			//std::cout << "distance of " << pV1->idx() << " " << pV2->idx() << " to the center is " << orig_distance << "\n";

			// get the closest face (3rd vertex closest to center)
			// start the distance as furthest vertex?
			
			for (M::CFace* pF : edges_faces[pE->idx()])
			{
				bool bad = false;
				for (std::pair<M::CFace*, std::pair<M::CEdge*, CPoint>> pair2 : single_to_double)
				{
					if (pF == pair2.first && pE == pair2.second.first && (center_of_mass - pair2.second.second).norm() < 0.00001 )
					{
						//std::cout << "yup!\n\n\n\n";
						bad = true;
						break;
					}
				}
				if (bad)
				{
					continue;
				}
				// first check that the face isn't a double
				//also get the other 2 edges;
				M::CEdge* other_e1 = NULL;
				M::CEdge* other_e2 = NULL;
				bad = false;
				for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
				{
					M::CEdge* pE2 = *feiter;
					if (pE2 == current_loop_edges[(i + 1) % current_loop_edges.size()] || pE2 == current_loop_edges[(i + current_loop_edges.size() - 1) % current_loop_edges.size()])
					{
						bad = true;
						break;
					}
					if (pE2 != pE && other_e1 == NULL)
					{
						other_e1 = pE2;
					}
					else if (pE2 != pE && other_e2 == NULL)
					{
						other_e2 = pE2;
					}
				}
				if (bad)
				{
					//std::cout << "this was actually a " << count << ", not a single\n";
					continue;
				}
				// get the 3rd vertex
				M::CVertex* pV3;
				

				for (M::FaceVertexIterator fviter(pF); !fviter.end(); ++fviter)
				{
					M::CVertex* pV = *fviter;
					if (pV != pV1 && pV != pV2)
					{
						pV3 = pV;
						//found the third, exit.
						break;
					}

				}
				CPoint PV3_p = pV3->point();
				//compare midpoint of closer new edge to midpoint of original edge
				CPoint mid_closer = (closer_point + PV3_p) / 2.0;
				if ((mid_closer - center_of_mass).norm() < distance)
				{
					//std::cout << "this single is: " << orig_distance << " is the old length\n";
					//std::cout << "this single is: " << (mid_closer - center_of_mass).norm() << " is the new length\n";
					distance = (mid_closer - center_of_mass).norm();
					best_face_o = pF;
					best_edge_o = pE;
					idx_best_edge_o = i;
					best_improve_o = orig_distance;
					continue;

				}
				else
				{
					continue;
				}
				// check that 3rd vertex isn't already in the loop or destroyed by the doubles
				/*if (std::find(loop_vertices.begin(), loop_vertices.end(), pV3) != loop_vertices.end())
				{
					//std::cout << "can't go to a vertex already in the loop!\n";
					continue;
				}*/
				/*std::vector<M::CEdge*> vec1;
				std::vector<M::CEdge*> vec2;
				vec1.push_back(other_e1);
				vec1.push_back(other_e2);
				vec2.push_back(other_e2);
				vec2.push_back(other_e1);
				bool bad = false;
				for (std::pair<M::CFace*, std::vector<M::CEdge*>> pair2 : double_to_single)
				{
					if (pF == pair2.first)
					{
						if (vec1 == pair2.second || vec2 == pair2.second)
						{
							bad = true;
							break;
						}
					}
				}
				if (bad)
				{
					continue;
				}
				bad = false;
				for (std::pair<M::CFace*, M::CEdge*> pair2 : single_to_double)
				{
					if (pF == pair2.first && pE == pair2.second)
					{
						bad = true;
						break;
					}
				}
				if (bad)
				{
					continue;
				}*/
				// new vertex
				//CPoint p1 = pV3->point();
				//find the 2 new midpoints
				CPoint COM = center_of_mass;
				/*COM *= double(loop_vertices.size());
				COM += PV3_p;
				COM /= double(loop_vertices.size() - 1);*/
				CPoint mid1 = (pV1->point() + pV3->point()) / 2.0;
				CPoint mid2 = (pV2->point() + pV3->point()) / 2.0;
				CPoint p1 = ((mid1 - COM).norm() > (mid2 - COM).norm()) ? mid1 : mid2;
				//p1 = pV3->point();
				// old edge midpoint
				CPoint p2 = (pV1->point() + pV2->point()) / 2.0;
				// farther edge midpoint?
				//CPoint p2 = ((pV1->point() - center_of_mass).norm() > (pV2->point() - center_of_mass).norm()) ? pV1->point() : pV2->point();
				// find improvement = old dist - new dist
				CPoint p3 = (pV1->point() - center_of_mass).norm() > (pV2->point() - center_of_mass).norm() ? pV1->point() : pV2->point();
				double improve = (p3 - COM).norm() - (p1 - center_of_mass).norm();
				double distance = (p2 - center_of_mass).norm();
				if (improve > 0.0 && distance >= best_improve_o )
				{
					if (distance == best_improve_o)
					{
						if (improve >= equal_dist_imp)
						{
							equal_dist_imp = improve;
						}
						else
						{
							continue;
						}
					}
					best_edge_o = pE;
					best_face_o = pF;
					best_improve_o = distance; // improve instead of distance?
					idx_best_edge_o = i;
				}
				/*if (improve >= 0.0)
				{
					best_edge_os.push_back(pE);
					best_face_os.push_back(pF);
					best_improve_o = improve;
					idx_best_edge_os.push_back(i);
				}*/
			}

		}
		return best_improve_o;
		


	}
	void CHandleTunnelLoop::_change_double()
	{
		M::CEdge* edg1 = best_edge_o1;
		M::CEdge* edg2 = best_edge_o2;
		M::CEdge* edg3;
		std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
		std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
		_intersection(vec1, vec2);
		if (face_intersection.size() != 1)
		{
			std::cout << "face intersection was 1, but now is not??";
			return;
		}
		M::CFace* fac1 = face_intersection[0];
		for (M::FaceEdgeIterator feiter(fac1); !feiter.end(); ++feiter)
		{
			M::CEdge* pE = *feiter;
			if (pE != edg1 && pE != edg2)
			{
				edg3 = pE;
				break;
			}
		}
		// find center of edg3
		M::CVertex* pV = m_pMesh->edge_vertex(edg3, 0);
		M::CVertex* pW = m_pMesh->edge_vertex(edg3, 1);
		CPoint p1 = (pV->point() + pW->point()) / 2.0;
		// find shared vertex of edg1 and edg2
		M::CVertex* pV1 = m_pMesh->edge_vertex(edg1, 0);
		M::CVertex* pW1 = m_pMesh->edge_vertex(edg1, 1);
		M::CVertex* pV2 = m_pMesh->edge_vertex(edg2, 0);
		M::CVertex* pW2 = m_pMesh->edge_vertex(edg2, 1);
		M::CVertex* shared_v;
		CPoint p2;
		if (pV1 == pV2 || pV1 == pW2)
		{
			p2 = pV1->point();
			shared_v = pV1;
		}
		else if (pW1 == pV2 || pW1 == pW2)
		{
			p2 = pW1->point();
			shared_v = pW1;
		}
		else
		{
			std::cout << "ERROR: edges share a face but don't share a vertex-----------------------";
		}
		//std::cout << "made a change!\n";
		// make the change, since p1 is closer to center of mass than p2
		// update center of mass by removing p2;
		center_of_mass *= loop_vertices.size();
		center_of_mass -= p2;
		center_of_mass /= double(loop_vertices.size() - 1);
		// update vertex list by removing shared_v;
		//last_step_d1 = shared_v;
		std::vector<M::CEdge*> two_edges;
		two_edges.push_back(edg1);
		two_edges.push_back(edg2);
		std::pair<M::CFace*, std::vector<M::CEdge*>> this_pair = std::make_pair(fac1, two_edges);
		double_to_single.push_back(this_pair);
		//loop_vertices.erase(std::remove(loop_vertices.begin(), loop_vertices.end(), shared_v), loop_vertices.end());
		// update edge list by removing edg1 and edg2 but adding edg3 at position i;
		// check if 2nd edge is at index 0 now
		//std::cout << "index of shared v: " << std::find(loop_vertices.begin(), loop_vertices.end(), shared_v) - loop_vertices.begin() << " of out " << loop_vertices.size() << "\n";
		//std::cout << "while the 2nd edge index was " << idx_best_edge_o2 << " out of " << current_loop_edges.size() << "\n";
		if (idx_best_edge_o2 == 0)
		{
			green_edges.push_back(current_loop_edges[idx_best_edge_o1]);
			current_loop_edges.erase(current_loop_edges.begin() + idx_best_edge_o1);
			green_edges.push_back(current_loop_edges[0]);
			current_loop_edges.erase(current_loop_edges.begin());
			current_loop_edges.push_back(edg3);
			loop_vertices.erase(loop_vertices.begin());
		}
		else
		{
			green_edges.push_back(current_loop_edges[idx_best_edge_o2]);
			current_loop_edges.erase(current_loop_edges.begin() + idx_best_edge_o2);
			green_edges.push_back(current_loop_edges[idx_best_edge_o1]);
			current_loop_edges.erase(current_loop_edges.begin() + idx_best_edge_o1);
			current_loop_edges.insert(current_loop_edges.begin() + idx_best_edge_o1, edg3);
			loop_vertices.erase(loop_vertices.begin() + idx_best_edge_o2);

		}
		/*current_loop_edges.erase(std::remove(current_loop_edges.begin(), current_loop_edges.end(), edg1), current_loop_edges.end());
		current_loop_edges.erase(std::remove(current_loop_edges.begin(), current_loop_edges.end(), edg2), current_loop_edges.end());
		if (idx_best_edge_o1 >= current_loop_edges.size())
		{
			current_loop_edges.push_back(edg3);
		}
		else
		{
			current_loop_edges.insert(current_loop_edges.begin() + idx_best_edge_o1, edg3);
		}*/
		//std::cout << "after inserting, current loop has size " << current_loop_edges.size() << "\n";
	}
	void CHandleTunnelLoop::_change_single()
	{
		//std::cout << "our best improvement was: " << best_improve_o << "\n";
		// make change to the best edge

		// remove current edge
		current_loop_edges.erase(current_loop_edges.begin() + idx_best_edge_o);

		// get the other 2 edges's vertices and the removed edge's vertices
		M::CEdge* edg1 = NULL;
		M::CEdge* edg2 = NULL;
		for (M::FaceEdgeIterator feiter(best_face_o); !feiter.end(); ++feiter)
		{
			M::CEdge* pE = *feiter;
			if (pE != best_edge_o && edg1 == NULL)
			{
				edg1 = pE;
			}
			else if (pE != best_edge_o && edg2 == NULL)
			{
				edg2 = pE;
			}
		}
		M::CVertex* V1 = m_pMesh->edge_vertex(edg1, 0);
		M::CVertex* W1 = m_pMesh->edge_vertex(edg1, 1);
		M::CVertex* V2 = m_pMesh->edge_vertex(edg2, 0);
		M::CVertex* W2 = m_pMesh->edge_vertex(edg2, 1);
		// first find shared vertex
		M::CVertex* shared_v;
		M::CVertex* other_v1;
		M::CVertex* other_v2;
		if (V1 == V2 || V1 == W2)
		{
			shared_v = V1;
			if (V1 == V2)
			{
				other_v1 = W1;
				other_v2 = W2;
			}
			else if (V1 == W2)
			{
				other_v1 = W1;
				other_v2 = V2;
			}
		}
		else if (W1 == V2 || W1 == W2)
		{
			shared_v = W1;
			if (W1 == V2)
			{
				other_v1 = V1;
				other_v2 = W2;
			}
			else if (W1 == W2)
			{
				other_v1 = V1;
				other_v2 = V2;
			}
		}
		std::pair<M::CFace*, std::pair<M::CEdge*, CPoint>> this_pair2 = std::make_pair(best_face_o, std::make_pair(best_edge_o, center_of_mass));
		single_to_double.push_back(this_pair2);
		CPoint shared_p = shared_v->point();
		// update center of mass
		center_of_mass *= loop_vertices.size();
		center_of_mass += shared_p;
		center_of_mass /= double(loop_vertices.size() + 1);
		// add 2 edges in the correct order(check next edge, the current ith edge)
		M::CVertex* V3 = m_pMesh->edge_vertex(current_loop_edges[idx_best_edge_o % int(current_loop_edges.size())], 0);
		M::CVertex* W3 = m_pMesh->edge_vertex(current_loop_edges[idx_best_edge_o % int(current_loop_edges.size())], 1);
		// insert the edge which connects to ith(because we removed the ith edge already) at ith index
		// insert the other edge at ith index
		if ((other_v1 == V3 || other_v1 == W3) && (other_v2 != V3 && other_v2 != W3))
		{
			if (idx_best_edge_o == current_loop_edges.size())
			{
				// we removed the last edge
				current_loop_edges.push_back(edg2);
				current_loop_edges.push_back(edg1);
				loop_vertices.push_back(shared_v);
			}
			else
			{
				current_loop_edges.insert(current_loop_edges.begin() + idx_best_edge_o, edg1);
				current_loop_edges.insert(current_loop_edges.begin() + idx_best_edge_o, edg2);
				loop_vertices.insert(loop_vertices.begin() + idx_best_edge_o + 1, shared_v);
			}
		}
		else
		{
			if (idx_best_edge_o == current_loop_edges.size())
			{
				// we removed the last edge
				current_loop_edges.push_back(edg1);
				current_loop_edges.push_back(edg2);
				loop_vertices.push_back(shared_v);
			}
			else
			{
				current_loop_edges.insert(current_loop_edges.begin() + idx_best_edge_o, edg2);
				current_loop_edges.insert(current_loop_edges.begin() + idx_best_edge_o, edg1);
				loop_vertices.insert(loop_vertices.begin() + idx_best_edge_o + 1, shared_v);
			}
		}
		if (_triple2(idx_best_edge_o))
		{
			index = (index + current_loop_edges.size() - 2) % current_loop_edges.size();
		}
		/*
		// add the 3rd vertex, find index of the 2 vertices, i and i + 1. insert 3rd vertex at i+1
		auto pos1 = std::find(loop_vertices.begin(), loop_vertices.end(), other_v1);
		auto pos2 = std::find(loop_vertices.begin(), loop_vertices.end(), other_v2);
		//std::cout << "pos1 is " << pos1 - loop_vertices.begin() << "\n";
		//std::cout << "pos2 is " << pos2 - loop_vertices.begin() << "\n";
		//std::cout << "the vertices have idx " << other_v1->idx() << " and " << other_v2->idx() << "\n";
		if (pos1 - pos2 != 1 && pos2 - pos1 != 1 && pos1 - pos2 != int(loop_vertices.size()) - 1 && pos2 - pos1 != int(loop_vertices.size()) - 1)
		{
			std::cout << "something broke in finding the indicies of the 2 vertices\n";
		}
		else if (pos1 - pos2 == 1)
		{
			loop_vertices.insert(pos1, shared_v);
		}
		else if (pos2 - pos1 == 1)
		{
			loop_vertices.insert(pos2, shared_v);
		}
		else if (pos1 - pos2 == int(loop_vertices.size()) - 1)
		{
			loop_vertices.insert(pos2, shared_v);
		}
		else if (pos2 - pos1 == int(loop_vertices.size()) - 1)
		{
			loop_vertices.insert(pos1, shared_v);
		}

		//last_step_s = shared_v;
		*/
	}
	bool CHandleTunnelLoop::_shorten_double()
	{
		for (int i = 0; i < current_loop_edges.size(); i++)
		{
			M::CEdge* edg1 = current_loop_edges[i % current_loop_edges.size()];
			M::CEdge* edg2 = current_loop_edges[(i + 1) % current_loop_edges.size()];
			M::CEdge* edg3;
			std::vector<M::CFace*> vec1 = edges_faces[edg1->idx()];
			std::vector<M::CFace*> vec2 = edges_faces[edg2->idx()];
			_intersection(vec1, vec2);
			if (face_intersection.size() == 1)
			{
				M::CFace* fac1 = face_intersection[0];
				for (M::FaceEdgeIterator feiter(fac1); !feiter.end(); ++feiter)
				{
					M::CEdge* pE = *feiter;
					if (pE != edg1 && pE != edg2)
					{
						edg3 = pE;
						break;
					}
				}
				// find center of edg3
				M::CVertex* pV = m_pMesh->edge_vertex(edg3, 0);
				M::CVertex* pW = m_pMesh->edge_vertex(edg3, 1);
				double l1 = (pV->point() - pW->point()).norm();
				// find lengths of edge1 and edge 2;
				M::CVertex* pV1 = m_pMesh->edge_vertex(edg1, 0);
				M::CVertex* pW1 = m_pMesh->edge_vertex(edg1, 1);
				M::CVertex* pV2 = m_pMesh->edge_vertex(edg2, 0);
				M::CVertex* pW2 = m_pMesh->edge_vertex(edg2, 1);



				/*bool bad = false;
				for (std::pair<M::CFace*, M::CEdge*> pair2 : single_to_double)
				{
					if (fac1 == pair2.first && edg3 == pair2.second)
					{
						bad = true;
						break;
					}
				}
				if (bad)
				{
					continue;
				}
				std::vector<M::CEdge*> vec3;
				std::vector<M::CEdge*> vec4;
				vec3.push_back(edg1);
				vec3.push_back(edg2);
				vec4.push_back(edg2);
				vec4.push_back(edg1);
				bad = false;
				for (std::pair<M::CFace*, std::vector<M::CEdge*>> pair2 : double_to_single)
				{
					if (fac1 == pair2.first)
					{
						if (vec3 == pair2.second || vec4 == pair2.second)
						{
							bad = true;
							break;
						}
					}
				}
				if (bad)
				{
					continue;
				}*/

				M::CVertex* shared_v;
				CPoint p2;
				if (pV1 == pV2 || pV1 == pW2)
				{
					p2 = pV1->point();
					shared_v = pV1;
				}
				else if (pW1 == pV2 || pW1 == pW2)
				{
					p2 = pW1->point();
					shared_v = pW1;
				}
				else
				{
					std::cout << "ERROR: edges share a face but don't share a vertex-----------------------";
				}
				//std::cout << "made a change!\n";
				// make the change, since p1 is closer to center of mass than p2
				// update center of mass by removing p2;
				center_of_mass *= loop_vertices.size();
				center_of_mass -= p2;
				center_of_mass /= double(loop_vertices.size() - 1);
				// update vertex list by removing shared_v;
				std::vector<M::CEdge*> two_edges;
				two_edges.push_back(edg1);
				two_edges.push_back(edg2);
				std::pair<M::CFace*, std::vector<M::CEdge*>> this_pair = std::make_pair(fac1, two_edges);
				double_to_single.push_back(this_pair);
				loop_vertices.erase(std::remove(loop_vertices.begin(), loop_vertices.end(), shared_v), loop_vertices.end());
				// update edge list by removing edg1 and edg2 but adding edg3 at position i;
				current_loop_edges.erase(std::remove(current_loop_edges.begin(), current_loop_edges.end(), edg1), current_loop_edges.end());
				current_loop_edges.erase(std::remove(current_loop_edges.begin(), current_loop_edges.end(), edg2), current_loop_edges.end());
				if (idx_best_edge_o1 >= current_loop_edges.size())
				{
					current_loop_edges.push_back(edg3);
				}
				else
				{
					current_loop_edges.insert(current_loop_edges.begin() + i, edg3);
				}
				return true;

			}
		}
		return false;
	}
	void CHandleTunnelLoop::_intersection(std::vector<M::CFace*> v1, std::vector<M::CFace*> v2)
	{
		face_intersection.clear();

		for (M::CFace* f : v1)
		{
			if (std::find(v2.begin(), v2.end(), f) != v2.end())
			{
				face_intersection.push_back(f);
				break;
			}
		}
	}
	int CHandleTunnelLoop::_intersection(M::CEdge* e1, M::CEdge* e2, M::CEdge* e3)
	{
		std::vector<M::CVertex*> v1;
		v1.push_back(m_pMesh->edge_vertex(e1, 0));
		v1.push_back(m_pMesh->edge_vertex(e1, 1));
		std::vector<M::CVertex*> v2;
		if (e2 == NULL)
		{
			std::cout << "this edge was null!?\n";
			return -1;
		}
		//std::cout << e2->idx() << "\n"; // it gives a null edge?
		v2.push_back(m_pMesh->edge_vertex(e2, 0));
		v2.push_back(m_pMesh->edge_vertex(e2, 1));
		std::vector<M::CVertex*> v3;
		v3.push_back(m_pMesh->edge_vertex(e3, 0));
		v3.push_back(m_pMesh->edge_vertex(e3, 1));

		M::CVertex* inter1 = NULL;
		M::CVertex* inter2 = NULL;
		for (M::CVertex* v : v1)
		{
			if (std::find(v2.begin(), v2.end(), v) != v2.end())
			{
				inter1 = v;
				break;
			}
		}
		for (M::CVertex* v : v1)
		{
			if (std::find(v3.begin(), v3.end(), v) != v3.end())
			{
				inter2 = v;
				break;
			}
		}
		if (inter1 != NULL && inter2 != NULL && inter1 == inter2)
		{
			return 0;
		}
		else if (inter1 == NULL)
		{
			
			//std::cout << "there's a hole here!!!\n";
			return 1;
		}
		else if (inter2 == NULL)
		{

			//std::cout << "there's a hole here!!!\n";
			return 2;
		}
		return -1;
	}
	int CHandleTunnelLoop::_intersection(M::CEdge* e1, M::CEdge* e2)
	{
		std::vector<M::CVertex*> v1;
		v1.push_back(m_pMesh->edge_vertex(e1, 0));
		v1.push_back(m_pMesh->edge_vertex(e1, 1));
		std::vector<M::CVertex*> v2;
		v2.push_back(m_pMesh->edge_vertex(e2, 0));
		v2.push_back(m_pMesh->edge_vertex(e2, 1));

		M::CVertex* inter1 = NULL;
		for (M::CVertex* v : v1)
		{
			if (std::find(v2.begin(), v2.end(), v) != v2.end())
			{
				inter1 = v;
				break;
			}
		}
		if (inter1 == NULL)
		{

			//std::cout << "there's a gap here!!!\n";
			return 0;
		}
		return -1;
	}
	bool CHandleTunnelLoop::_different_side(M::CVertex* A, M::CVertex* B, CPoint M, M::CVertex* C)
	{
		//std::cout << A->point().print() << " " << B->point().print() << " " << M.print() << " " << C->point().print() << "\n";
		// line segment AB, center of mass M, third point C.
		CPoint line_seg1 = A->point() - B->point(); // vector AB;
		CPoint N = line_seg1 * ((line_seg1 * (M - B->point())) / (line_seg1 * line_seg1));
		//std::cout << "M - B is " << (M - B->point()).print() << "\n";
		//std::cout << "C - B is " << (C->point() - B->point()).print() << "\n";
		//std::cout << "N is " << N.print() << "\n";
		//std::cout << "line_seg1 is " << line_seg1.print() << "\n";
		CPoint X = (M - B->point()) - N;
		//std::cout << "X is " << X.print() << "\n";
		double Y = N * X;
		//std::cout << "Y is " << Y << "\n";
		double M_side = (X * (M - B->point()))/* / Y - 1.0*/;
		double C_side = (X * (C->point() - B->point()))/* / Y - 1.0*/;
		//std::cout << M_side << " " << C_side << "\n";
		if ((M_side > 0 && C_side > 0) || (M_side < 0 && C_side < 0))
		{
			//if ((C_side < epsi && C_side > 0) || (C_side > (-1 * epsi) && C_side < 0))
			//{
			//	return true;
			//}
			return false;
		}
		else
		{
			//if ((C_side < epsi && C_side > 0) || (C_side > (-1 * epsi) && C_side < 0))
			//{
			//	return false;
			//}
			return true;
		}


	}
	bool CHandleTunnelLoop::_null_homologous(std::vector<M::CEdge*> myEdges)
	{
		return true; // just pick the largest one at the end
		if (trickShorten)
		{
			if (myEdges.size() < 4)
			{
				return true;
			}
			int constant = 3;
			if (m_boundary_edges.size() > 100000)
			{
				constant = 1;
			}
			if (loop_edges.size() > 20)
			{
				if (myEdges.size() < loop_edges.size() / (10 / constant))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else if (loop_edges.size() > 10)
			{
				if (myEdges.size() < loop_edges.size() / (5.0 / constant))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				if (myEdges.size() < loop_edges.size() / 1.5)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			
		}
		if (myEdges.size() <= 2)
		{
			std::cout << "Checked an empty loop. It was broken.\n";
			return true;
		}
		else if (myEdges.size() == 3)
		{
			std::cout << "This loop has 3 edges, so it is null-homologous\n";
			return true;
		}
		int safety_c = 0;
		inSet.clear();
		inSetGens.clear();
		int number = 0;
		int gnumber = 0;
		for (M::CEdge* ed : myEdges)
		{
			inSet.insert(ed->idx());
			number += 1;
			if (ed->generator())
			{
				inSetGens.insert(ed->idx());
				gnumber += 1;
			}
		}
		M::CEdge* phead = idx_edges[*inSetGens.rbegin()];
		if (phead == NULL)
		{
			std::cout << "There were no edges in the generators list\n";
			return true;
		}
		while (number > 0 /*&& phead != NULL*/ && phead->pair() != NULL)
		{
			M::CFace* pF = phead->pair();
			if (safety_c >= 10000)
			{
				break;
			}
			safety_c += 1;
			for (M::FaceEdgeIterator feiter(pF); !feiter.end(); ++feiter)
			{
				M::CEdge* ed = *feiter;
				if (inSet.find(ed->idx()) != inSet.end())
				{
					inSet.erase(ed->idx());
					number -= 1;
				}
				else
				{
					inSet.insert(ed->idx());
					number += 1;
				}
				if (ed->generator())
				{
					if (inSetGens.find(ed->idx()) != inSetGens.end())
					{
						inSetGens.erase(ed->idx());
						gnumber -= 1;
					}
					else
					{
						inSetGens.insert(ed->idx());
						gnumber += 1;
					}

				}


			}
			if (gnumber > 0)
			{
				phead = idx_edges[*inSetGens.rbegin()];
			}
			else
			{
				std::cout << "ran out of generators\n";
				phead = NULL;
			}
			
		}
		/*while (number > 0 && phead != NULL && phead->pair() != NULL)
		{
			M::CFace* pF2 = phead->pair();
			for (int edge1 : facesPair[pF2])
			{
				if (inSet.find(edge1) != inSet.end())
				{
					inSet.erase(edge1);
					number -= 1;
				}
				else
				{
					inSet.insert(edge1);
					number += 1;
				}
				if (idx_edges[edge1]->generator())
				{
					if (inSetGens.find(edge1) != inSetGens.end())
					{
						inSetGens.erase(edge1);
						gnumber -= 1;
					}
					else
					{
						inSetGens.insert(edge1);
						gnumber += 1;
					}
				}
			}
	

			if (gnumber > 0)
			{
				phead = idx_edges[*inSetGens.rbegin()];
			}
			else
			{
				phead = NULL;
			}
		}*/
		if (safety_c >= 10000)
		{
			std::cout << "we used the safety!!---------\n";
			return false;
		}
		std::cout << "number is " << number << " while gnumber is " << gnumber << "\n";
		return number <= 0;

	}
}
