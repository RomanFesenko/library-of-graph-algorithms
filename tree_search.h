
#ifndef _tree_search_
#define _tree_search_

#include <vector>
#include <optional>
#include <algorithm>

#include "graph_traits.h"
#include "search_settings.h"

enum  node_state_t: char
{
    undiscovered_id=0,
    discovered_id,
    closed_id
};

template<class graph_t,class l_t=graph_traits<graph_t>::weight_t>
class tree_search_t
{
    using size_t=std::size_t;

    template<class g_t,class s_t>
    friend search_termination_t
    breadth_search(bool,g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t,size_t>&);

    template<class g_t,class s_t>
    friend search_termination_t
    depth_search(bool,g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t,std::pair<size_t,size_t>>&);

    template<class g_t,class s_t,class u_t>
    friend bool
    dag_priority_search(g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t>&tree,u_t);

    template<class g_t,class s_t,class u_t,class q_t>
    friend search_termination_t
    __priority_search(g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t>&,u_t,q_t&);

    template<class g_t,class f_t,class u_t>
    friend void
    priority_floyd_warshall(g_t&graph,const std::vector<typename graph_traits<g_t>::node_handler_t>&,
                            std::vector<tree_search_t<g_t>>&,f_t efilter,u_t);

    template<class g_t,class f_t,class u_t>
    friend bool
    priority_bellman_ford(std::size_t,g_t&graph,typename graph_traits<g_t>::node_handler_t,
                          tree_search_t<g_t>&,f_t,u_t);

    public:
    using traits=  graph_traits<graph_t>;
    using node_t=  traits::node_handler_t;
    using edge_t=  traits::edge_handler_t;
    using label_t= l_t;

    private:
    const graph_t*             m_graph=nullptr;
    std::vector<node_state_t>  m_state_s;
    std::vector<node_t>        m_prev_node_s;
    std::vector<edge_t>        m_prev_edge_s;
    std::vector<label_t>       m_dists_s;

    void m_resize(size_t i)
    {
        if(i>m_state_s.size())
        {
            m_state_s.resize(i,undiscovered_id);
            m_prev_node_s.resize(i,{});
            m_prev_edge_s.resize(i,{});
            m_dists_s.resize(i,{});
        }
    }
    void m_resize_for_node(node_t node)
    {
        m_resize(traits::index(*m_graph,node)+1);
    }
    void m_init(const graph_t&graph)
    {
        drop();
        m_graph=&graph;
    }
    bool m_is_equal(node_t n1,node_t n2)const
    {
        return traits::index(*m_graph,n1)==traits::index(*m_graph,n2);
    }
    node_t&m_prev_node(node_t node)
    {
        return m_prev_node_s[traits::index(*m_graph,node)];
    }
    edge_t&m_prev_edge(node_t node)
    {
        return m_prev_edge_s[traits::index(*m_graph,node)];
    }
    label_t&m_dist(node_t node)
    {
        return m_dists_s[traits::index(*m_graph,node)];
    }
    node_state_t&m_state(node_t node)
    {
        return m_state_s[traits::index(*m_graph,node)];
    }
    public:
    bool in_tree(node_t node)const
    {
        size_t index=traits::index(*m_graph,node);
        return index<m_state_s.size()? m_state_s[index]!=node_state_t::undiscovered_id : false;
    }
    node_state_t state(node_t node)const
    {
        size_t index=traits::index(*m_graph,node);
        return index<m_state_s.size()? m_state_s[index]:node_state_t::undiscovered_id;
    }
    node_t prev_node(node_t node)const
    {
        return m_prev_node_s[traits::index(*m_graph,node)];
    }
    edge_t prev_edge(node_t node)const
    {
        return m_prev_edge_s[traits::index(*m_graph,node)];
    }
    template<class func_t>
    bool traverse_path(node_t to,func_t func)const
    {
        if(!in_tree(to)) return false;
        for(;;)
        {
            node_t prev=prev_node(to);
            assert(in_tree(prev));
            if(m_is_equal(prev,to)) break;
            func(prev,prev_edge(to));
            to=prev;
        }
        return true;
    }
    bool path_from_root(node_t to,std::vector<std::pair<node_t,edge_t>>&p)const
    {
        p.clear();
        traverse_path(to,[&](node_t n,edge_t e){p.push_back({n,e});});
        std::reverse(p.begin(),p.end());
        return true;
    }
    label_t dist(node_t node)const
    {
        return m_dists_s[traits::index(*m_graph,node)];
    }
    std::optional<label_t> opt_dist(node_t node)const
    {
        size_t index=traits::index(*m_graph,node);
        if(index<m_state_s.size()&&m_state_s[index]==closed_id)
        {
            return m_dists_s[index];
        }
        else
        {
            return {};
        }
    }
    void clear()
    {
        m_state_s.clear();
        m_prev_node_s.clear();
        m_prev_edge_s.clear();
        m_dists_s.clear();
        m_graph=nullptr;
    }
    void drop()
    {
        std::fill(m_state_s.begin(),m_state_s.end(),node_state_t::undiscovered_id);
        m_graph=nullptr;
    }

};


#endif






















