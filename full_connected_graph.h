
#ifndef _liga_full_connected_graph_
#define _liga_full_connected_graph_

#include <vector>

namespace liga{

template<class iterator_t>
class full_connected_graph_t
{
    public:
    using edge_handler_t=iterator_t;
    using node_handler_t=std::decay_t<decltype(*iterator_t{})>;
    private:
    edge_handler_t m_begin;
    edge_handler_t m_end;
    public:
    full_connected_graph_t(iterator_t begin,iterator_t end):m_begin(begin),m_end(end)
    {
    }
    edge_handler_t edge_begin(node_handler_t n)const
    {
        return m_begin;
    }
    void edge_inc(node_handler_t n,edge_handler_t&e)const
    {
        ++e;
    }
    bool edge_end(node_handler_t n,edge_handler_t e)const
    {
        return e==m_end;
    }
    node_handler_t target(node_handler_t n,edge_handler_t e)const
    {
        return *e;
    }
};

}

#endif






















