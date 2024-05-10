
#ifndef _search_settings_
#define _search_settings_

#include <vector>
#include <optional>
#include <algorithm>
#include <functional>


namespace liga{


struct default_index_map_t
{
    template<class value_t>
    inline std::size_t operator()(value_t val)const
    {
        if constexpr(std::is_integral_v<value_t>)
        {
            return val;
        }
        else if constexpr(std::is_pointer_v<value_t>)
        {
            return val->index();
        }
        else
        {
            return val.index();
        }
    }
};

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


template<class prefunc_t,
         class filter_t,
         class efunc_t,
         class postfunc_t>
class custom_search_t
{
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
    template<class node_t>
    constexpr bool node_preprocess(node_t n){return m_prefunc(n);}

    template<class node_t,class edge_t>
    constexpr bool edge_filter(node_t n,edge_t e){return m_filter(n,e);}

    template<class node_t,class edge_t>
    constexpr bool edge_process(node_t n,edge_t e){return m_efunc(n,e);}

    template<class node_t>
    constexpr bool node_postprocess(node_t n){return m_postfunc(n);}

    template<class other_prefunc_t>
    constexpr auto set_node_preprocess(other_prefunc_t f) const
    {
        return custom_search_t<other_prefunc_t,
                               filter_t,
                               efunc_t,
                               postfunc_t>(f,m_filter,m_efunc,m_postfunc);
    }

    template<class other_filter_t>
    constexpr auto set_edge_filter(other_filter_t f) const
    {
        return custom_search_t<prefunc_t,
                               other_filter_t,
                               efunc_t,
                               postfunc_t>(m_prefunc,f,m_efunc,m_postfunc);

    }

    template<class other_efunc_t>
    constexpr auto set_edge_process(other_efunc_t f) const
    {
        return custom_search_t<prefunc_t,
                               filter_t,
                               other_efunc_t,
                               postfunc_t>(m_prefunc,m_filter,f,m_postfunc);

    }

    template<class other_postfunc_t>
    constexpr auto set_node_postprocess(other_postfunc_t f) const
    {
        return custom_search_t<prefunc_t,
                               filter_t,
                               efunc_t,
                               other_postfunc_t>(m_prefunc,m_filter,m_efunc,f);

    }
};

namespace __detail_settings
{

template<class node_t,class comp_t=std::equal_to<>>
struct search_node_fnctr_t
{
    node_t  m_target;
    comp_t  m_comp;
    constexpr search_node_fnctr_t(node_t target,comp_t comp):
    m_target(target),m_comp(comp)
    {
    }
    constexpr bool operator()(node_t node)const
    {
        return !m_comp(node,m_target);
    }
};


}//__detail_settings

template<class node_t,class comp_t=std::equal_to<>>
class search_node_t:public custom_search_t<__detail_settings::search_node_fnctr_t<node_t,comp_t>,
                                           always_bool_t<true>,
                                           always_bool_t<true>,
                                           always_bool_t<true>>
{
    using base_t=custom_search_t<__detail_settings::search_node_fnctr_t<node_t,comp_t>,
                                 always_bool_t<true>,
                                 always_bool_t<true>,
                                 always_bool_t<true>>;
    public:
    constexpr search_node_t(node_t target,comp_t comp=comp_t{}):
    base_t(__detail_settings::search_node_fnctr_t<node_t,comp_t>(target,comp),{},{},{})
    {
    }
};


class full_search_t:public custom_search_t<always_bool_t<true>,
                                           always_bool_t<true>,
                                           always_bool_t<true>,
                                           always_bool_t<true>>

{
    using base_t=custom_search_t<always_bool_t<true>,
                                 always_bool_t<true>,
                                 always_bool_t<true>,
                                 always_bool_t<true>>;
    public:
    constexpr full_search_t():base_t({},{},{},{})
    {
    }
};

template<class _weight_t,class comp_t=std::less<_weight_t>,class upd_t=std::plus<_weight_t>>
class minimize_distance_t
{
    using weight_t=_weight_t;

    weight_t m_init;
    comp_t   m_comp;
    upd_t    m_upd;
    public:
    constexpr minimize_distance_t(weight_t i=weight_t{0},comp_t c=comp_t{},upd_t u=upd_t{}):
    m_init(i),m_comp(c),m_upd(u)
    {
    }
    constexpr bool priority(weight_t w1,weight_t w2 )const
    {
        return m_comp(w1,w2);
    }
    constexpr weight_t weight_update(weight_t wnode,weight_t wedge)const
    {
        return m_upd(wnode,wedge);
    }
    constexpr weight_t init_weight()const
    {
        return m_init;
    }
    void set_init_weight(weight_t w)
    {
        m_init=w;
    }
};

template<class _weight_t>
struct minimize_edge_t
{
    using weight_t=_weight_t;
    bool priority(weight_t w1,weight_t w2 )const {return w1<w2;}
    weight_t weight_update(weight_t wnode,weight_t wedge)const {return wedge;}
    weight_t init_weight()const{return weight_t{0};}
};

template<class _weight_t>
struct maximize_flow_t
{
    using weight_t=_weight_t;
    weight_t max_flow;
    bool priority(weight_t new_flow,weight_t old_flow )const {return new_flow>old_flow;}
    weight_t weight_update(weight_t preflow,weight_t edge_capacity)const
    {
        return std::min(preflow,edge_capacity);
    }
    weight_t init_weight()const{return max_flow;}
};

}// liga

#endif






















