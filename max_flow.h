
/** @file max_flow.h
 *  Maximal flow algorithms:
 *  sparse_max_flow_augmentation
 *  dense_max_flow_augmentation
 *  preflow_push_relabel_to_front
 *  preflow_push_fifo
 *  dinic_max_flow
 *  preflow_push_highest_label
*/

#ifndef _liga_max_flow_
#define _liga_max_flow_

#include <vector>
#include <optional>
#include <utility>
#include <type_traits>

#include "graph_adapter.h"
#include "search_settings.h"
#include "breadth_search.h"
#include "priority_search.h"
#include "bellman_ford.h"


namespace liga{

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

template<class p_t>
struct network_traits_t
{
    using size_t=std::size_t;
    using pipe_t=p_t;
    using flow_t=pipe_t::flow_t;
    size_t source;
    size_t sink;
    std::vector<pipe_t> pipes;
    void clear()
    {
        pipes.clear();
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
    using residual_pipe_t=liga::__detail_flows::residual_pipe_t<pipe_t>;
    using node_handler_t=size_t;
    using edge_handler_t=std::vector<std::pair<size_t,size_t>>::const_iterator;
    using adjacency_list_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    private:
    std::vector<residual_pipe_t> *m_pipes=nullptr;
    adjacency_list_t             *m_adj_list=nullptr;
    public:
    residual_network_t(std::vector<residual_pipe_t>&pipes,adjacency_list_t&adjacency_list):
    m_pipes(&pipes),
    m_adj_list(&adjacency_list)
    {
    }
    const residual_pipe_t&pipe(edge_handler_t&e)const
    {
        return (*m_pipes)[e->second];
    }
    edge_handler_t edge_begin(node_handler_t n)const
    {
        return (*m_adj_list)[n].begin();
    }
    void edge_inc(node_handler_t n,edge_handler_t&e)const
    {
        ++e;
    }
    bool edge_end(node_handler_t n,edge_handler_t e)const
    {
        return e==(*m_adj_list)[n].end();
    }
    node_handler_t target(node_handler_t n,edge_handler_t e)const
    {
        return e->first;
    }

    flow_t residual_capacity(node_handler_t n,edge_handler_t e)const
    {
        return (*m_pipes)[e->second].residual_capacity(n);
    }

    void add_flow(node_handler_t n,edge_handler_t e,flow_t f)
    {
        (*m_pipes)[e->second].add_flow(n,f);
    }
    flow_t max_source_flow(node_handler_t node)const
    {
        flow_t ret{0};
        for(auto [to,edge_ind]:(*m_adj_list)[node])
        {
            ret+=(*m_pipes)[edge_ind].residual_capacity(node);
        }
        return ret;
    }
    flow_t max_sinc_flow(node_handler_t node)const
    {
        flow_t ret{0};
        for(auto [to,edge_ind]:(*m_adj_list)[node])
        {
            ret+=(*m_pipes)[edge_ind].residual_capacity(to);
        }
        return ret;
    }
    void clear()
    {
        m_pipes->clear();
        m_adj_list->clear();
    }
    void build(const std::vector<pipe_t>&pipes)
    {
        clear();
        size_t num_nodes=0;
        for(auto&pipe:pipes)
        {
            assert(pipe.from!=pipe.to);
            num_nodes=std::max({pipe.from+1,pipe.to+1,num_nodes});
        }
        m_adj_list->resize(num_nodes,{});
        for(auto&pipe:pipes)
        {
            m_pipes->push_back(pipe);
            (*m_adj_list)[pipe.from].push_back({pipe.to,m_pipes->size()-1});
            (*m_adj_list)[pipe.to].push_back({pipe.from,m_pipes->size()-1});
        }
    }
    residual_pipe_t&add_pipe(const pipe_t&pipe)
    {
        size_t num_nodes=std::max({pipe.from+1,pipe.to+1,m_adj_list->size()});
        if(num_nodes>m_adj_list->size()) m_adj_list->resize(num_nodes,{});
        m_pipes->push_back(pipe);
        (*m_adj_list)[pipe.from].push_back({pipe.to,m_pipes->size()-1});
        (*m_adj_list)[pipe.to].push_back({pipe.from,m_pipes->size()-1});
        return m_pipes->back();
    }
    void get_flows(std::vector<std::tuple<size_t,size_t,flow_t>>&flows)const
    {
        flows.clear();
        for(auto&pipe:*m_pipes)
        {
            flows.push_back(pipe.get_flow());
        }
    }

    size_t num_nodes()const{return m_adj_list->size();}
};



template<class pipe_t,class ftr_t>
bool discharge_or_relabel(std::size_t node,
                          residual_network_t<pipe_t> net,
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

/**
 *  @brief Find maximal flow using flow augmentation along edge-shortest path in residual network.
 *  @param net_data     Network data:source,sink and network edges with its capacities.
 *  @param flows        Network edges with its flows.
 *  @return             Maximal flow.
*/

template<class pipe_t>
typename pipe_t::flow_t min_edges_augmentation(const network_traits_t<pipe_t>&net_data,
                                               std::vector<std::tuple<std::size_t,
                                                                      std::size_t,
                                                                      typename pipe_t::flow_t>>&flows)
{
    using namespace __detail_flows;

    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t network(pipes,list);
    network.build(net_data.pipes);

    auto search=search_node_t(net_data.sink).set_edge_filter([network](node_t out_node,edge_t edge)
    {
        return network.residual_capacity(out_node,edge)!=flow_t{0};
    });

    tree_search_t<node_t,edge_t,std::size_t,default_index_map_t> tree;
    flow_t total{0};
    while(breadth_search(true,network,net_data.source,search,tree)==search_termination_t::node_preprocess_id)
    {
        assert(tree.in_tree(net_data.sink));
        std::optional<flow_t> added;
        tree.traverse_path(net_data.sink,[&added,network](node_t node,edge_t edge)
        {
            flow_t residual=network.residual_capacity(node,edge);
            added=added? std::min(residual,*added):residual;
        });
        assert(added>flow_t{0});
        tree.traverse_path(net_data.sink,[=](node_t node,edge_t edge)mutable
        {
            network.add_flow(node,edge,*added);
        });
        total+=*added;
    }
    network.get_flows(flows);
    return total;
}

/**
 *  @brief Find maximal flow using flow augmentation along max-capacity path in residual network.
 *  @param net_data     Network data:source,sink and network edges with its capacities.
 *  @param flows        Network edges with its flows.
 *  @return             Maximal flow.
 *
 * Version, optimized for sparse networks.
*/

template<class pipe_t>
typename pipe_t::flow_t sparse_max_flow_augmentation(const network_traits_t<pipe_t>&data,
                                                     std::vector<std::tuple<std::size_t,
                                                                            std::size_t,
                                                                            typename pipe_t::flow_t>>&flows)
{
    using namespace __detail_flows;

    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t network(pipes,list);
    network.build(data.pipes);

    auto weight_map=[=](node_t n,edge_t e)
    {
        return network.residual_capacity(n,e);
    };
    flow_t mf=std::min(network.max_source_flow(data.source),network.max_sinc_flow(data.sink));
    maximize_flow_t<flow_t> upd(mf);
    tree_search_t<node_t,edge_t,flow_t,default_index_map_t> tree;
    flow_t total{0};
    while(true)
    {
        sparse_priority_search(network,data.source,search_node_t(data.sink),tree,weight_map,upd);
        flow_t added=tree.dist(data.sink);
        if(added==flow_t{0}) break;
        tree.traverse_path(data.sink,[=](node_t node,edge_t edge)mutable
        {
            network.add_flow(node,edge,added);
        });
        total+=added;
    }
    network.get_flows(flows);
    return total;
}

/**
 *  @brief Find maximal flow using flow augmentation along max-capacity path in residual network.
 *  @param net_data     Network data:source,sink and network edges with its capacities.
 *  @param flows        Network edges with its flows.
 *  @return             Maximal flow.
 *
 * Version, optimized for dense networks.
*/

template<class pipe_t>
typename pipe_t::flow_t dense_max_flow_augmentation(const network_traits_t<pipe_t>&data,
                                                    std::vector<std::tuple<std::size_t,
                                                                           std::size_t,
                                                                           typename pipe_t::flow_t>>&flows)
{
    using namespace __detail_flows;

    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t network(pipes,list);
    network.build(data.pipes);

    auto weight_map=[=](node_t n,edge_t e)
    {
        return network.residual_capacity(n,e);
    };
    flow_t mf=std::min(network.max_source_flow(data.source),network.max_sinc_flow(data.sink));
    maximize_flow_t<flow_t> upd(mf);
    tree_search_t<node_t,edge_t,flow_t,default_index_map_t> tree;
    flow_t total{0};
    while(true)
    {
        dense_priority_search(network,data.source,search_node_t(data.sink),tree,weight_map,upd);
        flow_t added=tree.dist(data.sink);
        if(added==flow_t{0}) break;
        tree.traverse_path(data.sink,[=](node_t node,edge_t edge)mutable
        {
            network.add_flow(node,edge,added);
        });
        total+=added;
    }
    network.get_flows(flows);
    return total;
}

/**
 *  @brief Find maximal flow using push-preflow algorithm.
 *  @param net_data     Network data:source,sink and network edges with its capacities.
 *  @param flows        Network edges with its flows.
 *  @return             Maximal flow.
 *
 *  Overflowed nodes discharge in topological sort order of residual network.
*/

template<class pipe_t>
typename pipe_t::flow_t preflow_push_relabel_to_front(const network_traits_t<pipe_t>&data,
                                                      std::vector<std::tuple<std::size_t,
                                                                             std::size_t,
                                                                             typename pipe_t::flow_t>>&flows)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t net(pipes,list);
    net.build(data.pipes);

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
    net.get_flows(flows);
    return overflow[data.sink];
}

/**
 *  @brief Find maximal flow using push-preflow algorithm.
 *  @param net_data     Network data:source,sink and network edges with its capacities.
 *  @param flows        Network edges with its flows.
 *  @return             Maximal flow.
 *
 *  Overflowed nodes discharge in FIFO-order: discharged nodes removed from head of queue,
 *  new overflowed nodes pushes in tail of queue.
*/

template<class pipe_t>
typename pipe_t::flow_t preflow_push_fifo(const network_traits_t<pipe_t>&data,
                                          std::vector<std::tuple<std::size_t,
                                                                 std::size_t,
                                                                 typename pipe_t::flow_t>>&flows)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t net(pipes,list);
    net.build(data.pipes);

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
    net.get_flows(flows);
    return res;

}

/**
 *  @brief Find maximal flow using push-preflow algorithm.
 *  @param net_data     Network data:source,sink and network edges with its capacities.
 *  @param flows        Network edges with its flows.
 *  @return             Maximal flow.
 *
 *  Overflowed nodes discharges in highest-label order.
*/

template<class pipe_t>
typename pipe_t::flow_t preflow_push_highest_label(const network_traits_t<pipe_t>&data,
                                                   std::vector<std::tuple<std::size_t,
                                                                          std::size_t,
                                                                          typename pipe_t::flow_t>>&flows)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t net(pipes,list);
    net.build(data.pipes);

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
    net.get_flows(flows);
    return res;
}

