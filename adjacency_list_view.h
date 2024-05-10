#ifndef _liga_adjacency_list_view
#define _liga_adjacency_list_view

#include <vector>
#include <type_traits>
#include "search_settings.h"

namespace liga{

template<class data_t,class target_t,class node_t,class index_map_t=default_index_map_t>
class adjacency_list_view_t
{
    const data_t* m_data=nullptr;
    target_t      m_target;
    index_map_t   m_index_map;
    public:
    using edge_handler_t=decltype(std::begin((*m_data)[0]));
    using node_handler_t=node_t;
    constexpr adjacency_list_view_t(){}
    constexpr adjacency_list_view_t(const data_t&data,
                                    target_t target=target_t{},
                                    index_map_t imap=index_map_t{}):
    m_data(&data),
    m_target(target),
    m_index_map(imap)
    {
    }
    const data_t* data()const{return m_data;}
    void set_data(const data_t&data){m_data=&data;}

    inline edge_handler_t edge_begin(node_handler_t n)const
    {
        return std::begin((*m_data)[m_index_map(n)]);
    }
    inline void edge_inc(node_handler_t n,edge_handler_t&e)const
    {
        ++e;
    }
    inline bool edge_end(node_handler_t n,edge_handler_t e)const
    {
        return e==std::end((*m_data)[m_index_map(n)]);
    }
    inline node_handler_t target(node_handler_t n,edge_handler_t e)const
    {
        return m_target(e);
    }
};

}// liga

#endif


























