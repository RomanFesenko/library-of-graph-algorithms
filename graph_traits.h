
#ifndef _graph_traits_
#define _graph_traits_

#include <type_traits>


#include "graph_concepts.h"

template<class graph_t>
struct graph_traits
{
    static constexpr bool is_weighted=graph_t::is_weighted;
    using node_handler_t=graph_t::node_handler_t;
    using edge_handler_t=graph_t::edge_handler_t;
    using weight_t=std::conditional_t<is_weighted,typename graph_t::weight_t,std::size_t>;
    static edge_handler_t edge_begin(const graph_t&g,node_handler_t n)
    {
        return g.edge_begin(n);
    }
    static void edge_inc(const graph_t&g,node_handler_t n,edge_handler_t&e)
    {
        g.edge_inc(n,e);
    }
    static bool edge_end(const graph_t&g,node_handler_t n,edge_handler_t e)
    {
        return g.edge_end(n,e);
    }
    static node_handler_t target(const graph_t&g,node_handler_t n,edge_handler_t e)
    {
        return g.target(n,e);
    }
    static std::size_t index(const graph_t&g,node_handler_t n)
    {
        return g.index(n);
    }
    static weight_t weight(const graph_t&g,node_handler_t n,edge_handler_t e)
    requires is_weighted
    {
        return g.weight(n,e);
    }
};

template<vector_list_cpt vlist_t>
struct graph_traits<vlist_t>
{
    using const_vlist_t=const vlist_t;
    static constexpr bool is_weighted=false;
    using node_handler_t=std::size_t;
    using edge_handler_t=decltype(const_vlist_t{}[0].begin());
    using weight_t=std::size_t;
    static edge_handler_t edge_begin(const vlist_t&g,node_handler_t n)
    {
        return g[n].begin();
    }
    static void edge_inc(const vlist_t&g,node_handler_t n,edge_handler_t&e)
    {
        ++e;
    }
    static bool edge_end(const vlist_t&g,node_handler_t n,edge_handler_t e)
    {
        return e==g[n].end();
    }
    static node_handler_t target(const vlist_t&g,node_handler_t n,edge_handler_t e)
    {
        return *e;
    }
    static std::size_t index(const vlist_t&g,node_handler_t n)
    {
        return n;
    }
};


template<w_vector_list_cpt vlist_t>
struct graph_traits<vlist_t>
{
    static constexpr bool is_weighted=true;
    using const_vlist_t=const vlist_t;
    using node_handler_t=std::size_t;
    using edge_handler_t=decltype(const_vlist_t{}[0].begin());
    using weight_t=std::decay_t<decltype(std::get<1>(*edge_handler_t{}))> ;
    static edge_handler_t edge_begin(const vlist_t&g,node_handler_t n)
    {
        return g[n].begin();
    }
    static void edge_inc(const vlist_t&g,node_handler_t n,edge_handler_t&e)
    {
        ++e;
    }
    static bool edge_end(const vlist_t&g,node_handler_t n,edge_handler_t e)
    {
        return e==g[n].end();
    }
    static node_handler_t target(const vlist_t&g,node_handler_t n,edge_handler_t e)
    {
        return std::get<0>(*e);
    }
    static weight_t weight(const vlist_t&g,node_handler_t n,edge_handler_t e)
    {
        return std::get<1>(*e);
    }
    static std::size_t index(const vlist_t&g,node_handler_t n)
    {
        return n;
    }
};


#endif






















