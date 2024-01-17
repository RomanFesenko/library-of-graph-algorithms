/** @file tree_search.h
 *  tree_search definition;
*/

#ifndef _tree_search_
#define _tree_search_

#include <vector>
#include <optional>
#include <algorithm>

#include "graph_traits.h"
#include "graph_utility.h"
#include "search_settings.h"
#include "node_property.h"

namespace liga{

enum  node_state_t: char
{
    undiscovered_id=0,
    discovered_id,
    closed_id
};
/**
 *  @brief A standard result of few graph algorithm.
 *  For all nodes define state,label, previous nodes and
 *  previous edge on direct path from tree root to node.
*/

template<class graph_t,class l_t=graph_traits<graph_t>::weight_t>
class tree_search_t
{
    using size_t=std::size_t;

    template<class g_t,class s_t>
    friend search_termination_t
    breadth_search(bool,const g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t,size_t>&);

    template<class g_t,class s_t>
    friend search_termination_t
    depth_search(bool,const g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t,std::pair<size_t,size_t>>&);

    template<class g_t,class c_t>
    friend std::size_t
    scc_tarjan(const g_t&,const c_t&,node_property_t<g_t,std::optional<typename graph_traits<g_t>::node_handler_t>>&);

    template<class g_t,class c_t>
    friend std::size_t
    scc_kosaraju(const g_t&,const c_t&,node_property_t<g_t,std::optional<typename graph_traits<g_t>::node_handler_t>>&);

    template<class g_t,class s_t,class u_t>
    friend bool
    dag_priority_search(const g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t>&tree,u_t);

    template<class g_t,class s_t,class u_t,class q_t>
    friend search_termination_t
    __priority_search(const g_t&g,typename graph_traits<g_t>::node_handler_t,s_t,tree_search_t<g_t>&,u_t,q_t&);

    template<class g_t,class e_t,class w_upd_t>
    friend std::optional<typename graph_traits<g_t>::node_handler_t>
    __priority_floyd_warshall(const g_t&,
                              const std::vector<typename graph_traits<g_t>::node_handler_t>&,
                              node_property_t<g_t,tree_search_t<g_t>>&,
                              e_t,
                              w_upd_t );

    template<class g_t,class f_t,class u_t>
    friend std::optional<typename graph_traits<g_t>::node_handler_t>
    __priority_bellman_ford(const g_t&graph,
                            typename graph_traits<g_t>::node_handler_t,
                            tree_search_t<g_t>&,
                            f_t,
                            u_t);

    public:
    using traits=  graph_traits<graph_t>;
    using node_t=  traits::node_handler_t;
    using edge_t=  traits::edge_handler_t;
    using label_t= l_t;
    using path_t=std::vector<std::pair<node_t,edge_t>>;

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
    /**
     *  Returns true if  node belong this tree.
     */
    bool in_tree(node_t node)const
    {
        size_t index=traits::index(*m_graph,node);
        return index<m_state_s.size()? m_state_s[index]!=node_state_t::undiscovered_id : false;
    }
    /**
     *  Returns true if node is root of this tree.
     */
    bool is_root(node_t node)const
    {
        return in_tree(node)&&is_equal(*m_graph,node,m_prev_node_s[traits::index(*m_graph,node)]);
    }
    /**
     *  Returns state of node.
     */
    node_state_t state(node_t node)const
    {
        size_t index=traits::index(*m_graph,node);
        return index<m_state_s.size()? m_state_s[index]:node_state_t::undiscovered_id;
    }
    /**
     *  Returns previous node on the path from root to node.
     */
    node_t prev_node(node_t node)const
    {
        return m_prev_node_s[traits::index(*m_graph,node)];
    }
    /**
     *  Returns previous edge on the path from root to node.
     *  Condition traits::target(*m_graph,prev_node,prev_edge)==node is satisfied.
     */
    edge_t prev_edge(node_t node)const
    {
        return m_prev_edge_s[traits::index(*m_graph,node)];
    }
    /**
     *  Returns label of tree node. Can be:
     *  -distance to this node if tree is result of shortest paths algorithm.
     *  -pair{time.first,time.last} for depth search algorithm.
     */
    label_t dist(node_t node)const
    {
        return m_dists_s[traits::index(*m_graph,node)];
    }
    /**
     *  @brief  Traverse path from node to tree root.
     *  @param to    Start node.
     *  @param func  Functor,which received path edges.
     *  @return      True if node contained in tree.
    */
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
    bool path_from_root(node_t to,path_t&p)const
    {
        if(!in_tree(to)) return false;
        p.clear();
        traverse_path(to,[&](node_t n,edge_t e){p.push_back({n,e});});
        std::reverse(p.begin(),p.end());
        return true;
    }
    /**
     *  Return label of the node if it have closed state,
     *  otherwise return empty optional.
    */
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
    /**
     *  @brief  Find cycle, if it exist or empty vector.
     *  @param provoke  One node of this cycle OR reachable from it.
     *  @return         Vector of cycle's edges.
    */
    path_t find_cycle(node_t provoke )const
    {
        assert(in_tree(provoke));
        assert(!is_root(provoke));
        node_property_t<graph_t,char> mark(*m_graph,0);
        path_t res,stack;
        mark.ref(provoke)=1;
        stack.push_back({prev_node(provoke),prev_edge(provoke)});
        while(true)
        {
            auto [node,edge]=stack.back();
            if(mark(node))
            {
                node_t target;
                do
                {
                    assert(!stack.empty());
                    res.push_back(stack.back());
                    stack.pop_back();
                    target=traits::target(*m_graph,res.back().first,res.back().second);
                }
                while(!is_equal(*m_graph,target,node));

                return res;
            }
            else if(is_root(node))
            {
                return {};
            }
            else
            {
                mark.ref(node)=1;
                stack.push_back({prev_node(node),prev_edge(node)});
            }
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

}// liga

#endif






















