
/** @file euler_tour.h
 *  Content:
 *  has_direct_euler_tour
 *  has_undirect_euler_tour
 *  has_direct_euler_cycle
 *  has_undirect_euler_cycle
 *  euler_tour
 *  euler_cycle
*/


#ifndef _liga_euler_tour_
#define _liga_euler_tour_

#include <optional>
#include <utility>
#include <vector>

#include "property_map.h"
#include "search_settings.h"
#include "graph_utility.h"
#include "breadth_search.h"

namespace liga{

template<class graph_t>
struct tour_edge_t
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    node_t source;
    node_t target;
    edge_t edge;
};

template<class graph_t,class index_map_t>
std::vector<tour_edge_t<graph_t>>
__make_direct_tour(graph_t graph,
                   typename graph_t::node_handler_t source,
                   index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    property_map_t<node_t,std::optional<edge_t>,index_map_t> current_edge({},index_map);

    std::vector<tour_edge_t<graph_t>> tour;
    std::vector<tour_edge_t<graph_t>> stack;
    stack.push_back({{},source,{}});
    while(!stack.empty())
    {
        node_t node=stack.back().target;
        if(!current_edge.ref(node)) current_edge.ref(node)=graph.edge_begin(node);
        edge_t&edge=*current_edge.ref(node);
        if(graph.edge_end(node,edge))
        {
            tour.push_back(stack.back());
            stack.pop_back();
        }
        else
        {
            stack.push_back({node,graph.target(node,edge),edge});
            graph.edge_inc(node,edge);
        }
    }
    tour.pop_back();
    std::reverse(tour.begin(),tour.end());
    return tour;
}

template<class graph_t,class node_index_map_t,class edge_index_map_t>
std::vector<tour_edge_t<graph_t>>
__make_undirect_tour(graph_t graph,
                     typename graph_t::node_handler_t source,
                     node_index_map_t node_index_map,
                     edge_index_map_t edge_index_map)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    property_map_t<node_t,std::optional<edge_t>,node_index_map_t> current_edge({},node_index_map);
    property_map_t<edge_t,char,edge_index_map_t>                  visited_edge(0,edge_index_map);

    std::vector<tour_edge_t<graph_t>> tour;
    std::vector<tour_edge_t<graph_t>> stack;
    stack.push_back({{},source,{}});
    while(!stack.empty())
    {
        node_t node=stack.back().target;
        if(!current_edge.ref(node)) current_edge.ref(node)=graph.edge_begin(node);
        edge_t&edge=*current_edge.ref(node);
        while(!graph.edge_end(node,edge)&&visited_edge(edge))
        {
            graph.edge_inc(node,edge);
        }
        if(graph.edge_end(node,edge))
        {
            tour.push_back(stack.back());
            stack.pop_back();
        }
        else
        {
            stack.push_back({node,graph.target(node,edge),edge});
            visited_edge.ref(edge)=1;
            graph.edge_inc(node,edge);
        }
    }
    tour.pop_back();
    std::reverse(tour.begin(),tour.end());
    return tour;
}

/**
 *  @brief  Check directed graph for existing Euler tour between two nodes.
 *  @param  graph       Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  index_map   Index map,which mapped graph nodes to unique integer.
 *  @param  ss          Pair of {first tour node -last tour node}, defined algorithm
 *  @return             True if Euler tour exist.
 *
*/

template<class graph_t,class cont_t,class index_map_t>
bool has_directed_euler_tour(graph_t graph,const cont_t&nodes,index_map_t index_map,
                             std::pair<typename graph_t::node_handler_t,
                                       typename graph_t::node_handler_t>&ss)
{
    using size_t=std::size_t;
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,std::pair<size_t,size_t>,index_map_t> degrees({0,0},index_map);
    direct_graph_node_degrees(graph,nodes,degrees);
    std::optional<node_t> source;
    std::optional<node_t> sink;
    size_t num_nodes=0;
    for(node_t node:nodes)
    {
        ++num_nodes;
        auto [out,in]=degrees(node);
        if(out==in)
        {
            if(out==0) return false;
            continue;
        }
        else if(out==in+1)
        {
            if(source) return false;
            source=node;
            continue;
        }
        else if(in==out+1)
        {
            if(sink) return false;
            sink=node;
            continue;
        }
        else return false;
    }
    if(!source||!sink||traverse_graph(true,graph,*source,index_map).first!=num_nodes) return false;
    ss={*source,*sink};
    return true;
}

template<class graph_t,class cont_t,class index_map_t>
bool has_directed_euler_tour(graph_t graph,const cont_t&nodes,index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    std::pair<node_t,node_t> p;
    return has_directed_euler_tour(graph,nodes,index_map,p);
}

/**
 *  @brief  Check undirected graph for existing Euler tour between two nodes.
 *  @param  graph       Undirected graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  index_map   Index map,which mapped graph nodes to unique integer.
 *  @param  ss          Pair of {first tour node - last tour node}, defined algorithm.
 *  @return             True if Euler tour exist.
 *
*/

template<class graph_t,class cont_t,class index_map_t>
bool has_undirected_euler_tour(graph_t graph,const cont_t&nodes,index_map_t index_map,
                               std::pair<typename graph_t::node_handler_t,
                                         typename graph_t::node_handler_t>&ss)
{
    using size_t=std::size_t;
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,size_t,index_map_t> degrees(0,index_map);
    undirect_graph_node_degrees(graph,nodes,degrees);
    size_t num_nodes=0;
    std::vector<node_t> odd;
    for(node_t node:nodes)
    {
        ++num_nodes;
        if(size_t deg=degrees(node);deg&1)
        {
            if(odd.size()==2) return false;
            odd.push_back(node);
        }
    }
    if(odd.size()!=2||traverse_graph(false,graph,odd[0],index_map).first!=num_nodes) return false;
    ss={odd[0],odd[1]};
    return true;
}

