/** @file astar.h
 *  Content:
 *  dense_astar_priority_search
 *  sparse_astar_priority_search
 *  dense_astar_shortest_path
 *  sparse_astar_shortest_path
*/

#ifndef _liga_astar_search_
#define _liga_astar_search_

#include <vector>
#include <optional>
#include <algorithm>

#include "priority_queue.h"
#include "search_settings.h"
#include "tree_search.h"

namespace liga{

template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class upt_t,class hfun_t,class queue_t,class efilter_t=always_bool_t<true>>
bool __astar_priority_search(graph_t graph,
                             typename graph_t::node_handler_t sourse,
                             typename graph_t::node_handler_t dest,
                             tree_search_t<typename graph_t::node_handler_t,
                                           typename graph_t::edge_handler_t,
                                           weight_t,
                                           index_map_t>&tree,
                             property_map_t<typename graph_t::node_handler_t,
                                            weight_t,
                                            index_map_t>&part_estimate,
                             property_map_t<typename graph_t::node_handler_t,
                                            weight_t,
                                            index_map_t>&total_estimate,
                             weight_map_t weight_map,
                             upt_t updater,
                             hfun_t hfunc,
                             queue_t&queue,
                             efilter_t efilter=always_bool_t<true>{})
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
        if(tree.is_equal(dest,opt_node)) return true;
        for(edge_t e=graph.edge_begin(opt_node);
                    !graph.edge_end(opt_node,e);
                     graph.edge_inc(opt_node,e))
        {
            if(!efilter(opt_node,e)) continue;
            node_t target=graph.target(opt_node,e);
            tree.m_resize_for_node(target);
            weight_t new_dist=updater.weight_update(tree.dist(opt_node),weight_map(opt_node,e));
            if(auto s=tree.state(target);s==node_state_t::undiscovered_id)
            {
                tree.m_dist(target)=new_dist;
                weight_t node_dest_dist=part_estimate.ref(target)=hfunc(target,dest);
                total_estimate.ref(target)=updater.weight_update(new_dist,node_dest_dist);
                tree.m_prev_node(target)=opt_node;
                tree.m_prev_edge(target)=e;
                tree.m_state(target)=node_state_t::discovered_id;
                queue.push(target);
            }
            else if(s==node_state_t::discovered_id&&updater.priority(new_dist,tree.m_dist(target)))
            {
                tree.m_dist(target)=new_dist;
                total_estimate.ref(target)=updater.weight_update(new_dist,part_estimate(target));
                tree.m_prev_node(target)=opt_node;
                tree.m_prev_edge(target)=e;
                queue.rebuild_after_mod(target);
            }
        }
    }
    return false;
}

template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class upt_t,class hfun_t,class efilter_t=always_bool_t<true>>
bool dense_astar_priority_search(graph_t graph,
                                 typename graph_t::node_handler_t sourse,
                                 typename graph_t::node_handler_t dest,
                                 tree_search_t<typename graph_t::node_handler_t,
                                               typename graph_t::edge_handler_t,
                                               weight_t,
                                               index_map_t>&tree,
                                 weight_map_t weight_map,
                                 upt_t updater,
                                 hfun_t hfunc,
                                 efilter_t efilter=always_bool_t<true>{})
{
    using node_t=graph_t::node_handler_t;

    property_map_t<node_t,weight_t,index_map_t> part_estimate(weight_t(0),tree.index_map());
    property_map_t<node_t,weight_t,index_map_t> total_estimate(weight_t(0),tree.index_map());
    auto comp=[&](node_t n1,node_t n2)
    {
        return updater.priority(total_estimate.cref(n1),total_estimate.cref(n2));
    };
    array_priority_queue_t<node_t,decltype(comp)> queue(comp);
    return __astar_priority_search(graph,
                                   sourse,
                                   dest,
                                   tree,
                                   part_estimate,
                                   total_estimate,
                                   updater,
                                   weight_map,
                                   hfunc,
                                   queue,
                                   efilter);
}


template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class upt_t,class hfun_t,class efilter_t=always_bool_t<true>>
bool sparse_astar_priority_search(graph_t graph,
                                  typename graph_t::node_handler_t sourse,
                                  typename graph_t::node_handler_t dest,
                                  tree_search_t<typename graph_t::node_handler_t,
                                                typename graph_t::edge_handler_t,
                                                weight_t,
                                                index_map_t>&tree,
                                  weight_map_t weight_map,
                                  upt_t updater,
                                  hfun_t hfunc,
                                  efilter_t efilter=always_bool_t<true>{})
{
    using node_t=graph_t::node_handler_t;

    property_map_t<node_t,weight_t,index_map_t> part_estimate(weight_t(0),tree.index_map());
    property_map_t<node_t,weight_t,index_map_t> total_estimate(weight_t(0),tree.index_map());
    auto comp=[&](node_t n1,node_t n2)
    {
        return updater.priority(total_estimate(n1),total_estimate(n2));
    };
    heap_priority_queue_t<node_t,decltype(comp),index_map_t> queue(comp,tree.index_map());
    return __astar_priority_search(graph,
                                   sourse,
                                   dest,
                                   tree,
                                   part_estimate,
                                   total_estimate,
                                   weight_map,
                                   updater,
                                   hfunc,
                                   queue,
                                   efilter);
}

template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class hfun_t,class efilter_t=always_bool_t<true>>
bool dense_astar_shortest_path(graph_t graph,
                               typename graph_t::node_handler_t sourse,
                               typename graph_t::node_handler_t dest,
                               tree_search_t<typename graph_t::node_handler_t,
                                             typename graph_t::edge_handler_t,
                                             weight_t,
                                             index_map_t>&tree,
                               weight_map_t weight_map,
                               hfun_t hfunc,
                               efilter_t efilter=always_bool_t<true>{})
{
    return dense_astar_priority_search(graph,
                                       sourse,
                                       dest,
                                       tree,
                                       weight_map,
                                       minimize_distance_t<weight_t>{},
                                       hfunc,
                                       efilter);
}


template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class hfun_t,class efilter_t=always_bool_t<true>>
bool sparse_astar_shortest_path(graph_t graph,
                                typename graph_t::node_handler_t sourse,
                                typename graph_t::node_handler_t dest,
                                tree_search_t<typename graph_t::node_handler_t,
                                              typename graph_t::edge_handler_t,
                                              weight_t,
                                              index_map_t>&tree,
                                weight_map_t weight_map,
                                hfun_t hfunc,
                                efilter_t efilter=always_bool_t<true>{})
{
    return sparse_astar_priority_search(graph,
                                        sourse,
                                        dest,
                                        tree,
                                        weight_map,
                                        minimize_distance_t<weight_t>{},
                                        hfunc,
                                        efilter);
}


}// liga

#endif






















