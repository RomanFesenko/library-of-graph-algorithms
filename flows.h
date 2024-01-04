
#ifndef _flows_
#define _flows_

#include <vector>
#include <optional>
#include <utility>
#include <type_traits>


#include "graph_traits.h"
#include "search_settings.h"
#include "breadth_search.h"


template<class f_t>
struct direct_pipe_t
{
    using size_t=std::size_t;
    using flow_t=f_t;

    size_t from;
    size_t to;
    flow_t capacity;
};

template<class f_t>
struct bidirect_pipe_t
{
    using size_t=std::size_t;
    using flow_t=f_t;

    size_t from;
    size_t to;
    flow_t forward_capacity;
    flow_t backward_capacity;
};

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

template<class p_t>
struct networks_traits_t
{
    using size_t=std::size_t;
    using pipe_t=p_t;
    using flow_t=pipe_t::flow_t;

    size_t source;
    size_t sink;
    std::vector<pipe_t> pipes;

    std::vector<std::tuple<size_t,size_t,flow_t>> flows;


    networks_traits_t&set_pipes(const std::vector<pipe_t>&p)
    {
        pipes=p;
        return *this;
    }
    networks_traits_t&set_pipes(std::vector<pipe_t>&&p)
    {
        pipes=std::move(p);
        return *this;
    }

    networks_traits_t&set_source(size_t s)
    {
        source=s;
        return *this;
    }
    networks_traits_t&set_sink(size_t s)
    {
        sink=s;
        return *this;
    }
    void clear()
    {
        pipes.clear();
        flows.clear();
    }
};

namespace __detail_flows
{

template<class pipe_t>
struct residual_pipe_t;

template<class bp_t>
struct residual_pipe_t:public bp_t
{
    using base_pipe_t=bp_t;
    using flow_t=base_pipe_t::flow_t;

    flow_t flow;

    residual_pipe_t(const base_pipe_t&bp):bp_t(bp)
    {
        flow=flow_t{0};
    }

    flow_t residual_capacity(size_t out_node)const
    {
        return out_node==this->from? this->capacity-flow :flow;
    }
    void add_flow(size_t out_node,flow_t added)
    {
        if(out_node==this->from)
        {
            flow+=added;
        }
        else
        {
            flow-=added;
        }
    }

    std::tuple<size_t,size_t,flow_t> get_flow()const
    {
        return {this->from,this->to,flow};
    }
};

template<class flow_t>
struct residual_pipe_t<bidirect_pipe_t<flow_t>>:public bidirect_pipe_t<flow_t>
{
    using base_pipe_t=bidirect_pipe_t<flow_t>;

    flow_t forward_residual;
    flow_t backward_residual;

    residual_pipe_t(const base_pipe_t&bp):bidirect_pipe_t<flow_t>(bp)
    {
        forward_residual=this->forward_capacity;
        backward_residual=this->backward_capacity;
    }

    flow_t residual_capacity(size_t out_node)const
    {
        return out_node==this->from? forward_residual:backward_residual;
    }
    void add_flow(size_t out_node,flow_t added)
    {
        if(out_node==this->from)
        {
            forward_residual-=added;
            backward_residual+=added;
        }
        else
        {
            forward_residual+=added;
            backward_residual-=added;
        }
    }

    std::tuple<size_t,size_t,flow_t> get_flow()const
    {
        if(this->forward_capacity>=forward_residual)
        {
            return {this->from,this->to,this->forward_capacity-forward_residual};
        }
        else
        {
            assert(this->backward_capacity>=backward_residual);
            return {this->to,this->from,this->backward_capacity-backward_residual};
        }
    }
};


template<class pipe_t>
class residual_network_t
{
    public:
    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using residual_pipe_t=residual_pipe_t<pipe_t>;
    static constexpr bool is_weighted=false;
    using weight_t=void;
    using node_handler_t=size_t;
    using edge_handler_t=std::vector<std::pair<size_t,size_t>>::const_iterator;
    private:
    std::vector<residual_pipe_t>                       m_pipes;
    std::vector<std::vector<std::pair<size_t,size_t>>> m_adj_list;
    public:

