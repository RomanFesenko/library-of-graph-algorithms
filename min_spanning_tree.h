
/** @file min_spanning_tree.h
 *  Minimal spanning tree algorithms::
 *  dense_prim
 *  sparse_prim
 *  kruskal
*/

#ifndef _liga_min_spanning_tree_
#define _liga_min_spanning_tree_

#include <vector>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "search_settings.h"
#include "priority_search.h"
#include "breadth_search.h"
#include "union_find.h"

namespace liga{

namespace __detail_min_tree
{

template<class tree_t,class filter_t>
struct mst_search_adapter_t
{
    using size_t=std::size_t;
    using node_t=  tree_t::node_t;
    using edge_t=  tree_t::edge_t;
    using weight_t=tree_t::label_t;

    const tree_t&              tree;
    std::pair<weight_t,size_t>&tree_param;
    filter_t                   filter;

    mst_search_adapter_t(const tree_t&t,std::pair<weight_t,size_t>&param,filter_t f={}):
    tree(t),tree_param(param),filter(f)
    {
    }
    constexpr bool node_preprocess(node_t n)const
    {
        assert(tree.opt_dist(n));
        ++tree_param.second;
        tree_param.first+=tree.dist(n);
        return true;
    }
    constexpr bool edge_filter(node_t n,edge_t e)const{return filter(n,e);}
    constexpr bool edge_process(node_t node,edge_t edge)const{return true;}
};

}

/**
 *  @brief  Find minimal spanning tree in dense graph, using Prim algorithm.
 *  @param  graph       Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  source      Any node of the graph.
 *  @param  tree        Result minimal spanning tree rooted in source.
 *  @param  weight_map  Weight functor of type (out node, edge)->weight.
 *  @param  filter      Predicate - edge filter.
 *  @return             Pair {tree weight,number edges in tree }.
 *
 * Complexity: O(V*V) in worst case.
*/

template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class filter_t=always_bool_t<true>>
std::pair<weight_t,std::size_t>
dense_prim(graph_t graph,
           typename graph_t::node_handler_t sourse,
           tree_search_t<typename graph_t::node_handler_t,
                         typename graph_t::edge_handler_t,
                         weight_t,
                         index_map_t>&tree,
           weight_map_t weight_map,
           filter_t filter=always_bool_t<true>{})
{

    using tree_t=tree_search_t<typename graph_t::node_handler_t,
                               typename graph_t::edge_handler_t,
                               weight_t,
                               index_map_t>;
    std::pair<weight_t,std::size_t> param;
    __detail_min_tree::mst_search_adapter_t<tree_t,filter_t> search(tree,param,filter);
    dense_priority_search(graph,sourse,search,tree,weight_map,minimize_edge_t<weight_t>{});
    return param;
}

/**
 *  @brief  Find minimal spanning tree in sparse graph, using Prim algorithm.
 *  @param  graph       Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  source      Any node of the graph.
 *  @param  tree        Result minimal spanning tree rooted in source.
 *  @param  weight_map  Weight functor of type (out node, edge)->weight.
 *  @param  filter      Predicate - edge filter.
 *  @return             Pair {tree weight,number edges in tree }.
 *
 * Complexity: O(E*ln(V)) in worst case.
*/

template<class graph_t,class weight_t,class index_map_t,class weight_map_t,class filter_t=always_bool_t<true>>
std::pair<weight_t,std::size_t>
sparse_prim(graph_t graph,
            typename graph_t::node_handler_t sourse,
            tree_search_t<typename graph_t::node_handler_t,
                          typename graph_t::edge_handler_t,
                          weight_t,
                          index_map_t>&tree,
            weight_map_t weight_map,
            filter_t filter=always_bool_t<true>{})
{
    using tree_t=tree_search_t<typename graph_t::node_handler_t,
                               typename graph_t::edge_handler_t,
                               weight_t,
                               index_map_t>;
    std::pair<weight_t,std::size_t> param;
    __detail_min_tree::mst_search_adapter_t<tree_t,filter_t> search(tree,param,filter);
    sparse_priority_search(graph,sourse,search,tree,weight_map,minimize_edge_t<weight_t>{});
    return param;
}

/**
 *  @brief  Find minimal spanning tree , using Kruskal algorithm.
 *  @param  graph       Weighted graph,that satisfy w_graph_cpt concept.
 *  @param  source      Any node of the graph.
 *  @param  index_map   Index map,which mapped node to unique integer.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @param  out_fnc     Functor, which receive MST-tree edges.
 *  @param  filter      Predicate - edge filter.
 *  @return             Pair {tree weight,number edges in tree }.
 *
 * Complexity: O(E*ln(E)) in worst case.
*/

template<class weight_t,class graph_t,class index_map_t,class weight_map_t,class out_t,class filter_t=always_bool_t<true>>
std::pair<weight_t,std::size_t>
kruskal(graph_t graph,
        typename graph_t::node_handler_t sourse,
        index_map_t index_map,
        weight_map_t weight_map,
        out_t out_fnc,
        filter_t filter=always_bool_t<true>{})
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    using size_t=std::size_t;

    std::pair<weight_t,size_t> res={weight_t{0},0};
    std::vector<std::pair<node_t,edge_t>> edges;
    size_t max_index=0;
    size_t num_nodes=0;

    auto search=full_search_t().set_node_preprocess([&](node_t n)
    {
        max_index=std::max(max_index,index_map(n));
        ++num_nodes;
        return true;
    }).
    set_edge_filter([&](node_t n,edge_t e)
    {
        return filter(n,e);
    }).
    set_edge_process([&](node_t n,edge_t e)
    {
        edges.push_back({n,e});
        return true;
    });

    breadth_search(false,graph,sourse,search,index_map);

    auto comp=[&](const std::pair<node_t,edge_t>&e1,const std::pair<node_t,edge_t>&e2)
    {
        return weight_map(e1.first,e1.second)>weight_map(e2.first,e2.second);
    };

    std::make_heap(edges.begin(),edges.end(),comp);
    weighted_unions_t wun(max_index+1);
    for(res.second++;res.second!=num_nodes;)
    {
        assert(!edges.empty());
        std::pop_heap(edges.begin(),edges.end(),comp);
        auto e=edges.back();
        edges.pop_back();
        if(wun.unite(index_map(e.first),index_map(graph.target(e.first,e.second))))
        {
            out_fnc(e.first,e.second);
            ++res.second;
            res.first+=weight_map(e.first,e.second);
        }
    }
    return res;
}

}// liga

#endif






















