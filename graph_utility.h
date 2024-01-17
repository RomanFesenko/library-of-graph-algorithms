
/** @file graph_utility.h
 *  Content:
 *  is_equal
 *  find_edge
 *  traverse_graph
 *  undirect_graph_node_degrees
 *  direct_graph_node_degrees
*/

#ifndef _graph_utility_
#define _graph_utility_

#include <vector>

#include "graph_traits.h"
#include "search_settings.h"
#include "node_property.h"
#include "breadth_search.h"

namespace liga{

/**
 *  @brief  Is the same nodes.
 *  @param  graph        Graph,that satisfy graph_cpt concept.
 *  @param  n1           First node.
 *  @param  n2           Second node.
 *  @return              True if nodes is same.
*/

template<class graph_t>
constexpr bool is_equal(const graph_t&graph,
                        typename graph_traits<graph_t>::node_handler_t n1,
                        typename graph_traits<graph_t>::node_handler_t n2)
{
    using traits=graph_traits<graph_t>;
    return traits::index(graph,n1)==traits::index(graph,n2);
}

/**
 *  @brief  Find edge between two nodes, if it exist.
 *  @param  graph        Graph,that satisfy graph_cpt concept.
 *  @param  out          Node-source.
 *  @param  in           Node-target.
 *  @return              Edge between node, if it exist or empty optional otherwise.
*/

template<class graph_t>
std::optional<typename graph_traits<graph_t>::edge_handler_t>
find_edge(const graph_t&graph,
          typename graph_traits<graph_t>::node_handler_t out,
          typename graph_traits<graph_t>::node_handler_t in)
{
    using gts=graph_traits<graph_t>;
    using node_t=gts::node_handler_t;
    using edge_t=gts::edge_handler_t;
    for(edge_t e=gts::edge_begin(graph,out);!gts::edge_end(graph,out,e);gts::edge_inc(graph,out,e))
    {
        node_t target=gts::target(graph,out,e);
        if(is_equal(graph,target,in)) return e;
    }
    return {};
}

/**
 *  @brief  Travers subgraph, reachable from source node,counting number of nodes and edges.
 *  @param  is_directed  Is graph directed.
 *  @param  g            Graph,that satisfy graph_cpt concept.
 *  @param  source       Node-source.
 *  @return              Pair {number nodes in subgraph,number edges in subgraph}.
*/

template<class graph_t>
std::pair<std::size_t,std::size_t> traverse_graph(bool is_directed,
                                                  const graph_t&g,
                                                  typename graph_traits<graph_t>::node_handler_t source)
{
    using gts=graph_traits<graph_t>;
    using size_t=std::size_t;
    using node_t=gts::node_handler_t;
    using edge_t=gts::edge_handler_t;
    std::pair<size_t,size_t> res;
    auto search=full_search_t<graph_t>{}.set_node_preprocess([&](node_t out_node)
    {
        ++res.first;
        return true;
    }).
    set_edge_process([&](node_t out_node,edge_t edge)
    {
        ++res.second;
        return true;
    });
    breadth_search(is_directed,g,source,search);
    return res;
}

/**
 *  @brief  Counting number of adjacency edges for all undirected graph nodes.
 *  @param  g            Graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of the graph.
 *  @param  degrees      Storage for result.
 *  @return              Nothing.
*/

template<class graph_t,class cont_t>
void undirect_graph_node_degrees(const graph_t&g,const cont_t&nodes,node_property_t<graph_t,std::size_t>&degrees)
{
    using gts=graph_traits<graph_t>;
    using node_t=gts::node_handler_t;
    using edge_t=gts::edge_handler_t;
    degrees.init(g,0);
    for(node_t out:nodes)
    {
        for(edge_t e=gts::edge_begin(g,out);!gts::edge_end(g,out,e);gts::edge_inc(g,out,e))
        {
            node_t in_node=gts::target(g,out,e);
            ++degrees.ref(out);
            ++degrees.ref(in_node);
        }
    }
    for(node_t out:nodes)
    {
        degrees.ref(out)=degrees.ref(out)/2;
    }
}

/**
 *  @brief  Counting number of input and output edges for all directed graph nodes.
 *  @param  g            Graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of the graph.
 *  @param  degrees      Storage for result {number output edges,number input edges}.
 *  @return              Nothing.
*/

template<class graph_t,class cont_t>
void direct_graph_node_degrees(const graph_t&g,const cont_t&nodes,node_property_t<graph_t,std::pair<std::size_t,std::size_t>>&degrees)
{
    using gts=graph_traits<graph_t>;
    using node_t=gts::node_handler_t;
    using edge_t=gts::edge_handler_t;
    degrees.init(g,{0,0});
    for(node_t out:nodes)
    {
        for(edge_t e=gts::edge_begin(g,out);!gts::edge_end(g,out,e);gts::edge_inc(g,out,e))
        {
            node_t in_node=gts::target(g,out,e);
            ++degrees.ref(out).first;
            ++degrees.ref(in_node).second;
        }
    }
}

}// liga

#endif






