    const residual_pipe_t&pipe(edge_handler_t&e)const
    {
        return m_pipes[e->second];
    }
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
    void set_traits(const networks_traits_t<pipe_t>&traits)
    {
        assert(traits.source!=traits.sink);
        clear();
        size_t num_nodes=0;
        for(auto&pipe:traits.pipes)
        {
            assert(pipe.from!=pipe.to);
            num_nodes=std::max({pipe.from+1,pipe.to+1,num_nodes});
        }
        m_adj_list.resize(num_nodes,{});
        for(auto&pipe:traits.pipes)
        {
            m_pipes.push_back(pipe);
            m_adj_list[pipe.from].push_back({pipe.to,m_pipes.size()-1});
            m_adj_list[pipe.to].push_back({pipe.from,m_pipes.size()-1});
        }
    }
    void get_flows(networks_traits_t<pipe_t>&traits)const
    {
        traits.flows.clear();
        for(auto&pipe:m_pipes)
        {
            traits.flows.push_back(pipe.get_flow());
        }
    }
    size_t num_nodes()const{return m_adj_list.size();}
};

template<class pipe_t,class ftr_t>
bool discharge_or_relabel(std::size_t node,
                          residual_network_t<pipe_t>&net,
                          std::vector<typename residual_network_t<pipe_t>::edge_handler_t>&cur_edge,
                          std::vector<typename pipe_t::flow_t>&overflow,
                          std::vector<std::size_t>&height,
                          ftr_t ftr)
{
    using network_t=residual_network_t<pipe_t>;
    using flow_t=network_t::flow_t;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;
    auto push_flow=[&](node_t from,edge_t edge)
    {
        node_t to=net.target(from,edge);
        flow_t res_cap=net.residual_capacity(from,edge);
        if(res_cap==flow_t{0}) return false;

        flow_t pushed;
        if(overflow[from]>res_cap)
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
        overflow[to]+=pushed;
        ftr(to);
        return true;
    };

    while(overflow[node]!=flow_t{0})
    {
        if(net.edge_end(node,cur_edge[node]))
        {
            for(edge_t e=net.edge_begin(node);!net.edge_end(node,e);net.edge_inc(node,e))
            {
                node_t to=net.target(node,e);
                if(net.residual_capacity(node,e)>flow_t{0}&&
                  (net.edge_end(node,cur_edge[node])||height[to]<height[node]))
                {
                    cur_edge[node]=e;
                    height[node]=height[to];
                }
            }
            ++height[node];
            return true;
        }
        else if(!(height[node]==height[net.target(node,cur_edge[node])]+1&&
                 push_flow(node,cur_edge[node])))
        {
            net.edge_inc(node,cur_edge[node]);
        }
    }
    return false;
}

} //__detail_flows

template<class pipe_t>
typename pipe_t::flow_t min_edges_augmentation(networks_traits_t<pipe_t>&net_data)
{
    using namespace __detail_flows;

    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t network;
    network.set_traits(net_data);

    auto search=search_node_t<network_t>(network,net_data.sink)
    .set_edge_filter([&](node_t out_node,edge_t edge)
    {
        return network.residual_capacity(out_node,edge)!=flow_t{0};
    });

    tree_search_t<network_t,std::size_t> tree;
    flow_t total{0};
    while(breadth_search(true,network,net_data.source,search,tree)==search_termination_t::node_preprocess_id)
    {
        assert(tree.in_tree(net_data.sink));
        std::optional<flow_t> added;
        tree.traverse_path(net_data.sink,[&](node_t node,edge_t edge)
        {
            flow_t residual=network.residual_capacity(node,edge);
            added=added? std::min(residual,*added):residual;
        });
        assert(added>flow_t{0});
        tree.traverse_path(net_data.sink,[&](node_t node,edge_t edge)
        {
            network.add_flow(node,edge,*added);
        });
        total+=*added;
    }
    network.get_flows(net_data);
    return total;
}

