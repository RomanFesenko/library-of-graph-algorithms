

/** @file bellman_ford.h
 *  Content:
 *  bellman_ford_shortest_path
 *  bellman_ford_negative_cycle
*/

#ifndef _bellman_ford_
#define _bellman_ford_

#include <vector>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "graph_traits.h"
#include "tree_search.h"

namespace liga{

template<class graph_t,class efilter_t,class w_upd_t>
std::optional<typename graph_traits<graph_t>::node_handler_t>
__priority_bellman_ford(const graph_t&gph,
                        typename graph_traits<graph_t>::node_handler_t sourse,
                        tree_search_t<graph_t>&tree,
                        efilter_t efilter,
                        w_upd_t updater)
{
    using gts=graph_traits<graph_t>;
    using node_t=gts::node_handler_t;
    using edge_t=gts::edge_handler_t;
    using weight_t=gts::weight_t;
    using size_t=std::size_t;

    tree.m_init(gph);
    tree.m_resize_for_node(sourse);
    std::vector<node_t> queue,aux_queue;
    node_property_t<graph_t,char> in_queue(gph,0);

    auto push_to_queue=[&](node_t node,node_t p_node,edge_t p_edge,weight_t dst)
    {
        tree.m_state(node)=node_state_t::discovered_id;
        tree.m_prev_node(node)=p_node;
        tree.m_prev_edge(node)=p_edge;
        tree.m_dist(node)=dst;
        if(!in_queue(node))
        {
            queue.push_back(node);
            in_queue.ref(node)=1;
        }
    };
    auto pop_from_queue=[&]()
    {
        node_t node=aux_queue.back();
        aux_queue.pop_back();
        tree.m_state(node)=node_state_t::closed_id;
        in_queue.ref(node)=0;
        return node;
    };
    push_to_queue(sourse,sourse,edge_t{},updater.init_weight());
    size_t num_detected_nodes=1;
    for(size_t edge_in_path=0;!queue.empty();++edge_in_path)
    {
        assert(num_detected_nodes>edge_in_path);
        std::swap(queue,aux_queue);
        queue.clear();
        while(!aux_queue.empty())
        {
            node_t frm=pop_from_queue();
            for(edge_t e=gts::edge_begin(gph,frm);!gts::edge_end(gph,frm,e);gts::edge_inc(gph,frm,e))
            {
                if(!efilter(frm,e)) continue;
                node_t target=gts::target(gph,frm,e);
                tree.m_resize_for_node(target);
                weight_t new_dist=updater.weight_update(tree.m_dist(frm),gts::weight(gph,frm,e));
                if(auto s=tree.m_state(target);s==node_state_t::undiscovered_id)
                {
                    ++num_detected_nodes;
                    push_to_queue(target,frm,e,new_dist);
                }
                else if(updater.priority(new_dist,tree.m_dist(target)))
                {
                    push_to_queue(target,frm,e,new_dist);
                    if(edge_in_path==num_detected_nodes-1) return target;
                }
            }
        }
    }
    return {};
}

/**
 *  @brief  Find generic Shortest paths from one node using Bellman-Ford algorithm.
 *  @param  graph        Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  source       Any node of graph.
 *  @param  tree         Tree of shortest paths.
 *  @param  efilter      Predictor- filter of edges.
 *  @param  updater      Weight updater,that satisfy weight_updater_cpt concept.
 *  @return              True if graph not content negative cycle.
 *
*/

template<class graph_t,class efilter_t,class w_upd_t>
bool bellman_ford_shortest_path(const graph_t&graph,
                                typename graph_traits<graph_t>::node_handler_t sourse,
                                tree_search_t<graph_t>&tree,
                                efilter_t efilter,
                                w_upd_t updater)
{
    return !__priority_bellman_ford(graph,sourse,tree,efilter,updater);
}


template<class graph_t,class efilter_t=always_bool_t<true>>
bool bellman_ford_shortest_path(const graph_t&graph,
                                typename graph_traits<graph_t>::node_handler_t sourse,
                                tree_search_t<graph_t>&tree,
                                efilter_t filter=always_bool_t<true>{})
{
    return bellman_ford_shortest_path(graph,sourse,tree,filter,minimize_distance_t<graph_t>{});
}

/**
 *  @brief  Find generic negative cycle using Bellman-Ford algorithm.
 *  @param  graph        Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  source       Any node of graph.
 *  @param  efilter      Predictor- filter of edges.
 *  @param  updater      Weight updater,that satisfy weight_updater_cpt concept.
 *  @return              Sequence of cycle edges if it exist.
*/

template<class graph_t,class efilter_t,class w_upd_t>
tree_search_t<graph_t>::path_t
bellman_ford_negative_cycle(const graph_t&graph,
                            typename graph_traits<graph_t>::node_handler_t sourse,
                            efilter_t filter,
                            w_upd_t updater)
{
    using tree_t=tree_search_t<graph_t>;
    using path_t=tree_t::path_t;
    tree_t tree;
    auto provoke_node=__priority_bellman_ford(graph,sourse,tree,filter,updater);
    return provoke_node? tree.find_cycle(*provoke_node):path_t{};;
}

template<class graph_t,class efilter_t=always_bool_t<true>>
tree_search_t<graph_t>::path_t
bellman_ford_negative_cycle(const graph_t&graph,
                            typename graph_traits<graph_t>::node_handler_t sourse,
                            efilter_t filter=always_bool_t<true>{})
{
    return bellman_ford_negative_cycle(graph,sourse,filter,minimize_distance_t<graph_t>{});
}

}// liga

#endif






















