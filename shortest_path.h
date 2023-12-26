
#ifndef _shortest_path_
#define _shortest_path_

#include <vector>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "search_settings.h"
#include "priority_search.h"

/**
    Call list  for all dijkstra_search functions:
    call_list:=graph_t node_t (search_adapter_t|node_t)? tree_search_t
*/

template<class graph_t,class search_t>
search_termination_t dense_dijkstra(graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    search_t search,
                    tree_search_t<graph_t>&tree)
{
    return dense_priority_search(g,sourse,search,tree,minimize_distance_t<graph_t>{});
}

template<class graph_t>
bool dense_dijkstra(graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    typename graph_traits<graph_t>::node_handler_t dest,
                    tree_search_t<graph_t>&tree)
{
    return dense_priority_search(g,sourse,search_node_t{g,dest},tree,minimize_distance_t<graph_t>{})
           ==search_termination_t::node_preprocess_id;
}

template<class graph_t>
void dense_dijkstra(graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    tree_search_t<graph_t>&tree)
{
    dense_priority_search(g,sourse,full_search_t<graph_t>{},tree,minimize_distance_t<graph_t>{});
}

///////////////////////////////////////////////////////////////////////////////////////

template<class graph_t,class search_t>
search_termination_t sparse_dijkstra(graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    search_t search,
                    tree_search_t<graph_t>&tree)
{
    return sparse_priority_search(g,sourse,search,tree,minimize_distance_t<graph_t>{});
}


template<class graph_t>
bool sparse_dijkstra(graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    typename graph_traits<graph_t>::node_handler_t dest,
                    tree_search_t<graph_t>&tree)
{
    return sparse_priority_search(g,sourse,search_node_t{g,dest},tree,minimize_distance_t<graph_t>{})
    ==search_termination_t::node_preprocess_id;
}

template<class graph_t>
void sparse_dijkstra(graph_t&g,
                    typename graph_traits<graph_t>::node_handler_t sourse,
                    tree_search_t<graph_t>&tree)
{
    sparse_priority_search(g,sourse,full_search_t<graph_t>{},tree,minimize_distance_t<graph_t>{});
}



template<class graph_t,class efilter_t,class w_upd_t>
void priority_floyd_warshall(graph_t&graph,
                    const std::vector<typename graph_traits<graph_t>::node_handler_t>&nodes,
                    std::vector<tree_search_t<graph_t>>&trees,
                    efilter_t efilter,
                    w_upd_t upd)
{
    using traits=graph_traits<graph_t>;
    using weight_t=traits::weight_t;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using size_t=std::size_t;
    const size_t num_nodes=graph.size();
    trees.clear();
    trees.resize(num_nodes);
    std::vector<int> nbi(num_nodes);
    auto node_by_index=[&](size_t s)
    {
        assert(nbi[s]>=0);
        return nodes[nbi[s]];
    };
    for(size_t i=0;i<num_nodes;++i)
    {
        size_t index=traits::index(graph,nodes[i]);
        //assert(index==i);
        if(index>=nbi.size()) nbi.resize(index+1,-1);
        nbi[index]=i;
    }
    for(size_t i=0;i<num_nodes;++i)
    {
        trees[i].m_init(graph);
        trees[i].m_resize(num_nodes);
        node_t from=node_by_index(i);
        trees[i].m_resize_for_node(from);
        trees[i].m_prev_node(from)=from;
        trees[i].m_dist(from)=upd.init_weight();
        trees[i].m_state(from)=node_state_t::closed_id;
        for(edge_t e=traits::edge_begin(graph,from);
                    !traits::edge_end(graph,from,e);
                     traits::edge_inc(graph,from,e))
        {
            if(!efilter(from,e)) continue;
            node_t to=traits::target(graph,from,e);
            trees[i].m_prev_node(to)=from;
            trees[i].m_prev_edge(to)=e;
            trees[i].m_dist(to)=traits::weight(graph,from,e);
            trees[i].m_state(to)=node_state_t::closed_id;
        }
    }

    // j(from) -> i(median) -> k (to)
    for(size_t i=0;i<num_nodes;++i)
    {
        node_t median=node_by_index(i);
        for(size_t j=0;j<num_nodes;++j)
        {
            if(!trees[j].in_tree(median)) continue;
            for(size_t k=0;k<num_nodes;++k)
            {
                node_t to=node_by_index(k);
                if(!trees[i].in_tree(to)) continue;
                if(!trees[j].in_tree(to))
                {
                    weight_t new_weight=upd.weight_update(trees[j].m_dist(median),
                                                          trees[i].m_dist(to));
                    trees[j].m_prev_node(to)=trees[i].m_prev_node(to);
                    trees[j].m_prev_edge(to)=trees[i].m_prev_edge(to);
                    trees[j].m_dist(to)=new_weight;
                    trees[j].m_state(to)=node_state_t::closed_id;
                }
                else
                {
                    weight_t new_weight=upd.weight_update(trees[j].m_dist(median),
                                                          trees[i].m_dist(to));
                    if(upd.priority(new_weight,trees[j].m_dist(to)))
                    {
                        trees[j].m_prev_node(to)=trees[i].m_prev_node(to);
                        trees[j].m_prev_edge(to)=trees[i].m_prev_edge(to);
                        trees[j].m_dist(to)=new_weight;
                    }
                }
            }
        }
    }
}