template<class pipe_t>
typename pipe_t::flow_t sparse_max_flow_augmentation(networks_traits_t<pipe_t>&data)
{
    using namespace __detail_flows;

    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t network;
    network.set_traits(data);
    auto wnet=make_weighted_graph(network,[&](node_t n,edge_t e)
    {
        return network.residual_capacity(n,e);
    });

    flow_t mf=std::min(network.max_source_flow(data.source),network.max_sinc_flow(data.sink));
    maximize_flow_t<decltype(wnet)> upd(mf);
    tree_search_t<decltype(wnet)> tree;
    flow_t total{0};
    while(true)
    {
        sparse_priority_search(wnet,data.source,search_node_t(network,data.sink),tree,upd);
        flow_t added=tree.dist(data.sink);
        if(added==flow_t{0}) break;
        tree.traverse_path(data.sink,[&](node_t node,edge_t edge)
        {
            network.add_flow(node,edge,added);
        });
        total+=added;
    }
    network.get_flows(data);
    return total;
}

template<class pipe_t>
typename pipe_t::flow_t dense_max_flow_augmentation(networks_traits_t<pipe_t>&data)
{
    using namespace __detail_flows;

    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t network;
    network.set_traits(data);
    auto wnet=make_weighted_graph(network,[&](node_t n,edge_t e)
    {
        return network.residual_capacity(n,e);
    });

    flow_t mf=std::min(network.max_source_flow(data.source),network.max_sinc_flow(data.sink));
    maximize_flow_t<decltype(wnet)> upd(mf);
    tree_search_t<decltype(wnet)> tree;
    flow_t total{0};
    while(true)
    {
        dense_priority_search(wnet,data.source,search_node_t(network,data.sink),tree,upd);
        flow_t added=tree.dist(data.sink);
        if(added==flow_t{0}) break;
        tree.traverse_path(data.sink,[&](node_t node,edge_t edge)
        {
            network.add_flow(node,edge,added);
        });
        total+=added;
    }
    network.get_flows(data);
    return total;
}

template<class pipe_t>
typename pipe_t::flow_t preflow_push_relabel_to_front(networks_traits_t<pipe_t>&data)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t net;
    net.set_traits(data);
    const node_t source=data.source;
    const node_t sink=data.sink;

    std::vector<edge_t> cur_edge(net.num_nodes());
    std::vector<flow_t> overflow(net.num_nodes(),flow_t{0});
    std::vector<size_t> height(net.num_nodes(),0);
    height[source]=net.num_nodes();

    std::vector<node_t> nodes;
    for(edge_t e=net.edge_begin(source);!net.edge_end(source,e);net.edge_inc(source,e))
    {
        node_t target=net.target(source,e);
        flow_t res=net.residual_capacity(source,e);
        net.add_flow(source,e,res);
        overflow[target]+=res;
        if(res>flow_t{0}&&target!=sink) nodes.push_back(target);
    }
    for(size_t i=0;i<net.num_nodes();++i)
    {
        cur_edge[i]=net.edge_begin(i);
        if(i==source||i==sink||overflow[i]>flow_t{0}) continue;
        nodes.push_back(i);
    }
    size_t num_relabeled=0;
    for(size_t i=0;i<nodes.size();)
    {
        if(size_t n=nodes[i];discharge_or_relabel<pipe_t>(n,net,cur_edge,overflow,height,[](size_t){}))
        {
            ++num_relabeled;
            std::copy_backward(nodes.begin(),nodes.begin()+i,nodes.begin()+i+1);
            nodes[0]=n;
            i=0;
        }
        else ++i;
    }
    assert(num_relabeled<=(2*net.num_nodes()-2)*(net.num_nodes()-2));
    assert(std::all_of(nodes.begin(),nodes.end(),[&](node_t n){return overflow[n]==flow_t{0};}));
    net.get_flows(data);
    return overflow[data.sink];
}


