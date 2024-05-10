/** @file min_cost_flow.h
 *  Minimal cost flow algorithms:
 *  sparse_min_cost_flow
 *  dense_min_cost_flow
 *  cycle_canceling_min_cost_flow
*/

#ifndef _liga_min_cost_flow_
#define _liga_min_cost_flow_

#include <vector>
#include <optional>
#include <utility>
#include <type_traits>

#include "max_flow.h"

namespace liga{

template<class f_t,class c_t>
struct costed_direct_pipe_t
{
    using size_t=std::size_t;
    using flow_t=f_t;
    using cost_t=c_t;

    size_t from;
    size_t to;
    flow_t capacity;
    cost_t cost;
};

/**
 *  @brief Find minimal cost flow using flow augmentation along min-cost path in residual network.
 *  @param net_data     Network data:source,sink and network edges with its capacities/costs.
 *  @param flows        Network edges with its flows.
 *  @param req_flow     Required total flow from source to sink.
 *  @return             tuple{is find required flow,total flow,total cost}
 *
 *  Version, optimized for sparse networks.
*/

template<class flow_t,class cost_t>
std::tuple<bool,flow_t,cost_t>
sparse_min_cost_flow(const network_traits_t<costed_direct_pipe_t<flow_t,cost_t>>&data,
                     std::vector<std::tuple<std::size_t,std::size_t,flow_t>>&flows,
                     flow_t req_flow)
{
    using namespace __detail_flows;

    using pipe_t=costed_direct_pipe_t<flow_t,cost_t>;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;
    using tuple_t=std::tuple<bool,flow_t,cost_t>;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t net(pipes,list);
    net.build(data.pipes);

    std::vector<cost_t> pot(net.num_nodes(),cost_t{0});
    // Edmond-Karp trick, which allow save positive costs for residual network edges.
    auto weight_map=[&](node_t n,edge_t e)->cost_t
    {
        auto&pipe=net.pipe(e);
        if(pipe.from==n)
        {
            assert( pipe.cost+pot[n]>=pot[pipe.to]);
            return pipe.cost+pot[n]-pot[pipe.to];
        }
        else
        {
            assert(pot[pipe.to]>=pot[pipe.from]+pipe.cost);
            return pot[pipe.to]-pot[pipe.from]-pipe.cost;
        }
    };
    auto search=full_search_t{}.set_edge_filter([=](node_t n,edge_t e)
    {
        return net.residual_capacity(n,e)>flow_t{0};
    });

    tree_search_t<node_t,edge_t,flow_t,default_index_map_t> tree;
    flow_t total_flow{0};
    cost_t total_cost{0};
    while(total_flow<req_flow)
    {
        sparse_priority_search(net,data.source,search,tree,weight_map,minimize_distance_t<flow_t>{});
        if(!tree.in_tree(data.sink)) break;
        cost_t unit_flow_cost=tree.dist(data.sink)+pot[data.sink];
        std::optional<flow_t> added;
        tree.traverse_path(data.sink,[&](node_t node,edge_t edge)
        {
            flow_t residual=net.residual_capacity(node,edge);
            added=added? std::min(residual,*added):residual;
        });
        assert(added>flow_t{0});
        if(*added+total_flow>req_flow)
        {
            added=req_flow-total_flow;
        }
        tree.traverse_path(data.sink,[=](node_t node,edge_t edge)mutable
        {
            net.add_flow(node,edge,*added);
        });
        total_flow+=*added;
        total_cost+=*added*unit_flow_cost;
        for(std::size_t i=0;i<net.num_nodes();++i)
        {
            if(auto opt=tree.opt_dist(i);opt)
            {
                pot[i]+=*opt;
            }
        }
    }
    net.get_flows(flows);
    return total_flow==req_flow? tuple_t{true,total_flow,total_cost}:
                                 tuple_t{false,total_flow,total_cost};
}

template<class flow_t,class cost_t>
std::pair<flow_t,cost_t>
sparse_min_cost_flow(network_traits_t<costed_direct_pipe_t<flow_t,cost_t>>&data,
                     std::vector<std::tuple<std::size_t,std::size_t,flow_t>>&flows)
{
    flow_t max=flow_t{0};
    for(auto&pipe:data.pipes)
    {
        if(pipe.from==data.source) max+=pipe.capacity;
    }
    auto res=sparse_min_cost_flow(data,flows,max);
    return {std::get<1>(res),std::get<2>(res)};
}

/**
 *  @brief Find minimal cost flow using flow augmentation along min-cost path in residual network.
 *  @param net_data     Network data:source,sink and network edges with its capacities/costs.
 *  @param flows        Network edges with its flows.
 *  @param req_flow     Required total flow from source to sink.
 *  @return             tuple{is find required flow,total flow,total cost}
 *
 *  Version, optimized for dense networks.
*/

template<class flow_t,class cost_t>
std::tuple<bool,flow_t,cost_t>
dense_min_cost_flow(const network_traits_t<costed_direct_pipe_t<flow_t,cost_t>>&data,
                    std::vector<std::tuple<std::size_t,std::size_t,flow_t>>&flows,
                    flow_t req_flow)
{
    using namespace __detail_flows;

    using pipe_t=costed_direct_pipe_t<flow_t,cost_t>;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;
    using tuple_t=std::tuple<bool,flow_t,cost_t>;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t net(pipes,list);
    net.build(data.pipes);

    std::vector<cost_t> pot(net.num_nodes(),cost_t{0});
    // Edmonds-Karp trick, which allow save positive costs for residual network edges.
    auto weight_map=[&](node_t n,edge_t e)->cost_t
    {
        auto&pipe=net.pipe(e);
        if(pipe.from==n)
        {
            assert(pipe.cost+pot[n]>=pot[pipe.to]);
            return pipe.cost+pot[n]-pot[pipe.to];
        }
        else
        {
            assert(pot[pipe.to]>=pot[pipe.from]+pipe.cost);
            return pot[pipe.to]-pot[pipe.from]-pipe.cost;
        }
    };
    auto search=full_search_t{}.set_edge_filter([=](node_t n,edge_t e)
    {
        return net.residual_capacity(n,e)>flow_t{0};
    });

    tree_search_t<node_t,edge_t,flow_t,default_index_map_t> tree;
    flow_t total_flow{0};
    cost_t total_cost{0};
    while(total_flow<req_flow)
    {
        dense_priority_search(net,data.source,search,tree,weight_map,minimize_distance_t<flow_t>{});
        if(!tree.in_tree(data.sink)) break;
        cost_t unit_flow_cost=tree.dist(data.sink)+pot[data.sink];
        std::optional<flow_t> added;
        tree.traverse_path(data.sink,[&](node_t node,edge_t edge)
        {
            flow_t residual=net.residual_capacity(node,edge);
            added=added? std::min(residual,*added):residual;
        });
        assert(added>flow_t{0});
        if(*added+total_flow>req_flow)
        {
            added=req_flow-total_flow;
        }
        tree.traverse_path(data.sink,[&](node_t node,edge_t edge)
        {
            net.add_flow(node,edge,*added);
        });
        total_flow+=*added;
        total_cost+=*added*unit_flow_cost;
        for(std::size_t i=0;i<net.num_nodes();++i)
        {
            if(auto opt=tree.opt_dist(i);opt)
            {
                pot[i]+=*opt;
            }
        }
    }
    net.get_flows(flows);
    return total_flow==req_flow? tuple_t{true,total_flow,total_cost}:
                                 tuple_t{false,total_flow,total_cost};
}

template<class flow_t,class cost_t>
std::pair<flow_t,cost_t>
dense_min_cost_flow(network_traits_t<costed_direct_pipe_t<flow_t,cost_t>>&data,
                    std::vector<std::tuple<std::size_t,std::size_t,flow_t>>&flows)
{
    flow_t max=flow_t{0};
    for(auto&pipe:data.pipes)
    {
        if(pipe.from==data.source) max+=pipe.capacity;
    }
    auto res=dense_min_cost_flow(data,flows,max);
    return {std::get<1>(res),std::get<2>(res)};
}

/**
 *  @brief Find minimal cost flow using cycle-canceling algorithm in residual network.
 *  @param net_data     Network data:source,sink and network edges with its capacities/costs.
 *  @param flows        Network edges with its flows.
 *  @param req_flow     Required total flow from source to sink.
 *  @return             tuple{is find required flow,total flow,total cost}
 *
 *  Find any negative-cost cycle in residual network, and saturate this cycle maximal
 *  possible flow.
*/

template<class flow_t,class cost_t>
std::tuple<bool,flow_t,cost_t>
cycle_canceling_min_cost_flow(const network_traits_t<costed_direct_pipe_t<flow_t,cost_t>>&data,
                              std::vector<std::tuple<std::size_t,std::size_t,flow_t>>&flows,
                              flow_t req_flow)
{
    static_assert(std::is_signed_v<cost_t>);

    using namespace __detail_flows;

    using network_t=residual_network_t<costed_direct_pipe_t<flow_t,cost_t>>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;
    using pipe_t=costed_direct_pipe_t<flow_t,cost_t>;
    using tuple_t=std::tuple<bool,flow_t,cost_t>;

    cost_t fict_edge_cost=cost_t{1};
    for(const pipe_t&pipe:data.pipes)
    {
        fict_edge_cost+=pipe.cost;
    }

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t net(pipes,list);
    net.build(data.pipes);

    auto& fict_pipe=net.add_pipe({data.source,data.sink,req_flow,fict_edge_cost});
    fict_pipe.add_flow(data.source,req_flow);

    cost_t total_cost=fict_edge_cost*req_flow;

    auto efilter=[&](node_t n,edge_t e)
    {
        return net.residual_capacity(n,e)>flow_t{0};
    };
    auto residual_cost=[=](node_t n,edge_t e)
    {
        auto&pipe=net.pipe(e);
        return pipe.from==n? pipe.cost:-pipe.cost;
    };
    while(true)
    {
        auto cycle=bellman_ford_negative_cycle<cost_t>(net,data.sink,default_index_map_t{},residual_cost,efilter);
        if(cycle.empty()) break;
        cost_t cost_for_unit=cost_t{0};
        std::optional<flow_t> added;
        for(auto [n,e]:cycle)
        {
            cost_for_unit+=residual_cost(n,e);
            added=added? std::min(*added,net.residual_capacity(n,e)):net.residual_capacity(n,e);
        }
        assert(cost_for_unit<cost_t{0});
        assert(added);
        for(auto [n,e]:cycle)
        {
            net.add_flow(n,e,*added);
        }
        total_cost+=*added*cost_for_unit;
    }
    flow_t total_flow=req_flow-fict_pipe.flow;
    total_cost-=fict_pipe.flow*fict_edge_cost;
    net.get_flows(flows);

    return total_flow==req_flow? tuple_t{true,total_flow,total_cost}:
                                 tuple_t{false,total_flow,total_cost};
}

template<class flow_t,class cost_t>
std::pair<flow_t,cost_t>
cycle_canceling_min_cost_flow(network_traits_t<costed_direct_pipe_t<flow_t,cost_t>>&data,
                              std::vector<std::tuple<std::size_t,std::size_t,flow_t>>&flows)
{
    flow_t max=flow_t{0};
    for(auto&pipe:data.pipes)
    {
        if(pipe.from==data.source) max+=pipe.capacity;
    }
    auto res=cycle_canceling_min_cost_flow(data,flows,max);
    return {std::get<1>(res),std::get<2>(res)};
}

}// liga

#endif






















