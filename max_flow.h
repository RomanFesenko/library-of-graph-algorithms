
#ifndef _max_flow_
#define _max_flow_

#include <vector>
#include <optional>
#include <utility>

#include "graph_traits.h"
#include "search_settings.h"
#include "breadth_search.h"

template<class flow_t>
class networks_traits_t
{
    using size_t=std::size_t;
    using edge_t=std::tuple<size_t,size_t,flow_t,flow_t>;
    size_t m_source;
    size_t m_sink;
    std::vector<edge_t> m_upper_cap;
    std::vector<edge_t> m_lower_cap;
    public:

    std::vector<std::tuple<size_t,size_t,flow_t>> flows;

    std::vector<edge_t>& upper_cap(){return m_upper_cap;}
    std::vector<edge_t>& lower_cap(){return m_lower_cap;}
    const std::vector<edge_t>& upper_cap()const{return m_upper_cap;}
    const std::vector<edge_t>& lower_cap()const{return m_lower_cap;}
    size_t                     source()const{return m_source;}
    size_t                     sink()const  {return m_sink;}
    networks_traits_t&upper_cap(const std::vector<edge_t>&upp)
    {
        m_upper_cap=upp;
        return *this;
    }
    networks_traits_t&upper_cap(std::vector<edge_t>&&upp)
    {
        m_upper_cap=std::move(upp);
        return *this;
    }

    networks_traits_t&lower_cap(const std::vector<edge_t>&low)
    {
        m_lower_cap=low;
        return *this;
    }
    networks_traits_t&lower_cap(std::vector<edge_t>&&low)
    {
        m_lower_cap=std::move(low);;
        return *this;
    }

    networks_traits_t&source(size_t s)
    {
        m_source=s;
        return *this;
    }
    networks_traits_t&sink(size_t s)
    {
        m_sink=s;
        return *this;
    }
};

namespace __detail_max_flow
{

template<class flow_t>
class residual_network_t
{
    struct pipe_t
    {
        using size_t=std::size_t;
        size_t from;
        size_t to;
        flow_t from_to_cap;
        flow_t to_from_cap;
        flow_t flow;
        bool   flow_forward;
        flow_t residual_capacity(size_t out_node)const
        {
            return out_node==from? (flow_forward? from_to_cap-flow:from_to_cap+flow):
                                   (flow_forward? to_from_cap+flow:to_from_cap-flow);
        }
        void add_flow(size_t out_node,flow_t added)
        {
            if((out_node==from)==flow_forward)
            {
                flow+=added;
            }
            else if(added<=flow)
            {
                flow-=added;
            }
            else
            {
                flow=added-flow;
                flow_forward=!flow_forward;
            }
        }
    };
    public:
    using size_t=std::size_t;
    static constexpr bool is_weighted=true;
    using node_handler_t=size_t;
    using edge_handler_t=std::vector<std::pair<size_t,size_t>>::const_iterator;
    using weight_t=flow_t;
    private:
    std::vector<pipe_t>                                m_pipes;
    std::vector<std::vector<std::pair<size_t,size_t>>> m_adj_list;
    public:

