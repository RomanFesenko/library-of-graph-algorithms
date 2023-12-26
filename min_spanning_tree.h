
#ifndef _min_spanning_tree_
#define _min_spanning_tree_

#include <vector>
#include <optional>
#include <algorithm>
#include <type_traits>

#include "search_settings.h"
#include "priority_search.h"
#include "breadth_search.h"
#include "union_find.h"

namespace __detail_min_tree
{

template<class graph_t,class filter_t/*=always_bool_t<true>*/>
struct mst_search_adapter_t
{
    using node_t=typename graph_traits<graph_t>::node_handler_t;
    using edge_t=typename graph_traits<graph_t>::edge_handler_t;
    using weight_t=typename graph_traits<graph_t>::weight_t;
    using tree_t= tree_search_t<graph_t>;

    const tree_t& m_tree;
    std::pair<weight_t,std::size_t>&m_res;
    filter_t m_filter;

    mst_search_adapter_t(const tree_t&t,std::pair<weight_t,std::size_t>&ref,filter_t f={}):
    m_tree(t),m_res(ref),m_filter(f)
    {
        m_res.first=weight_t{0};
        m_res.second=0;
    }

    constexpr bool node_preprocess(node_t n)const
    {
        assert(m_tree.opt_dist(n));
        ++m_res.second;
        m_res.first+=m_tree.dist(n);
        return true;
    }
    constexpr bool edge_filter(node_t n,edge_t e)const{return m_filter(n,e);}
    constexpr bool edge_process(node_t node,edge_t edge)const{return true;}
};

}

template<class graph_t,class filter_t=always_bool_t<true>>
std::pair<typename graph_traits<graph_t>::weight_t,std::size_t>
dense_prim(graph_t&g,
           typename graph_traits<graph_t>::node_handler_t sourse,
           tree_search_t<graph_t>&tree,
           filter_t filter=always_bool_t<true>{})
{

    using namespace __detail_min_tree;
    std::pair<typename graph_traits<graph_t>::weight_t,std::size_t> res={};

    mst_search_adapter_t<graph_t,filter_t> search(tree,res,filter);
    dense_priority_search(g,sourse,search,tree,minimize_edge_t<graph_t>{});
    return res;
}

template<class graph_t,class filter_t=always_bool_t<true>>
std::pair<typename graph_traits<graph_t>::weight_t,std::size_t>
sparse_prim(graph_t&g,
           typename graph_traits<graph_t>::node_handler_t sourse,
           tree_search_t<graph_t>&tree,
           filter_t filter=always_bool_t<true>{})
{

    using namespace __detail_min_tree;
    std::pair<typename graph_traits<graph_t>::weight_t,std::size_t> res;

    mst_search_adapter_t<graph_t,filter_t> search(tree,res,filter);
    sparse_priority_search(g,sourse,search,tree,minimize_edge_t<graph_t>{});
    return res;
}


template<class graph_t,class out_t,class filter_t=always_bool_t<true>>
std::pair<typename graph_traits<graph_t>::weight_t,std::size_t>
kruskal(graph_t&g,
        typename graph_traits<graph_t>::node_handler_t sourse,
        out_t out_fnc,
        filter_t filter=always_bool_t<true>{})
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using weight_t=traits::weight_t;
    using size_t=std::size_t;

    std::pair<weight_t,size_t> res={weight_t{0},0};
    std::vector<std::pair<node_t,edge_t>> edges;
    size_t max_index=0;
    size_t num_nodes=0;

    auto search=full_search_t<graph_t>().set_node_preprocess([&](node_t n)
    {
        max_index=std::max(max_index,traits::index(g,n));
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

    breadth_search(false,g,sourse,search);

    auto comp=[&](auto&e1,auto&e2)
    {
        return traits::weight(g,e1.first,e1.second)>traits::weight(g,e2.first,e2.second);
    };

    std::make_heap(edges.begin(),edges.end(),comp);
    weighted_unions_t wun(max_index+1);
    for(res.second++;res.second!=num_nodes;)
    {
        assert(!edges.empty());
        std::pop_heap(edges.begin(),edges.end(),comp);
        auto e=edges.back();
        edges.pop_back();
        if(wun.unite(traits::index(g,e.first),traits::index(g,traits::target(g,e.first,e.second))))
        {
            out_fnc(e.first,e.second);
            ++res.second;
            res.first+=traits::weight(g,e.first,e.second);
        }
    }
    return res;
}


#endif






















