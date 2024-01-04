
#ifndef _euler_tour_
#define _euler_tour_

#include <optional>
#include <utility>
#include <vector>

#include "graph_traits.h"
#include "node_property.h"
#include "search_settings.h"
#include "graph_utility.h"


template<class graph_t>
std::vector<std::pair<typename graph_traits<graph_t>::node_handler_t,
                      typename graph_traits<graph_t>::edge_handler_t>>
__make_direct_tour(graph_t&graph,
                   typename graph_traits<graph_t>::node_handler_t source)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;

    std::vector<std::pair<node_t,edge_t>> tour;
    node_property_t<graph_t,std::optional<edge_t>> cur_edge(graph,{});
    std::vector<std::tuple<node_t,node_t,edge_t>> stack;
    cur_edge.ref(source)=traits::edge_begin(graph,source);
    stack.push_back({source,source,edge_t{}});

    while(!stack.empty())
    {
        auto [top,prev_node,prev_edge]=stack.back();
        edge_t edge=*cur_edge(top);
        if(traits::edge_end(graph,top,edge))
        {
            tour.push_back({prev_node,prev_edge});
            stack.pop_back();
            continue;
        }
        else
        {
            node_t target=traits::target(graph,top,edge);
            if(!cur_edge(target))
            {
                cur_edge.ref(target)=traits::edge_begin(graph,target);
            }
            stack.push_back({target,top,edge});
            traits::edge_inc(graph,top,edge);
            cur_edge.ref(top)=edge;
        }
    }
    tour.pop_back();
    std::reverse(tour.begin(),tour.end());
    return tour;
}

template<class graph_t>
std::vector<std::pair<typename graph_traits<graph_t>::node_handler_t,
                      typename graph_traits<graph_t>::edge_handler_t>>
__make_undirect_tour(graph_t&graph,
                     typename graph_traits<graph_t>::node_handler_t source)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;

    std::vector<std::pair<node_t,edge_t>> tour;
    node_property_t<graph_t,std::optional<edge_t>> cur_edge(graph,{});
    std::vector<std::tuple<node_t,node_t,edge_t>> stack;
    cur_edge.ref(source)=traits::edge_begin(graph,source);

    stack.push_back({source,source,edge_t{}});
    while(!stack.empty())
    {
        auto [top,prev_node,prev_edge]=stack.back();
        edge_t edge=*cur_edge(top);
        if(traits::edge_end(graph,top,edge))
        {
            tour.push_back({prev_node,prev_edge});
            stack.pop_back();
            continue;
        }
        else
        {
            node_t target=traits::target(graph,top,edge);
            if(traits::index(graph,target)!=traits::index(graph,prev_node))
            {
                if(!cur_edge(target))
                {
                    cur_edge.ref(target)=traits::edge_begin(graph,target);
                }
                stack.push_back({target,top,edge});
            }
            traits::edge_inc(graph,top,edge);
            cur_edge.ref(top)=edge;
        }
    }
    tour.pop_back();
    std::reverse(tour.begin(),tour.end());
    return tour;
}

template<class graph_t,class cont_t>
bool has_direct_euler_tour(graph_t&graph,const cont_t&nodes,
                           std::pair<typename graph_traits<graph_t>::node_handler_t,
                                     typename graph_traits<graph_t>::node_handler_t>&ss)
{
    using size_t=std::size_t;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    node_property_t<graph_t,std::pair<size_t,size_t>> degrees(graph,{0,0});
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
    if(!source||!sink||traverse_graph(true,graph,*source).first!=num_nodes) return false;
    ss={*source,*sink};
    return true;
}

template<class graph_t,class cont_t>
bool has_direct_euler_tour(graph_t&graph,const cont_t&nodes)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    std::pair<node_t,node_t> p;
    return has_direct_euler_tour(graph,nodes,p);
}


template<class graph_t,class cont_t>
bool has_undirect_euler_tour(graph_t&graph,const cont_t&nodes,
                             std::pair<typename graph_traits<graph_t>::node_handler_t,
                                       typename graph_traits<graph_t>::node_handler_t>&ss)
{
    using size_t=std::size_t;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    node_property_t<graph_t,size_t> degrees(graph,0);
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
    if(odd.size()!=2||traverse_graph(false,graph,odd[0]).first!=num_nodes) return false;
    ss={odd[0],odd[1]};
    return true;
}

template<class graph_t,class cont_t>
bool has_undirect_euler_tour(graph_t&graph,const cont_t&nodes)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    std::pair<node_t,node_t> p;
    return has_undirect_euler_tour(graph,nodes,p);
}

template<class graph_t,class cont_t>
bool has_direct_euler_cycle(graph_t&graph,const cont_t&nodes)
{
    using size_t=std::size_t;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    node_property_t<graph_t,std::pair<size_t,size_t>> degrees(graph,{0,0});
    direct_graph_node_degrees(graph,nodes,degrees);
    size_t num_nodes=0;
    for(node_t node:nodes)
    {
        ++num_nodes;
        if(auto [out,in]=degrees(node);out!=in) return false;
    }
    return traverse_graph(true,graph,*std::begin(nodes)).first==num_nodes;
}

template<class graph_t,class cont_t>
bool has_undirect_euler_cycle(graph_t&graph,const cont_t&nodes)
{
    using size_t=std::size_t;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    node_property_t<graph_t,size_t> degrees(graph,0);
    undirect_graph_node_degrees(graph,nodes,degrees);
    size_t num_nodes=0;
    for(node_t node:nodes)
    {
        ++num_nodes;
        if(degrees(node)&1) return false;
    }
    return traverse_graph(false,graph,*std::begin(nodes)).first==num_nodes;
}

template<class graph_t,class cont_t>
std::vector<std::pair<typename graph_traits<graph_t>::node_handler_t,
                      typename graph_traits<graph_t>::edge_handler_t>>
euler_tour(bool is_direct,graph_t&graph,const cont_t&nodes)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    std::pair<node_t,node_t> ss;
    std::vector<std::pair<node_t,edge_t>> tour;
    if(is_direct)
    {
        if(!has_direct_euler_tour(graph,nodes,ss)) return {};
        tour=__make_direct_tour(graph,ss.first);
    }
    else
    {
        if(!has_undirect_euler_tour(graph,nodes,ss)) return {};
        tour=__make_undirect_tour(graph,ss.first);
    }
    assert(!tour.empty());
    auto [node,edge]=tour.back();
    assert(is_equal(graph,traits::target(graph,node,edge),ss.second));
    return tour;
}


template<class graph_t,class cont_t>
std::vector<std::pair<typename graph_traits<graph_t>::node_handler_t,
                      typename graph_traits<graph_t>::edge_handler_t>>
euler_cycle(bool is_direct,graph_t&graph,const cont_t&nodes)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    std::vector<std::pair<node_t,edge_t>> tour;
    if(is_direct)
    {
        if(!has_direct_euler_cycle(graph,nodes)) return {};
        tour=__make_direct_tour(graph,*std::begin(nodes));
    }
    else
    {
        if(!has_undirect_euler_cycle(graph,nodes)) return {};
        tour=__make_undirect_tour(graph,*std::begin(nodes));
    }
    assert(!tour.empty());
    auto [node,edge]=tour.back();
    assert(is_equal(graph,traits::target(graph,node,edge),*std::begin(nodes)));
    return tour;
}

#endif






















