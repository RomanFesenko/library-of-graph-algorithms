
/** @file graph_utility.h
 *  Content:
 *  is_equal
 *  find_edge
 *  traverse_graph
 *  undirect_graph_node_degrees
 *  direct_graph_node_degrees
*/

#ifndef _liga_graph_utility_
#define _liga_graph_utility_

#include <vector>
#include <type_traits>
#include <functional>

#include "search_settings.h"
#include "property_map.h"
#include "breadth_search.h"


namespace liga{

/**
 *  @brief  Find edge between two nodes, if it exist.
 *  @param  graph        Graph,that satisfy graph_cpt concept.
 *  @param  out          Node-source.
 *  @param  in           Node-target.
 *  @param  comp         Binary predicate of node equalency.
 *  @return              Edge between node, if it exist or empty optional otherwise.
*/

template<class graph_t,class comp_t=std::equal_to<>>
std::optional<typename graph_t::edge_handler_t>
find_edge(graph_t graph,
          typename graph_t::node_handler_t out,
          typename graph_t::node_handler_t in,
          comp_t comp=std::equal_to<>{})
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    for(edge_t e=graph.edge_begin(out);!graph.edge_end(out,e);graph.edge_inc(out,e))
    {
        node_t target=graph.target(out,e);
        if(comp(target,in)) return e;
    }
    return {};
}

template<class graph_t,class out_func_t,class comp_t=std::equal_to<>>
std::size_t traverse_edges(graph_t graph,
                           typename graph_t::node_handler_t out,
                           typename graph_t::node_handler_t in,
                           out_func_t out_func,
                           comp_t comp=std::equal_to<>{})
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    std::size_t num=0;
    for(edge_t e=graph.edge_begin(out);!graph.edge_end(out,e);graph.edge_inc(out,e))
    {
        node_t target=graph.target(out,e);
        if(comp(target,in))
        {
            out_func(e);
            ++num;
        }
    }
    return num;
}

/**
 *  @brief  Travers subgraph, reachable from source node,counting number of nodes and edges.
 *  @param  is_directed  Is graph directed.
 *  @param  graph        Graph,that satisfy graph_cpt concept.
 *  @param  source       Node-source.
 *  @param  index_map    Index map,which mapped graph nodes to unique integer.
 *  @return              Pair {number nodes in subgraph,number edges in subgraph}.
*/

template<class graph_t,class index_map_t>
std::pair<std::size_t,std::size_t> traverse_graph(bool is_directed,
                                                  graph_t graph,
                                                  typename graph_t::node_handler_t source,
                                                  index_map_t index_map)
{
    using size_t=std::size_t;
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    std::pair<size_t,size_t> res;
    auto search=full_search_t{}.set_node_preprocess([&](node_t out_node)
    {
        ++res.first;
        return true;
    }).
    set_edge_process([&](node_t out_node,edge_t edge)
    {
        ++res.second;
        return true;
    });
    breadth_search(is_directed,graph,source,search,index_map);
    return res;
}

/**
 *  @brief  Counting number of adjacency edges for all undirected graph nodes.
 *  @param  graph        Graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of the graph.
 *  @param  degrees      Storage for result.
 *  @return              Nothing.
*/

template<class graph_t,class cont_t,class index_map_t>
void undirect_graph_node_degrees(graph_t graph,
                                 const cont_t&nodes,
                                 property_map_t<typename graph_t::node_handler_t,
                                                std::size_t,
                                                index_map_t>&degrees)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    degrees.default_value()=0;
    for(node_t out:nodes)
    {
        for(edge_t e=graph.edge_begin(out);!graph.edge_end(out,e);graph.edge_inc(out,e))
        {
            node_t in_node=graph.target(out,e);
            ++degrees.ref(out);
            ++degrees.ref(in_node);
        }
    }
    for(node_t out:nodes)
    {
        degrees.ref(out)/=2;
    }
}

/**
 *  @brief  Counting number of input and output edges for all directed graph nodes.
 *  @param  graph        Graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of the graph.
 *  @param  degrees      Storage for result {number output edges,number input edges}.
 *  @return              Nothing.
*/

template<class graph_t,class cont_t,class index_map_t>
void direct_graph_node_degrees(graph_t graph,
                               const cont_t&nodes,
                               property_map_t<typename graph_t::node_handler_t,
                                              std::pair<std::size_t,std::size_t>,
                                              index_map_t>&degrees)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    degrees.default_value()={0,0};
    for(node_t out:nodes)
    {
        for(edge_t e=graph.edge_begin(out);!graph.edge_end(out,e);graph.edge_inc(out,e))
        {
            node_t in_node=graph.target(out,e);
            ++degrees.ref(out).first;
            ++degrees.ref(in_node).second;
        }
    }
}

}// liga

#endif






















