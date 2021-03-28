#ifndef _DARTLIB_HANDLE_TUNNEL_LOOP_H_
#define _DARTLIB_HANDLE_TUNNEL_LOOP_H_

#include <stdio.h>
#include <list>
#include <set>
#include <queue>
#include <fstream>
#include <iostream>
#include <map>
#include <algorithm>
#include <fstream>
#include <numeric>

#include "MyTMesh.h"
#include "MyMesh.h"

#define V 200000

namespace DartLib
{

class CHandleTunnelLoop
{
  public:

	  

    using M = CMyTMesh;
    using S = CMyMesh;

    CHandleTunnelLoop();

    CHandleTunnelLoop(M* pMesh);

    void set_mesh(M* pMesh);

    void boundary_surface_pair();

    void interior_volume_pair();

    std::set<M::CEdge*>& boundary_edges() { return m_boundary_edges; };

	void setExterior();
	void write_tets(const std::string& output);
	void write_m(const std::string& output);
	void write_boundary();
	void write_tunnels(const std::string& output);
	void write_shortened_tunnels(const std::string& output);
	void write_after_obj(const std::string& output);
	void write_good_after_obj(const std::string& output);
	void write_before_obj(const std::string& output);
	void write_before_ply(const std::string& output);
    
    void exact_boundary(S& surface);

    void prune();

	void remove_unconnected();
	void start_shorten();
	void start_shorten2();
	void shorten();
	void next_shorten_step();
	void go_back();
	void go_forward();

	void display_individual(int which_edge);

	void display_all_before_prune();
	void display_all_before();
	void display_all_unshortened();
	void display_all_middle();
	void display_all_after();
	void display_single_loop();

	void display_before(int which);
	void display_unshortened(int which);
	void display_middle(int which);
	void display_tested(int which);
	void display_after(int which);

	void show_original();

	void show_starting(int which);

	void display_pair(int i);
	void display_correct_loop(int w);
	void display_generated_loop(int w);
	void display_edges(std::vector<int> loop);
	void display_loop(std::vector<int> l);
	void display_loop(std::vector<M::CVertex*> l);
	void display_loop(std::vector<M::CEdge*> l);
	void add_tunnel(std::string line);
	void add_shortened_tunnel(std::string line);
	void add_tunnel_old(std::string line);

	int gcd(int a, int b);
	int lcm(int a, int b);

	void set_name(std::string name);
	void shorten_demo(int which);
	void find_connected_components();

  protected:
	
    void _extract_boundary_surface();

    void _extract_interior_volume();

    void _pair(std::set<M::CVertex*>& vertices);
    void _pair(std::set<M::CEdge*>& edges);
    void _pair(std::set<M::CFace*>& faces);

    void _mark_loop(M::CFace* face);
	void _mark_loop(M::CEdge* edge, M::CFace* face);
    void _mark_loop(M::CEdge* edge);
	void _mark_loop(std::set<int> hLoop);

    bool _shrink_triangles();
	int _shorten_single();
	int _shorten_single_final();
	void _shorten3();
	void _shorten2();
	bool _shorten(std::vector<M::CEdge*> ed_loop);
	bool _shorten();
	bool _delete_triple();
	double _check_double();
	bool _shorten_double();
	double _check_single();
	bool _fill_gaps(int index);
	bool _repeats(int index);
	bool _repeats2(int index);
	bool _triple0(int index);
	bool _triple1(int index);
	bool _triple2(int index);
	bool _double(int index);
	bool _single(int index);
	void _change_double();
	void _change_single();
	void _intersection(std::vector<M::CFace*> v1, std::vector<M::CFace*> v2);
	int _intersection(M::CEdge* e1, M::CEdge* e2, M::CEdge* e3);
	int _intersection(M::CEdge* e1, M::CEdge* e2);
	bool _different_side(M::CVertex* A, M::CVertex* B, CPoint M, M::CVertex* C);
	bool _null_homologous(std::vector<M::CEdge*> myEdges);
	

    void _prune();

