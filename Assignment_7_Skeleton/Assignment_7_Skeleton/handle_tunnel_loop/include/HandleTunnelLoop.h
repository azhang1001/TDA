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

    void write_m(const std::string& output);
	void write_after_obj(const std::string& output);
	void write_good_after_obj(const std::string& output);
	void write_before_obj(const std::string& output);
	void write_before_ply(const std::string& output);
    
    void exact_boundary(S& surface);

    void prune();

	void shorten();
	void next_shorten_step();
	void go_back();
	void go_forward();

	void display_individual(int which_edge);

	void display_all_before();
	
	void display_all_after();

	void display_before(int which);

	void display_after(int which);

	void show_original();

	void show_starting(int which);

	void display_before_prune(int which);

	void display_loop(std::vector<int> l);
	void display_loop(std::vector<M::CVertex*> l);
	void display_loop(std::vector<M::CEdge*> l);
	

	int gcd(int a, int b);
	int lcm(int a, int b);



  protected:
	
    void _extract_boundary_surface();

    void _extract_interior_volume();

    void _pair(std::set<M::CVertex*>& vertices);
    void _pair(std::set<M::CEdge*>& edges);
    void _pair(std::set<M::CFace*>& faces);

    void _mark_loop(M::CFace* face);
    void _mark_loop(M::CEdge* edge);

    bool _shrink_triangles();

	void _shorten2();

	void _shorten();
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

    void _prune();

	int CHandleTunnelLoop::minDistance(std::vector<double> dist, std::vector<bool> sptSet);
	std::vector<int> CHandleTunnelLoop::dijkstra2(int src, int dest);
	std::vector<int> CHandleTunnelLoop::dijkstra(int s, int t);
    
	std::vector<int> remove_dup(std::vector<int> v);



  protected:
    M* m_pMesh;

    int m_genus;

    /* boundary surface */
    std::set<M::CVertex*> m_boundary_vertices;
    std::set<M::CEdge*>   m_boundary_edges;
    std::set<M::CFace*>   m_boundary_faces;

    /* interior volume */
    std::set<M::CVertex*> m_inner_vertices;
    std::set<M::CEdge*>   m_inner_edges;
    std::set<M::CFace*>   m_inner_faces;

    std::set<M::CEdge*>   m_generators;

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


	std::vector<std::vector<M::CEdge*>> before_prune_edges;
	std::vector<std::vector<M::CEdge*>> final_edges;
	std::vector<std::vector<M::CEdge*>> good_final_edges;
	std::vector<std::vector<M::CEdge*>> before_edges;
	std::vector<int> before_edges_search_size;
	std::vector<M::CEdge*> handletunnel_edges;
	std::vector<M::CEdge*> search_start_edges;
	std::vector<std::vector<int>> search_start_verts;
	std::vector<std::vector<int>> final_vertices;
	std::vector<std::vector<int>> good_final_vertices;
	std::vector<std::vector<int>> before_vertices;
	std::vector<M::CEdge*> green_edges;
	std::vector<std::vector<M::CFace*>> edges_faces;


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
			//if (pv->idx() > max_id && pv->generator())
            if (pv->idx() > max_id)
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