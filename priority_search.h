/** @file priority_search.h
 *  Content:
 *  dense_priority_search
 *  sparse_priority_search
*/

#ifndef _graph_priority_search_
#define _graph_priority_search_

#include <vector>
#include <optional>
#include <algorithm>

#include "priority_queue.h"
#include "search_settings.h"
#include "tree_search.h"

namespace liga{

template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t,class upt_t,class queue_t>
search_termination_t
__priority_search(graph_t graph,
                  typename graph_t::node_handler_t sourse,
                  search_t&&search,
                  tree_search_t<typename graph_t::node_handler_t,
                                typename graph_t::edge_handler_t,
                                weight_t,
                                index_map_t>&tree,
                  weight_map_t weight_map,
                  upt_t        updater,
                  queue_t&     queue)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    tree.drop();
    tree.m_resize_for_node(sourse);
    tree.m_state(sourse)=node_state_t::discovered_id;
    tree.m_dist(sourse)=updater.init_weight();
    tree.m_prev_node(sourse)=sourse;

    queue.clear();
    queue.push(sourse);
    while(!queue.empty())
    {
        node_t opt_node=queue.pop();
        tree.m_state(opt_node)=node_state_t::closed_id;
        if(!search.node_preprocess(opt_node)) return search_termination_t::node_preprocess_id;
        for(edge_t e=graph.edge_begin(opt_node);
                    !graph.edge_end(opt_node,e);
                     graph.edge_inc(opt_node,e))
        {
            if(!search.edge_filter(opt_node,e)) continue;
            node_t target=graph.target(opt_node,e);
            tree.m_resize_for_node(target);
            weight_t new_dist=updater.weight_update(tree.dist(opt_node),weight_map(opt_node,e));
            if(auto s=tree.state(target);s==node_state_t::undiscovered_id)
            {
                tree.m_dist(target)=new_dist;
                tree.m_prev_node(target)=opt_node;
                tree.m_prev_edge(target)=e;
                tree.m_state(target)=node_state_t::discovered_id;
                queue.push(target);
            }
            else if(s==node_state_t::discovered_id&&updater.priority(new_dist,tree.m_dist(target)))
            {
                tree.m_dist(target)=new_dist;
                tree.m_prev_node(target)=opt_node;
                tree.m_prev_edge(target)=e;
                queue.rebuild_after_mod(target);
            }
        }
    }
    return search_termination_t::complete_id;
}

/**
 *  @brief  Execute priority search in dense graph, using Dijkstra-like algorithm.
 *  @param  graph       Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  search      Search adapter, which satisfy priority_search_adapter_cpt.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @param  updater     Weight updater,that satisfy weight_updater_cpt concept.
 *  @return             Search termination cause.
 *
 * Complexity: O(V*V) in worst case.
 * Generic algorithm , in partial case can reduced to shortest-paths problem,
 * MST-problem, bottleneck graph problem and other.
 *
 * Formally, define two functions:
 * combine(weight, weight)-> weight
 * priority(weight, weight)-> boolean,
 * and weight of of path 0-...-[i-2]-[i-1]-[i] as
 * WP=combine(WE[i],combine(WE[i-1],combine(WE[i-2],..),..)),where WE[i] weight of [i-1]-[i] edge.
 *
 * Then algorithm find optimal path with weight WP_opt among all other paths with weight WP_other,
 * satisfied condition: priority(WP_other,WP_opt)==false
*/

template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t,class upd_t>
search_termination_t
dense_priority_search(graph_t graph,
                      typename graph_t::node_handler_t sourse,
                      search_t&&search,
                      tree_search_t<typename graph_t::node_handler_t,
                                    typename graph_t::edge_handler_t,
                                    weight_t,
                                    index_map_t>&tree,
                      weight_map_t weight_map,
                      upd_t        updater)
{
    using node_t=graph_t::node_handler_t;
    auto comp=[&](node_t n1,node_t n2)
    {
        return updater.priority(tree.dist(n1),tree.dist(n2));
    };
    array_priority_queue_t<node_t,decltype(comp)> queue(comp);
    return __priority_search(graph,sourse,std::forward<search_t>(search),tree,weight_map,updater,queue);
}


/**
 *  @brief  Execute priority search in sparse graph, using Dijkstra-like algorithm.
 *  @param  graph       Graph,that satisfy w_graph_cpt concept.
 *  @param  source      First node of all shortest paths.
 *  @param  search      Search adapter, which satisfy priority_search_adapter_cpt.
 *  @param  tree        Tree of shortest paths from source node,that build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @param  updater     Weight updater,that satisfy weight_updater_cpt concept.
 *  @return             Search termination cause.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t,class upd_t>
search_termination_t
sparse_priority_search(graph_t graph,
                       typename graph_t::node_handler_t sourse,
                       search_t&&search,
                       tree_search_t<typename graph_t::node_handler_t,
                                     typename graph_t::edge_handler_t,
                                     weight_t,
                                     index_map_t>&tree,
                       weight_map_t weight_map,
                       upd_t        updater)
{
    using node_t=graph_t::node_handler_t;
    auto comp=[&](node_t n1,node_t n2)
    {
        return updater.priority(tree.dist(n1),tree.dist(n2));
    };
    heap_priority_queue_t<node_t,decltype(comp),index_map_t> queue(comp,tree.index_map());
    return __priority_search(graph,sourse,std::forward<search_t>(search),tree,weight_map,updater,queue);
}

}// liga

#endif






