template<class pipe_t>
typename pipe_t::flow_t preflow_push_fifo(networks_traits_t<pipe_t>&data)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t net;
    net.set_traits(data);
    const node_t source=data.source;
    const node_t sink=data.sink;

    std::vector<edge_t> cur_edge(net.num_nodes());
    std::vector<size_t> height(net.num_nodes(),0);
    std::vector<flow_t> overflow(net.num_nodes(),flow_t{0});

    std::vector<node_t> queue,aux_queue;
    std::vector<char>   in_queue(net.num_nodes(),0);
    height[source]=net.num_nodes();

    auto push_to_queue=[&](node_t node)
    {
        if(node!=sink&&node!=source&&!in_queue[node])
        {
            queue.push_back(node);
            in_queue[node]=1;
        }
    };

    for(size_t n=0;n<net.num_nodes();++n)
    {
        cur_edge[n]=net.edge_begin(n);
    }

    for(edge_t e=net.edge_begin(source);!net.edge_end(source,e);net.edge_inc(source,e))
    {
        node_t target=net.target(source,e);
        flow_t res=net.residual_capacity(source,e);
        net.add_flow(source,e,res);
        push_to_queue(target);
        overflow[target]+=res;
    }

    size_t num_relabeled=0;
    while(!queue.empty())
    {
        std::swap(queue,aux_queue);
        queue.clear();
        while(!aux_queue.empty())
        {
            node_t overflowed=aux_queue.back();
            aux_queue.pop_back();
            in_queue[overflowed]=0;
            if(discharge_or_relabel<pipe_t>(overflowed,net,cur_edge,overflow,height,push_to_queue))
            {
                ++num_relabeled;
                queue.push_back(overflowed);
                in_queue[overflowed]=1;
            }
        }
    }
    assert(num_relabeled<=(2*net.num_nodes()-2)*(net.num_nodes()-2));
    flow_t res=overflow[data.sink];
    overflow[data.source]=overflow[data.sink]=flow_t{0};
    assert(std::all_of(overflow.begin(),overflow.end(),[&](flow_t f){return f==flow_t{0};}));
    net.get_flows(data);
    return res;

}

template<class pipe_t>
typename pipe_t::flow_t preflow_push_highest_label(networks_traits_t<pipe_t>&data)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t net;
    net.set_traits(data);
    const node_t source=data.source;
    const node_t sink=data.sink;

    std::vector<edge_t> cur_edge(net.num_nodes());
    std::vector<size_t> height(net.num_nodes(),0);
    std::vector<flow_t> overflow(net.num_nodes(),flow_t{0});

    std::vector<std::vector<node_t>> queue(2*net.num_nodes()-1);
    std::vector<char> in_queue(net.num_nodes(),0);
    height[source]=net.num_nodes();

    auto push_to_queue=[&](node_t node)
    {
        if(node!=sink&&node!=source&&!in_queue[node])
        {
            queue[height[node]].push_back(node);
            in_queue[node]=1;
        }
    };

    for(size_t n=0;n<net.num_nodes();++n)
    {
        cur_edge[n]=net.edge_begin(n);
    }

    for(edge_t e=net.edge_begin(source);!net.edge_end(source,e);net.edge_inc(source,e))
    {
        node_t target=net.target(source,e);
        flow_t res=net.residual_capacity(source,e);
        net.add_flow(source,e,res);
        push_to_queue(target);
        overflow[target]+=res;
    }

    size_t num_relabeled=0;
    for(size_t max_height=0;max_height!=0||!queue[0].empty();)
    {
        if(queue[max_height].empty())
        {
            --max_height;
        }
        else
        {
            node_t node=queue[max_height].back();
            queue[max_height].pop_back();
            in_queue[node]=0;
            if(discharge_or_relabel<pipe_t>(node,net,cur_edge,overflow,height,push_to_queue))
            {
                ++num_relabeled;
                max_height=height[node];
                queue[max_height].push_back(node);
                in_queue[node]=1;
                assert(queue[max_height].size()==1);
            }
        }
    }
    assert(num_relabeled<=(2*net.num_nodes()-2)*(net.num_nodes()-2));
    flow_t res=overflow[data.sink];
    overflow[data.source]=overflow[data.sink]=flow_t{0};
    assert(std::all_of(overflow.begin(),overflow.end(),[&](flow_t f){return f==flow_t{0};}));
    net.get_flows(data);
    return res;
}


