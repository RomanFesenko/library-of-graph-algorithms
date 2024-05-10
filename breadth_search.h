
/** @file breadth_search.h
 *  Content:
 *  breadth_search
 *  is_bipartite
*/

#ifndef _breadth_search_
#define _breadth_search_

#include <vector>

#include "tree_search.h"
#include "property_map.h"
#include "search_settings.h"

namespace liga{

/**
 *  @brief  Breadth first search in graph.
 *  @param  is_directed Is directed input graph.
 *  @param  g           Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  search      Search adapter,that satisfy breadth_search_adapter_cpt concept.
 *  @param  tree        Tree search,that build algorithm.
 *  @return             Search termination cause.
*/

template<class graph_t,class search_t,class index_map_t>
search_termination_t breadth_search(bool is_directed,
                                    graph_t graph,
                                    typename graph_t::node_handler_t source,
                                    search_t&&search,
                                    tree_search_t<typename graph_t::node_handler_t,
                                                  typename graph_t::edge_handler_t,
                                                  std::size_t,
                                                  index_map_t>&tree)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    tree.drop();
    std::vector<node_t> queue,aux_queue;
    tree.m_resize_for_node(source);
    tree.m_state(source)=node_state_t::discovered_id;
    tree.m_dist(source)=0;
    tree.m_prev_node(source)=source;
    queue.push_back(source);
    while(!queue.empty())
    {
        std::swap(aux_queue,queue);
        queue.clear();
        while(!aux_queue.empty())
        {
            node_t from=aux_queue.back();
            aux_queue.pop_back();
            tree.m_state(from)=node_state_t::closed_id;
            if(!search.node_preprocess(from)) return search_termination_t::node_preprocess_id;
            for(edge_t e=graph.edge_begin(from);!graph.edge_end(from,e);graph.edge_inc(from,e))
            {
                if(!search.edge_filter(from,e)) continue;
                node_t target=graph.target(from,e);
                if(is_directed||tree.state(target)!=node_state_t::closed_id)
                {
                    if(!search.edge_process(from,e)) return search_termination_t::edge_process_id;
                }
                if(tree.in_tree(target)) continue;
                tree.m_resize_for_node(target);
                tree.m_dist(target)=tree.m_dist(from)+1;
                tree.m_prev_node(target)=from;
                tree.m_prev_edge(target)=e;
                tree.m_state(target)=node_state_t::discovered_id;
                queue.push_back(target);
            }
        }
    }
    return search_termination_t::complete_id;
}

/**
 *  @brief  Breadth first search in graph.
 *  @param  is_directed Is directed input graph.
 *  @param  g           Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  search      Search adapter,that satisfy breadth_search_adapter_cpt concept.
 *  @param  state       Node states after algorithm termination.
 *  @return             Search termination cause.
*/

template<class graph_t,class search_t,class index_map_t>
search_termination_t breadth_search(bool is_directed,
                                    graph_t graph,
                                    typename graph_t::node_handler_t source,
                                    search_t&&search,
                                    property_map_t<typename graph_t::node_handler_t,
                                                   node_state_t,
                                                   index_map_t>&state)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    state.clear();
    state.default_value()=node_state_t::undiscovered_id;
    std::vector<node_t> queue,aux_queue;
    state.ref(source)=node_state_t::discovered_id;
    queue.push_back(source);
    while(!queue.empty())
    {
        std::swap(aux_queue,queue);
        queue.clear();
        while(!aux_queue.empty())
        {
            node_t from=aux_queue.back();
            aux_queue.pop_back();
            state.ref(from)=node_state_t::closed_id;
            if(!search.node_preprocess(from)) return search_termination_t::node_preprocess_id;
            for(edge_t e=graph.edge_begin(from);!graph.edge_end(from,e);graph.edge_inc(from,e))
            {
                if(!search.edge_filter(from,e)) continue;
                node_t target=graph.target(from,e);
                if(is_directed||state.cref(target)!=node_state_t::closed_id)
                {
                    if(!search.edge_process(from,e)) return search_termination_t::edge_process_id;
                }
                if(state.cref(target)!=node_state_t::undiscovered_id) continue;
                state.ref(target)=node_state_t::discovered_id;
                queue.push_back(target);
            }
        }
    }
    return search_termination_t::complete_id;
}

/**
 *  @brief  Breadth first search in graph.
 *  @param  is_directed Is directed input graph.
 *  @param  graph       Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  search      Search adapter,that satisfy breadth_search_adapter_cpt concept.
 *  @param  index_map   Index map,which mapped node to unique integer.
 *  @return             Search termination cause.
*/

template<class graph_t,class search_t,class index_map_t>
search_termination_t breadth_search(bool is_directed,
                                    graph_t graph,
                                    typename graph_t::node_handler_t source,
                                    search_t&&search,
                                    index_map_t index_map)
{

    property_map_t<typename graph_t::node_handler_t,
                   node_state_t,
                   index_map_t> state(node_state_t::undiscovered_id,index_map);
    return breadth_search(is_directed,graph,source,std::forward<search_t>(search),state);
}

/**
 *  @brief  Is bipartite undirected graph.
 *  @param  g           Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  index_map   Index map,which mapped graph nodes to unique integer.
 *  @return             True if graph bipartite.
*/

template<class graph_t,class index_map_t>
bool is_bipartite(graph_t graph,
                  typename graph_t::node_handler_t source,
                  index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    property_map_t<node_t,char,index_map_t> color(2,index_map);
    color.ref(source)=0;
    auto search=full_search_t().set_edge_process([&](node_t out_node,edge_t edge)
    {
        assert(color.cref(out_node)!=2);
        node_t t=graph.target(out_node,edge);
        if(color.cref(t)==2)
        {
            color.ref(t)=color.cref(out_node)==0? 1:0;
        }
        return color.cref(t)!=color.cref(out_node);
    });
    return breadth_search(false,graph,source,search,index_map)!=search_termination_t::edge_process_id;
}

}// liga

#endif






















