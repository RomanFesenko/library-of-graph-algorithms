
#ifndef _search_settings_
#define _search_settings_

#include <vector>
#include <optional>
#include <algorithm>

#include "graph_traits.h"


enum class search_termination_t:char
{
    node_preprocess_id=0,
    edge_process_id,
    node_postprocess_id,
    complete_id
};

template<bool bool_>
struct always_bool_t
{
    template<class...types>
    constexpr bool operator()(types...)const{return bool_;}
};

template<class graph_t,
         class prefunc_t,
         class filter_t,
         class efunc_t,
         class postfunc_t>
class custom_search_t
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    prefunc_t  m_prefunc;
    filter_t   m_filter;
    efunc_t    m_efunc;
    postfunc_t m_postfunc;
    public:
    constexpr custom_search_t(prefunc_t pref,filter_t filt,efunc_t efunc,postfunc_t postfunc):
    m_prefunc(pref),
    m_filter(filt),
    m_efunc(efunc),
    m_postfunc(postfunc)
    {
    }
    constexpr bool node_preprocess(node_t n){return m_prefunc(n);}
    constexpr bool edge_filter(node_t n,edge_t e){return m_filter(n,e);}
    constexpr bool edge_process(node_t n,edge_t e){return m_efunc(n,e);}
    constexpr bool node_postprocess(node_t n){return m_postfunc(n);}

    template<class other_prefunc_t>
    constexpr auto set_node_preprocess(other_prefunc_t f) const
    {
        return custom_search_t<graph_t,
                               other_prefunc_t,
                               filter_t,
                               efunc_t,
                               postfunc_t>(f,m_filter,m_efunc,m_postfunc);
    }

    template<class other_filter_t>
    constexpr auto set_edge_filter(other_filter_t f) const
    {
        return custom_search_t<graph_t,
                               prefunc_t,
                               other_filter_t,
                               efunc_t,
                               postfunc_t>(m_prefunc,f,m_efunc,m_postfunc);
    }

    template<class other_efunc_t>
    constexpr auto set_edge_process(other_efunc_t f) const
    {
        return custom_search_t<graph_t,
                               prefunc_t,
                               filter_t,
                               other_efunc_t,
                               postfunc_t>(m_prefunc,m_filter,f,m_postfunc);
    }

    template<class other_postfunc_t>
    constexpr auto set_node_postprocess(other_postfunc_t f) const
    {
        return custom_search_t<graph_t,
                               prefunc_t,
                               filter_t,
                               efunc_t,
                               other_postfunc_t>(m_prefunc,m_filter,m_efunc,f);
    }
};

namespace __detail_settings
{

template<class graph_t>
struct search_node_fnctr_t
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    graph_t&     m_graph;
    std::size_t  m_target;
    constexpr search_node_fnctr_t(graph_t&graph,node_t t):
    m_graph(graph),m_target(traits::index(graph,t))
    {
    }
    constexpr bool operator()(node_t node)const
    {
        return traits::index(m_graph,node)!=m_target;
    }
};


}//__detail_settings

template<class graph_t>
class search_node_t:public custom_search_t<graph_t,
                                            __detail_settings::search_node_fnctr_t<graph_t>,
                                            always_bool_t<true>,
                                            always_bool_t<true>,
                                            always_bool_t<true>>
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using base_t=custom_search_t<graph_t,
                                 __detail_settings::search_node_fnctr_t<graph_t>,
                                 always_bool_t<true>,
                                 always_bool_t<true>,
                                 always_bool_t<true>>;
    public:
    constexpr search_node_t(graph_t&graph,node_t t):
    base_t(__detail_settings::search_node_fnctr_t<graph_t>(graph,t),{},{},{})
    {
    }
};


template<class graph_t>
class full_search_t:public custom_search_t<graph_t,
                                            always_bool_t<true>,
                                            always_bool_t<true>,
                                            always_bool_t<true>,
                                            always_bool_t<true>>
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using base_t=custom_search_t<graph_t,
                                 always_bool_t<true>,
                                 always_bool_t<true>,
                                 always_bool_t<true>,
                                 always_bool_t<true>>;
    public:
    constexpr full_search_t():base_t({},{},{},{})
    {
    }
};

template<class graph_t>
struct minimize_distance_t
{
    using traits=graph_traits<graph_t>;
    using weight_t=traits::weight_t;
    bool priority(weight_t w1,weight_t w2 )const {return w1<w2;}
    weight_t weight_update(weight_t wnode,weight_t wedge)const {return wnode+wedge;}
    weight_t init_weight()const{return weight_t{0};}
};

template<class graph_t>
struct minimize_edge_t
{
    using traits=graph_traits<graph_t>;
    using weight_t=traits::weight_t;
    bool priority(weight_t w1,weight_t w2 )const {return w1<w2;}
    weight_t weight_update(weight_t wnode,weight_t wedge)const {return wedge;}
    weight_t init_weight()const{return weight_t{0};}
};

template<class graph_t>
struct maximize_flow_t
{
    using traits=graph_traits<graph_t>;
    using weight_t=traits::weight_t;
    weight_t m_max_flow;
    bool priority(weight_t new_flow,weight_t old_flow )const {return new_flow>old_flow;}
    weight_t weight_update(weight_t preflow,weight_t edge_capacity)const
    {
        return std::min(preflow,edge_capacity);
    }
    weight_t init_weight()const{return m_max_flow;}
};


#endif






