	int CHandleTunnelLoop::minDistance(std::vector<double> dist, std::vector<bool> sptSet);
	std::vector<int> CHandleTunnelLoop::dijkstra2(int src, int dest);
	std::vector<int> CHandleTunnelLoop::dijkstra(int s, int t);
    
	std::vector<int> remove_dup(std::vector<int> v);
	bool trip = false;


  protected:
    M* m_pMesh;

	std::string file_name;

    int m_genus;
	bool exterior_volume = false;
	bool trickShorten = false;

	bool exterior_tunnel = false;
	bool boundary_shorten = false; // testing
	bool fastFacePairing = true;
    /* boundary surface */
    std::set<M::CVertex*> m_boundary_vertices;
    std::set<M::CEdge*>   m_boundary_edges;
    std::set<M::CFace*>   m_boundary_faces;

    /* interior volume */
    std::set<M::CVertex*> m_inner_vertices;
    std::set<M::CEdge*>   m_inner_edges;
    std::set<M::CFace*>   m_inner_faces;

	/* exterior volume */
	std::set<M::CVertex*> m_outer_vertices;
	std::set<M::CEdge*>   m_outer_edges;
	std::set<M::CFace*>   m_outer_faces;

    std::set<M::CEdge*>   m_generators;
	
	std::vector<std::vector<int>> new_tets;
	std::vector<std::vector<int>> all_tets;
	std::vector<std::vector<CPoint>> old_tets;
	std::vector<std::vector<M::CVertex*>> middle_vertices;
	std::vector<M::CVertex*> loop_vertices;
	std::vector<int> new_loop;
	std::vector<M::CEdge*> loop_edges;
	std::vector<M::CEdge*> current_loop_edges;
	std::vector<M::CEdge*> fall_back;
	std::vector<M::CFace*> face_intersection;
	CPoint center_of_mass;
	M::CVertex* last_step_d1;
	M::CVertex* last_step_d2;
	M::CVertex* last_step_s;
	std::vector<std::pair<M::CFace*, std::pair<M::CEdge*, CPoint>>> single_to_double;
	std::vector<std::pair<M::CFace*, std::vector<M::CEdge*>>> double_to_single;

	std::set<M::CEdge*> single_loop;
	std::set<M::CFace*> recently_used; 
	std::vector<std::vector<M::CEdge*>> before_prune_edges;
	std::vector<std::vector<M::CEdge*>> final_edges;
	std::vector<std::vector<M::CEdge*>> good_final_edges;
	std::vector<std::vector<M::CEdge*>> before_edges;
	std::vector<std::vector<M::CEdge*>> unshortened_edges;
	std::vector<std::vector<M::CEdge*>> after_edges;
	std::vector<std::vector<M::CEdge*>> tested_edges;
	std::vector<std::vector<M::CVertex*>> tested_vertices;
	std::vector<std::vector<M::CEdge*>> middle_edges;
	std::vector<int> before_edges_search_size;
	std::vector<M::CEdge*> handletunnel_edges;
	std::vector<M::CEdge*> search_start_edges;
	std::vector<std::vector<int>> search_start_verts;
	std::vector<std::vector<int>> final_vertices;
	std::vector<std::vector<M::CVertex*>> good_final_vertices;
	std::vector<std::vector<int>> before_vertices;
	std::vector<M::CEdge*> green_edges;
	std::vector<std::vector<M::CFace*>> edges_faces;
	std::vector<M::CFace*> killer_faces_list;
	
	std::map<int, CPoint> verts_O;
	//double graph[V][V];
	std::vector<std::map<int, double>> graph;
	std::vector<std::vector<std::pair<int, double>>> adj;
	std::vector<std::vector<int>> face_exist;
	std::vector<int> ignore_these;

	double old_dist = DBL_MAX;
	double new_dist = 0.0;

	std::vector<bool> in;//CHANGES HERE==============================================================================================================
	std::vector<M::CVertex*> idx_verts;
	std::vector<M::CVertex*> idx_all_verts;
	std::vector<M::CEdge*> idx_edges;
	std::vector<M::CFace*> idx_faces;
	std::set<int> inSet;
	std::set<int> inSetGens;
	int fails = 0;
	int MVERT = 4; //  minimum number of vertices used for searching;
	int index = 0;
	bool skip_singles = false;