template<class graph_t,class efilter_t=always_bool_t<true>>
void floyd_warshall(graph_t&graph,
                    const std::vector<typename graph_traits<graph_t>::node_handler_t>&nodes,
                    std::vector<tree_search_t<graph_t>>&trees,
                    efilter_t filter=always_bool_t<true>{})
{
    priority_floyd_warshall(graph,nodes,trees,filter,minimize_distance_t<graph_t>{});
}


template<class graph_t,class efilter_t,class w_upd_t>
bool priority_bellman_ford(std::size_t num_nodes,
                  graph_t&graph,
                  typename graph_traits<graph_t>::node_handler_t sourse,
                  tree_search_t<graph_t>&tree,
                  efilter_t efilter,
                  w_upd_t updater)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using weight_t=traits::weight_t;
    using size_t=std::size_t;

    tree.m_init(graph);
    tree.m_resize(num_nodes);
    tree.m_resize_for_node(sourse);
    tree.m_dist(sourse)=updater.init_weight();
    tree.m_prev_node(sourse)=sourse;
    tree.m_state(sourse)=node_state_t::discovered_id;
    std::vector<node_t> queue,aux_queue;
    queue.push_back(sourse);

    auto push_to_queue=[&](node_t node)
    {
        if(auto s=tree.m_state(node);s!=node_state_t::discovered_id)
        {
            tree.m_state(node)=node_state_t::discovered_id;;
            aux_queue.push_back(node);
        }
    };
    auto pop_from_queue=[&]()
    {
        node_t node=queue.back();
        queue.pop_back();
        tree.m_state(node)=node_state_t::closed_id;
        return node;
    };

    for(size_t edge_in_path=0;!queue.empty();++edge_in_path)
    {
        while(!queue.empty())
        {
            node_t from=pop_from_queue();
            for(edge_t e=traits::edge_begin(graph,from);
                        !traits::edge_end(graph,from,e);
                         traits::edge_inc(graph,from,e))
            {
                if(!efilter(from,e)) continue;
                node_t target=traits::target(graph,from,e);
                tree.m_resize_for_node(target);
                weight_t new_dist=updater.weight_update(tree.m_dist(from),traits::weight(graph,from,e));
                if(auto s=tree.m_state(target);s==node_state_t::undiscovered_id||updater.priority(new_dist,tree.m_dist(target)))
                {
                    if(edge_in_path==num_nodes) return false;
                    tree.m_dist(target)=new_dist;
                    tree.m_prev_node(target)=from;
                    tree.m_prev_edge(target)=e;
                    push_to_queue(target);
                }
            }
        }
        std::swap(queue,aux_queue);
        aux_queue.clear();
    }
    return true;
}

template<class graph_t,class efilter_t=always_bool_t<true>>
bool bellman_ford(std::size_t num_nodes,
                  graph_t&graph,
                  typename graph_traits<graph_t>::node_handler_t sourse,
                  tree_search_t<graph_t>&tree,
                  efilter_t filter=always_bool_t<true>{})
{
    return priority_bellman_ford(num_nodes,graph,sourse,tree,filter,minimize_distance_t<graph_t>{});
}


#endif






















