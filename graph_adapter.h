#ifndef _liga_graph_adapter_
#define _liga_graph_adapter_

#include <vector>
#include <type_traits>

namespace liga{

/*
Example:
graph_t graph,graph2;
auto v=filtred_graph_t(joined_graph_t(graph,graph2),filter);
*/

template<class base_graph_t,class filter_t>
class filtred_graph_t
{
    public:
    using node_handler_t=base_graph_t::node_handler_t;
    using edge_handler_t=base_graph_t::edge_handler_t;
    private:
    base_graph_t m_base;
    filter_t     m_filter;
    public:
    filtred_graph_t(base_graph_t v,filter_t f):m_base(v),m_filter(f)
    {
    }
    edge_handler_t edge_begin(node_handler_t n)const
    {
        edge_handler_t e=m_base.edge_begin(n);
        while(!edge_end(n,e)&&!m_filter(n,e))
        {
            m_base.edge_inc(n,e);
        }
        return e;
    }
    void edge_inc(node_handler_t n,edge_handler_t&e)const
    {
        while(!edge_end(n,e)&&!m_filter(n,e))
        {
            m_base.edge_inc(n,e);
        }
    }
    inline bool edge_end(node_handler_t n,edge_handler_t e)const
    {
        return m_base.edge_end(n,e);
    }
    inline node_handler_t target(node_handler_t n,edge_handler_t e)const
    {
        return m_base.target(n,e);
    }
};

template<class fst_graph_t,class snd_graph_t>
class joined_graph_t
{
    public:
    using node_handler_t=fst_graph_t::node_handler_t;
    struct edge_handler_t
    {
        using base_edge_t=fst_graph_t::edge_handler_t;
        base_edge_t fst;
        base_edge_t snd;
        bool is_fst;
        base_edge_t base_edge()const
        {
            return is_fst? fst:snd;
        }
    };

    private:
    fst_graph_t m_fst_graph;
    snd_graph_t m_snd_graph;

    public:
    joined_graph_t(fst_graph_t v1,snd_graph_t v2):m_fst_graph(v1),m_snd_graph(v2)
    {
        static_assert(std::is_same_v<typename fst_graph_t::node_handler_t,
                                     typename snd_graph_t::node_handler_t>);
        static_assert(std::is_same_v<typename fst_graph_t::edge_handler_t,
                                     typename snd_graph_t::edge_handler_t>);
    }
    edge_handler_t edge_begin(node_handler_t n)const
    {
        edge_handler_t fst=m_fst_graph.edge_begin(n);
        edge_handler_t result;
        if(!m_fst_graph.edge_end(n,fst))
        {
            return edge_handler_t{fst,{},true};
        }
        else
        {
            return edge_handler_t{{},m_snd_graph.edge_begin(n),false};
        }
    }
    void edge_inc(node_handler_t n,edge_handler_t&e)const
    {
        if(e.is_fst)
        {
            m_fst_graph.edge_inc(n,e.fst);
            if(m_fst_graph.edge_end(n,e.fst))
            {
                e.is_fst=false;
                e.snd=m_snd_graph.edge_begin(n);
            }
        }
        else
        {
            m_snd_graph.edge_inc(n,e.snd);
        }
    }
    inline bool edge_end(node_handler_t n,edge_handler_t e)const
    {
        return !e.is_fst&&m_snd_graph.edge_end(n,e.snd);
    }
    inline node_handler_t target(node_handler_t n,edge_handler_t e)const
    {
        return e.is_fst? m_fst_graph.target(n,e.fst):m_snd_graph.target(n,e.snd);
    }
};

}// liga

#endif


























