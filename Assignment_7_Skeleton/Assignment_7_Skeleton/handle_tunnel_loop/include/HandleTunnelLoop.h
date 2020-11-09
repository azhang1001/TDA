#ifndef _DARTLIB_HANDLE_TUNNEL_LOOP_H_
#define _DARTLIB_HANDLE_TUNNEL_LOOP_H_

#include <stdio.h>
#include <list>
#include <set>
#include <queue>
#include <fstream>
#include <iostream>

#include "MyTMesh.h"
#include "MyMesh.h"

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
    
    void exact_boundary(S& surface);

    void prune();
  protected:
    void _extract_boundary_surface();

    void _extract_interior_volume();

    void _pair(std::set<M::CVertex*>& vertices);
    void _pair(std::set<M::CEdge*>& edges);
    void _pair(std::set<M::CFace*>& faces);

    void _mark_loop(M::CFace* face);
    void _mark_loop(M::CEdge* edge);

    bool _shrink_triangles();
    void _prune();
    
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
            // if (pv->idx() > max_id && pv->generator())
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