template<class graph_t,class cont_t,class index_map_t>
bool has_undirected_euler_tour(graph_t graph,const cont_t&nodes,index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    std::pair<node_t,node_t> p;
    return has_undirected_euler_tour(graph,nodes,index_map,p);
}

/**
 *  @brief  Check directed graph for existing Euler cycle.
 *  @param  graph       Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  index_map   Index map,which mapped graph nodes to unique integer.
 *  @return             True if Euler cycle exist.
 *
*/

template<class graph_t,class cont_t,class index_map_t>
bool has_directed_euler_cycle(graph_t graph,const cont_t&nodes,index_map_t index_map)
{
    using size_t=std::size_t;
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,std::pair<size_t,size_t>,index_map_t> degrees({0,0},index_map);
    direct_graph_node_degrees(graph,nodes,degrees);
    size_t num_nodes=0;
    for(node_t node:nodes)
    {
        ++num_nodes;
        if(auto [out,in]=degrees.cref(node);out!=in) return false;
    }
    return traverse_graph(true,graph,*std::begin(nodes),index_map).first==num_nodes;
}

/**
 *  @brief  Check undirected graph for existing Euler cycle.
 *  @param  graph       Undirected graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  index_map   Index map,which mapped graph nodes to unique integer.
 *  @return             True if Euler cycle exist.
 *
*/

template<class graph_t,class cont_t,class index_map_t>
bool has_undirected_euler_cycle(graph_t graph,const cont_t&nodes,index_map_t index_map)
{
    using size_t=std::size_t;
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,size_t,index_map_t> degrees(0,index_map);
    undirect_graph_node_degrees(graph,nodes,degrees);
    size_t num_nodes=0;
    for(node_t node:nodes)
    {
        ++num_nodes;
        if(degrees.cref(node)&1) return false;
    }
    return traverse_graph(false,graph,*std::begin(nodes),index_map).first==num_nodes;
}

/**
 *  @brief  Find Euler tour in directed graph if it exist.
 *  @param  graph       Graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  index_map   Index map,which mapped graph nodes to unique integer.
 *  @return             Euler tour if it exist or empty tour otherwise.
 *
*/

template<class graph_t,class cont_t,class node_index_map_t>
std::vector<tour_edge_t<graph_t>> directed_euler_tour(graph_t graph,
                                                      const cont_t&nodes,
                                                      node_index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    std::pair<node_t,node_t> ss;
    std::vector<tour_edge_t<graph_t>> tour;
    if(!has_directed_euler_tour(graph,nodes,index_map,ss)) return {};
    tour=__make_direct_tour(graph,ss.first,index_map);
    assert(!tour.empty());
    return tour;
}

/**
 *  @brief  Find Euler tour in undirected graph if it exist.
 *  @param  graph           Graph,that satisfy graph_cpt concept.
 *  @param  nodes           All nodes of the graph.
 *  @param  node_index_map  Index map,which mapped graph nodes to unique integer.
 *  @param  edge_index_map  Index map,which mapped graph edges to unique integer.
 *  @return                 Euler tour if it exist or empty tour otherwise.
 *
*/

template<class graph_t,class cont_t,class node_index_map_t,class edge_index_map_t>
std::vector<tour_edge_t<graph_t>> undirected_euler_tour(graph_t graph,
                                                        const cont_t&nodes,
                                                        node_index_map_t node_index_map,
                                                        edge_index_map_t edge_index_map)
{
    using node_t=graph_t::node_handler_t;
    std::pair<node_t,node_t> ss;
    std::vector<tour_edge_t<graph_t>> tour;
    if(!has_undirected_euler_tour(graph,nodes,node_index_map,ss)) return {};
    tour=__make_undirect_tour(graph,ss.first,node_index_map,edge_index_map);
    assert(!tour.empty());
    return tour;
}

/**
 *  @brief  Find Euler cycle in directed graph if it exist.
 *  @param  graph       Graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of the graph.
 *  @param  index_map   Index map,which mapped graph nodes to unique integer.
 *  @return             Euler cycle if it exist or empty cycle otherwise.
 *
*/

template<class graph_t,class cont_t,class node_index_map_t>
std::vector<tour_edge_t<graph_t>> directed_euler_cycle(graph_t graph,
                                                       const cont_t&nodes,
                                                       node_index_map_t index_map)
{
    std::vector<tour_edge_t<graph_t>> tour;
    if(!has_directed_euler_cycle(graph,nodes,index_map)) return {};
    tour=__make_direct_tour(graph,*std::begin(nodes),index_map);
    assert(!tour.empty());
    return tour;
}

/**
 *  @brief  Find Euler cycle in undirected graph if it exist.
 *  @param  graph           Graph,that satisfy graph_cpt concept.
 *  @param  nodes           All nodes of the graph.
 *  @param  node_index_map  Index map,which mapped graph nodes to unique integer.
 *  @param  edge_index_map  Index map,which mapped graph edges to unique integer.
 *  @return                 Euler cycle if it exist or empty cycle otherwise.
 *
*/

template<class graph_t,class cont_t,class node_index_map_t,class edge_index_map_t>
std::vector<tour_edge_t<graph_t>> undirected_euler_cycle(graph_t graph,
                                                         const cont_t&nodes,
                                                         node_index_map_t node_index_map,
                                                         edge_index_map_t edge_index_map)
{
    std::vector<tour_edge_t<graph_t>> tour;
    if(!has_undirected_euler_cycle(graph,nodes,node_index_map)) return {};
    tour=__make_undirect_tour(graph,*std::begin(nodes),node_index_map,edge_index_map);
    assert(!tour.empty());
    return tour;
}

}// liga

#endif
