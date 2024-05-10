/** @file tree_search.h
 *  tree_search definition;
*/

#ifndef _liga_tree_search_
#define _liga_tree_search_

#include <vector>
#include <optional>
#include <algorithm>

#include "search_settings.h"
#include "property_map.h"

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

template<class _node_t,class _edge_t,class _label_t,class _index_map_t>
class tree_search_t
{
    using size_t=std::size_t;

    template<class graph_t,class search_t,class index_map_t>
    friend search_termination_t
    breadth_search(bool,
                   graph_t,
                   typename graph_t::node_handler_t,
                   search_t&&,
                   tree_search_t<typename graph_t::node_handler_t,
                                 typename graph_t::edge_handler_t,
                                 std::size_t,
                                 index_map_t>&);


    template<class graph_t,class search_t,class index_map_t>
    friend search_termination_t
    depth_search(bool,
                 graph_t,
                 typename graph_t::node_handler_t,
                 search_t&&,
                 tree_search_t<typename graph_t::node_handler_t,
                               typename graph_t::edge_handler_t,
                               std::pair<std::size_t,std::size_t>,
                               index_map_t>&);

    template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t,class upd_t>
    friend bool
    dag_priority_search(graph_t,
                        typename graph_t::node_handler_t sourse,
                        search_t&&,
                        tree_search_t<typename graph_t::node_handler_t,
                                      typename graph_t::edge_handler_t,
                                      weight_t,
                                      index_map_t>&,
                         weight_map_t,
                         upd_t);

    template<class graph_t,class cont_t,class index_map_t>
    friend std::size_t
    scc_tarjan(graph_t,
               const cont_t&,
               property_map_t<typename graph_t::node_handler_t,
                              std::optional<typename graph_t::node_handler_t>,
                              index_map_t>&);

    template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t,class upt_t,class queue_t>
    friend search_termination_t
    __priority_search(graph_t,
                      typename graph_t::node_handler_t,
                      search_t&&,
                      tree_search_t<typename graph_t::node_handler_t,
                                    typename graph_t::edge_handler_t,
                                    weight_t,
                                    index_map_t>&,
                      weight_map_t,
                      upt_t er,
                      queue_t&);

    template<class graph_t,class cont_t,class weight_t,class index_map_t,class weight_map_t,class efilter_t,class w_upd_t>
    friend std::optional<typename graph_t::node_handler_t>
    __priority_floyd_warshall(graph_t,
                              const cont_t&,
                              property_map_t<typename graph_t::node_handler_t,
                                             tree_search_t<typename graph_t::node_handler_t,
                                                           typename graph_t::edge_handler_t,
                                                           weight_t,
                                                           index_map_t>,
                                             index_map_t>&,
                              weight_map_t,
                              efilter_t,
                              w_upd_t);

    template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class efilter_t,class w_upd_t>
    friend std::optional<typename graph_t::node_handler_t>
    __priority_bellman_ford(graph_t,
                            typename graph_t::node_handler_t sourse,
                            tree_search_t<typename graph_t::node_handler_t,
                                          typename graph_t::edge_handler_t,
                                          weight_t,
                                          index_map_t>&,
                            weight_map_t,
                            efilter_t,
                            w_upd_t);

    template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class upt_t,class hfun_t,class queue_t,class efilter_t>
    friend bool
    __astar_priority_search(graph_t,
                            typename graph_t::node_handler_t,
                            typename graph_t::node_handler_t,
                            tree_search_t<typename graph_t::node_handler_t,
                                          typename graph_t::edge_handler_t,
                                          weight_t,
                                          index_map_t>&,
                            property_map_t<typename graph_t::node_handler_t,
                                           weight_t,
                                           index_map_t>&,
                            property_map_t<typename graph_t::node_handler_t,
                                           weight_t,
                                           index_map_t>&,
                            weight_map_t,
                            upt_t,
                            hfun_t,
                            queue_t&,
                            efilter_t);

    public:
    using node_t=      _node_t;
    using edge_t=      _edge_t;
    using label_t=     _label_t;
    using index_map_t= _index_map_t;
    using path_t=std::vector<std::pair<node_t,edge_t>>;

    private:
    index_map_t                m_index_map;
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
        m_resize(m_index_map(node)+1);
    }
    node_t&m_prev_node(node_t node)
    {
        return m_prev_node_s[m_index_map(node)];
    }
    edge_t&m_prev_edge(node_t node)
    {
        return m_prev_edge_s[m_index_map(node)];
    }
    label_t&m_dist(node_t node)
    {
        return m_dists_s[m_index_map(node)];
    }
    node_state_t&m_state(node_t node)
    {
        return m_state_s[m_index_map(node)];
    }
    public:
    tree_search_t(index_map_t imap={}):m_index_map(imap)
    {
    }

    bool is_equal(node_t n1,node_t n2)const
    {
        return m_index_map(n1)==m_index_map(n2);
    }
    index_map_t&index_map(){return m_index_map;}
    const index_map_t&index_map()const{return m_index_map;}
    /**
     *  Returns true if  node belong this tree.
     */
    bool in_tree(node_t node)const
    {
        size_t index=m_index_map(node);
        return index<m_state_s.size()? m_state_s[index]!=node_state_t::undiscovered_id:false;
    }
    /**
     *  Returns state of node.
     */
    node_state_t state(node_t node)const
    {
        size_t index=m_index_map(node);
        return index<m_state_s.size()? m_state_s[index]:node_state_t::undiscovered_id;
    }
    /**
     *  Returns previous node on the path from root to node.
     */
    node_t prev_node(node_t node)const
    {
        return m_prev_node_s[m_index_map(node)];
    }
    /**
     *  Returns previous edge on the path from root to node.
     *  Condition traits::target(*m_graph,prev_node,prev_edge)==node is satisfied.
     */
    edge_t prev_edge(node_t node)const
    {
        return m_prev_edge_s[m_index_map(node)];
    }
    /**
     *  Returns label of tree node. Can be:
     *  -distance to this node if tree is result of shortest paths algorithm.
     *  -pair{time.first,time.last} for depth search algorithm.
     */
    label_t dist(node_t node)const
    {
        return m_dists_s[m_index_map(node)];
    }
    /**
     *  Returns true if node is root of this tree.
     */
    bool is_root(node_t node)const
    {
        return in_tree(node)&&is_equal(node,prev_node(node));
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
            if(is_equal(prev,to)) break;
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
        size_t index=m_index_map(node);
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
    template<class graph_t>
    path_t find_cycle(const graph_t&graph,node_t provoke)const
    {
        assert(in_tree(provoke));
        assert(!is_root(provoke));
        property_map_t<node_t,char,index_map_t> mark(0,m_index_map);
        path_t res,stack;
        mark.ref(provoke)=1;
        stack.push_back({prev_node(provoke),prev_edge(provoke)});
        while(true)
        {
            auto [node,edge]=stack.back();
            if(mark.cref(node))
            {
                node_t target;
                do
                {
                    assert(!stack.empty());
                    res.push_back(stack.back());
                    stack.pop_back();
                    target=graph.target(res.back().first,res.back().second);
                }
                while(!is_equal(target,node));

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
    }
    void drop()
    {
        std::fill(m_state_s.begin(),m_state_s.end(),node_state_t::undiscovered_id);
    }

};

}// liga

#endif






