    edge_handler_t edge_begin(node_handler_t n)const
    {
        return m_adj_list[n].begin();
    }
    void edge_inc(node_handler_t n,edge_handler_t&e)const
    {
        ++e;
    }
    bool edge_end(node_handler_t n,edge_handler_t e)const
    {
        return e==m_adj_list[n].end();
    }
    node_handler_t target(node_handler_t n,edge_handler_t e)const
    {
        return e->first;
    }
    size_t index(node_handler_t n)const
    {
        return n;
    }
    flow_t residual_capacity(node_handler_t n,edge_handler_t e)const
    {
        return m_pipes[e->second].residual_capacity(n);
    }
    flow_t weight(node_handler_t n,edge_handler_t e)const
    {
        return residual_capacity(n,e);
    }
    void add_flow(node_handler_t n,edge_handler_t e,flow_t f)
    {
        m_pipes[e->second].add_flow(n,f);
    }
    flow_t max_source_flow(node_handler_t node)const
    {
        flow_t ret{0};
        for(auto [to,edge_ind]:m_adj_list[node])
        {
            ret+=m_pipes[edge_ind].residual_capacity(node);
        }
        return ret;
    }
    flow_t max_sinc_flow(node_handler_t node)const
    {
        flow_t ret{0};
        for(auto [to,edge_ind]:m_adj_list[node])
        {
            ret+=m_pipes[edge_ind].residual_capacity(to);
        }
        return ret;
    }
    void clear()
    {
        m_pipes.clear();
        m_adj_list.clear();
    }
    void set_traits(const networks_traits_t<flow_t>&traits)
    {
        assert(traits.source()!=traits.sink());
        clear();
        size_t num_nodes=0;
        for(auto&[from,to,cap,rev_cap]:traits.upper_cap())
        {
            assert(from!=to);
            num_nodes=std::max({from+1,to+1,num_nodes});
        }
        m_adj_list.resize(num_nodes,{});
        for(auto&[from,to,cap,rev_cap]:traits.upper_cap())
        {
            pipe_t pipe;
            pipe.from=from;
            pipe.to=to;
            pipe.from_to_cap=cap;
            pipe.to_from_cap=rev_cap;
            pipe.flow=flow_t{0};
            pipe.flow_forward=true;
            m_pipes.push_back(pipe);
            m_adj_list[from].push_back({to,m_pipes.size()-1});
            m_adj_list[to].push_back({from,m_pipes.size()-1});
        }
    }
    void get_flows(networks_traits_t<flow_t>&traits)const
    {
        traits.flows.clear();
        for(auto&pipe:m_pipes)
        {
            pipe.flow_forward? traits.flows.push_back({pipe.from,pipe.to,pipe.flow}):
                               traits.flows.push_back({pipe.to,pipe.from,pipe.flow});
        }
    }
    size_t num_nodes()const{return m_adj_list.size();}
};

} //__detail_max_flow

template<class flow_t>
flow_t min_edges_augmentation(networks_traits_t<flow_t>&net_data)
{
    using namespace __detail_max_flow;

    using network_t=residual_network_t<flow_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t network;
    network.set_traits(net_data);

    auto search=search_node_t<network_t>(network,net_data.sink())
    .set_edge_filter([&](node_t out_node,edge_t edge)
    {
        return network.weight(out_node,edge)!=flow_t{0};
    });

    tree_search_t<network_t> tree;
    flow_t total{0};
    while(breadth_search(true,network,net_data.source(),search,tree)==search_termination_t::node_preprocess_id)
    {
        assert(tree.in_tree(net_data.sink()));
        std::optional<flow_t> added;
        tree.traverse_path(net_data.sink(),[&](node_t node,edge_t edge)
        {
            flow_t residual=network.residual_capacity(node,edge);
            added=added? std::min(residual,*added):residual;
        });
        assert(added&&*added>flow_t{0});
        tree.traverse_path(net_data.sink(),[&](node_t node,edge_t edge)
        {
            network.add_flow(node,edge,*added);
        });
        total+=*added;
    }
    network.get_flows(net_data);
    return total;
}

template<class flow_t>
flow_t sparse_max_flow_augmentation(networks_traits_t<flow_t>&data)
{
    using namespace __detail_max_flow;

    using network_t=residual_network_t<flow_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t network;
    network.set_traits(data);
    flow_t mf=std::min(network.max_source_flow(data.source()),network.max_sinc_flow(data.sink()));
    maximize_flow_t<network_t> upd(mf);
    tree_search_t<network_t> tree;
    flow_t total{0};
    while(true)
    {
        sparse_priority_search(network,data.source(),search_node_t(network,data.sink()),tree,upd);
        flow_t added=tree.dist(data.sink());
        if(added==flow_t{0}) break;
        tree.traverse_path(data.sink(),[&](node_t node,edge_t edge)
        {
            network.add_flow(node,edge,added);
        });
        total+=added;
    }
    network.get_flows(data);
    return total;
}