template<class pipe_t>
typename pipe_t::flow_t dinic_max_flow(networks_traits_t<pipe_t>&data)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t net;
    net.set_traits(data);
    std::vector<edge_t> cur_edge(net.num_nodes());
    std::vector<flow_t> edge_cap(net.num_nodes());
    tree_search_t<network_t,std::size_t> tree;
    flow_t total{0};

    auto search=search_node_t<network_t>(net,data.sink).set_edge_filter([&](node_t n,edge_t e)
    {
        return net.residual_capacity(n,e)!=flow_t{0};
    });

    auto is_feasible_edge=[&](node_t from)
    {
        edge_cap[from]=net.residual_capacity(from,cur_edge[from]);
        if(edge_cap[from]==flow_t{0}) return false;
        node_t to=net.target(from,cur_edge[from]);
        return tree.opt_dist(to)>tree.opt_dist(from)&&!net.edge_end(to,cur_edge[to]);
    };

    size_t phases=0;
    while(breadth_search(true,net,data.source,search,tree)==search_termination_t::node_preprocess_id)
    {
        phases++;
        assert(phases<=net.num_nodes()-1);
        for(size_t node=0;node<net.num_nodes();++node)
        {
            cur_edge[node]=net.edge_begin(node);
        }
        while(true)
        {
            std::vector<node_t> stack{data.source};
            while(!stack.empty()&&stack.back()!=data.sink)
            {
                node_t from=stack.back();
                if(net.edge_end(from,cur_edge[from]))
                {
                    stack.pop_back();
                }
                else if(!is_feasible_edge(from))
                {
                    net.edge_inc(from,cur_edge[from]);
                }
                else
                {
                    node_t to=net.target(from,cur_edge[from]);
                    assert(tree.opt_dist(to)&&*tree.opt_dist(to)==*tree.opt_dist(from)+1);
                    stack.push_back(to);
                }
            }
            if(!stack.empty())
            {
                stack.pop_back();
                assert(!stack.empty());
                node_t bottleneck=*std::min_element(stack.begin(),stack.end(),[&](node_t n1,node_t n2)
                {
                    return edge_cap[n1]<edge_cap[n2];
                });
                for(node_t node:stack)
                {
                    net.add_flow(node,cur_edge[node],edge_cap[bottleneck]);
                }
                total+=edge_cap[bottleneck];
                assert(edge_cap[bottleneck]>flow_t{0});
            }
            else break;
        }
    }
    net.get_flows(data);
    return total;
}

/**
=========================================================================|
                                                                         |
                        Minimal cost flow                                |
                                                                         |
=========================================================================|
*/

template<class flow_t,class cost_t>
std::optional<cost_t>
sparse_min_cost_flow(networks_traits_t<costed_direct_pipe_t<flow_t,cost_t>>&data,flow_t req_flow)
{
    using namespace __detail_flows;

    using network_t=residual_network_t<costed_direct_pipe_t<flow_t,cost_t>>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    network_t net;
    net.set_traits(data);
    std::vector<cost_t> pot(net.num_nodes(),cost_t{0});

    auto wnet=make_weighted_graph(net,[&](node_t n,edge_t e)->cost_t
    {
        auto&pipe=net.pipe(e);
        if(pipe.from==n)
        {
            return pipe.cost+pot[n]-pot[pipe.to];
        }
        else
        {
            return pot[pipe.to]-pot[pipe.from]-pipe.cost;
        }
    });

    auto search=search_node_t<network_t>(net,data.sink).set_edge_filter([&](node_t n,edge_t e)
    {
        return net.residual_capacity(n,e)>flow_t{0};
    });

    minimize_distance_t<decltype(wnet)> upd;

    tree_search_t<decltype(wnet)> tree;
    flow_t total_flow{0};
    cost_t total_cost{0};
    while(total_flow<req_flow&&sparse_priority_search(wnet,data.source,search,tree,upd)==
          search_termination_t::node_preprocess_id)
    {
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
        //std::cout<<"total_flow+=added:"<<total_flow<<'\n';
        for(std::size_t i=0;i<net.num_nodes();++i)
        {
            if(auto opt=tree.opt_dist(i);opt)
            {
                pot[i]+=*opt;
            }
        }
    }
    net.get_flows(data);
    return total_flow==req_flow? total_cost: std::optional<cost_t>{};
}


#endif






