	//variables for shortening in double and single
	
	double best_improve_o = 0.0;
	double equal_dist_imp = 0.0;

	M::CFace* best_face_o;

	M::CEdge* best_edge_o1;
	M::CEdge* best_edge_o2;
	int idx_best_edge_o1;
	int idx_best_edge_o2;

	M::CEdge* best_edge_o;
	int idx_best_edge_o;

	double epsi = 0.00005;


	std::vector<M::CFace*> best_face_os;

	std::vector<M::CEdge*> best_edge_o1s;
	std::vector<M::CEdge*> best_edge_o2s;
	std::vector<int> idx_best_edge_o1s;
	std::vector<int> idx_best_edge_o2s;

	std::vector<M::CEdge*> best_edge_os;
	std::vector<int> idx_best_edge_os;

	M::CEdge* current_edge_to_green;

	int stopped_naturally = 0;
	int time = 0;
	int paired_generators = 0;
	bool skip_this_one = false;

	std::vector<std::pair<M::CEdge*, M::CFace*>> pairing_information;
	std::vector<std::set<int>> pairing_loop;
	std::vector<std::set<int>> handle_loop_edges;
	std::vector<M::CEdge*> m_handle_gens;
	M::CEdge* current_generator;
	std::vector<std::pair<M::CEdge*, std::set<M::CEdge*>>> generated_edge_loops;
	std::vector<std::pair<M::CEdge*, std::vector<M::CEdge*>>> unkilled_generated_edge_loops;
	std::map<M::CEdge*, std::set<int>> edgesPair;
	std::map<M::CFace*, std::set<int>> facesPair;

	std::unordered_map<std::string, int> pointVertex;
	std::unordered_map<std::string, int> pointsEdge;
	bool first_time = true;

	std::unordered_map<M::CVertex*, M::CVertex*> oppositeVertex;
	int shortening_index = -1;
};

template <typename T>
class Compare
{
  public:
    bool operator()(T*& t1, T*& t2)
    {
        if (t1->idx() < t2->idx())
            return true;
        return false;
    }
};

template <class T, class C>
class Cycle
{
  public:
    T* head()
    {
        T* phead = NULL;
        int max_id = -1;
        typename std::list<T*>::iterator iter;

        for (typename std::list<T*>::iterator viter = m_cycle.begin(); viter != m_cycle.end(); viter++)
        {
            T* pv = *viter;
			if (pv->idx() > max_id && pv->generator())
            //if (pv->idx() > max_id)
            {
                phead = pv;
                max_id = pv->idx();
            }
        }

        // if (phead != NULL) m_cycle.remove(phead);
        return phead;
    }

    void add(T* pw)
    {
        bool inside = false;

        for (typename std::list<T*>::iterator viter = m_cycle.begin(); viter != m_cycle.end(); viter++)
        {
            T* pv = *viter;
            if (pv->idx() == pw->idx())
            {
                inside = true;
                break;
            }
        }

        if (inside)
            m_cycle.remove(pw);
        else
            m_cycle.push_back(pw);
    };

	/*void add(T* pw)
	{
		if (in[pw->idx()] == true)
		{
			//std::cout << "removed " << pw->idx() << "\n";
			m_cycle.remove(pw);
			in[pw->idx()] = false;
		}
		else
		{
			m_cycle.push_back(pw);
			//std::cout << "created " << pw->idx() << "\n";
			in[pw->idx()] = true;
		}
	};*/

    bool empty() { return m_cycle.empty(); };

    void print()
    {
        for (typename std::list<T*>::iterator viter = m_cycle.begin(); viter != m_cycle.end(); viter++)
        {
            T* pv = *viter;
            std::cout << pv->idx() << " ";
        }
        std::cout << std::endl;
    }

  protected:
    std::list<T*> m_cycle;
};

};     // namespace DartLib
#endif //!_DARTLIB_HANDLE_TUNNEL_LOOP_H_