template<class flow_t>
flow_t dense_max_flow_augmentation(networks_traits_t<flow_t>&data)
{
    using namespace __detail_max_flow;

    using network_t=residual_network_t<flow_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t network;
    network.set_traits(data);
    flow_t mf=std::min(network.max_source_flow(data.source()),network.max_sinc_flow(data.sink()));
    maximize_flow_t<network_t> upd(mf);
    tree_search_t<network_t> tree;
    flow_t total{0};
    while(true)
    {
        dense_priority_search(network,data.source(),search_node_t(network,data.sink()),tree,upd);
        flow_t added=tree.dist(data.sink());
        if(added==flow_t{0}) break;
        tree.traverse_path(data.sink(),[&](node_t node,edge_t edge)
        {
            network.add_flow(node,edge,added);
        });
        total+=added;
    }
    network.get_flows(data);
    return total;
}

template<class flow_t>
flow_t preflow_push_relabel_to_front(networks_traits_t<flow_t>&data)
{
    using namespace __detail_max_flow;

    using size_t=std::size_t;
    using network_t=residual_network_t<flow_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t net;
    net.set_traits(data);
    const node_t source=data.source();

    std::vector<size_t> height(net.num_nodes(),0);
    std::vector<edge_t> cur_edge(net.num_nodes());
    std::vector<flow_t> overflow(net.num_nodes(),flow_t{0});
    height[source]=net.num_nodes();

    flow_t total{0};
    auto push_flow=[&](node_t from,edge_t edge)
    {
        node_t to=net.target(from,edge);
        flow_t res_cap=net.residual_capacity(from,edge);
        flow_t pushed;
        if(from==source)
        {
            pushed=res_cap;
        }
        else if(overflow[from]>res_cap)
        {
            pushed=res_cap;
            overflow[from]-=res_cap;
        }
        else
        {
            pushed=overflow[from];
            overflow[from]=flow_t{0};
        }
        net.add_flow(from,edge,pushed);
        if(to!=data.sink()&&to!=data.source())
        {
            overflow[to]+=pushed;
        }
        if(to==data.sink()) total+=pushed;
        return pushed>flow_t{0};
    };
    auto discharge=[&](node_t node)
    {
        size_t old_heght=height[node];
        while(overflow[node]!=flow_t{0})
        {
            if(net.edge_end(node,cur_edge[node]))
            {
                std::optional<size_t>  min_height;
                for(edge_t e=net.edge_begin(node);!net.edge_end(node,e);net.edge_inc(node,e))
                {
                    node_t to=net.target(node,e);
                    if(net.residual_capacity(node,e)>flow_t{0})
                    {
                        min_height=min_height? std::min(*min_height,height[to]):height[to];
                    }
                }
                assert(min_height);
                height[node]=*min_height+1;
                cur_edge[node]=net.edge_begin(node);
            }
            else if(!(height[node]==height[net.target(node,cur_edge[node])]+1&&
                      push_flow(node,cur_edge[node])))
            {
                net.edge_inc(node,cur_edge[node]);
            }
        }
        return old_heght!=height[node];
    };

    for(edge_t e=net.edge_begin(source);!net.edge_end(source,e);net.edge_inc(source,e))
    {
        push_flow(source,e);
    }
    std::vector<node_t> nodes;
    size_t overflowed=0;
    for(size_t i=0;i<net.num_nodes();++i)
    {
        cur_edge[i]=net.edge_begin(i);
        nodes.push_back(i);
        if(overflow[nodes.back()]>flow_t{0})
        {
            std::swap(nodes.back(),nodes[overflowed]);
            ++overflowed;
        }
    }
    for(size_t i=0;i<net.num_nodes();++i)
    {
        if(size_t n=nodes[i];discharge(n))
        {
            std::copy_backward(nodes.begin(),nodes.begin()+i,nodes.begin()+i+1);
            nodes[0]=n;
            i=0;
        }
    }
    assert(find_if(nodes.begin(),nodes.end(),[&](size_t n){return overflow[n]>flow_t{0};})==nodes.end());
    return total;
}


#endif






