/**
 *  @brief Find maximal flow using Dinic algorithm with O(V*V*E) complexity.
 *  @param net_data     Network data:source,sink and network edges with its capacities.
 *  @param flows        Network edges with its flows.
 *  @return             Maximal flow.
 *
 *  Algorithm consist of O(V) phases: on i-th phase find all augment flows with
 *  exactly i edges in augment path and from phase to phase i strongly increased.
*/

template<class pipe_t>
typename pipe_t::flow_t dinic_max_flow(const network_traits_t<pipe_t>&data,
                                       std::vector<std::tuple<std::size_t,
                                                              std::size_t,
                                                              typename pipe_t::flow_t>>&flows)
{
    using namespace __detail_flows;

    using size_t=std::size_t;
    using flow_t=pipe_t::flow_t;
    using network_t=residual_network_t<pipe_t>;
    using node_t=network_t::node_handler_t;
    using edge_t=network_t::edge_handler_t;

    std::vector<residual_pipe_t<pipe_t>> pipes;
    typename network_t::adjacency_list_t list;
    network_t net(pipes,list);
    net.build(data.pipes);

    std::vector<edge_t> cur_edge(net.num_nodes());
    std::vector<flow_t> edge_cap(net.num_nodes());
    tree_search_t<node_t,edge_t,flow_t,default_index_map_t> tree;
    flow_t total{0};

    auto search=search_node_t<node_t>(data.sink).set_edge_filter([=](node_t n,edge_t e)
    {
        return net.residual_capacity(n,e)!=flow_t{0};
    });

    auto is_feasible_edge=[&,net](node_t from)
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
    net.get_flows(flows);
    return total;
}

}// liga

#endif






















