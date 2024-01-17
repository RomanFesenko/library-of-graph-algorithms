
#ifndef _node_property_
#define _node_property_


#include "graph_traits.h"

namespace liga{

template<class graph_t,class value_t>
class node_property_t
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using size_t=std::size_t;
    const graph_t*       m_graph=nullptr;
    value_t              m_default;
    std::vector<value_t> m_value;
    public:
    node_property_t(const graph_t& graph,value_t val)
    {
        m_graph=&graph;
        m_default=val;
    }
    const value_t& default_val()const{return m_default;}
    const graph_t* graph()const{return m_graph;}
    void init(const graph_t& graph,value_t val)
    {
        m_graph=&graph;
        m_value.clear();
        m_default=val;
    }
    const value_t&operator()(node_t node)const
    {
        if(size_t indx=traits::index(*m_graph,node);indx<m_value.size())
        {
            return m_value[indx];
        }
        else
        {
            return m_default;
        }
    }
    value_t& ref(node_t node)
    {
        size_t indx=traits::index(*m_graph,node);
        if(indx>=m_value.size())
        {
            m_value.resize(indx+1,m_default);
        }
        return m_value[indx];
    }

    void operator()(node_t node,const value_t&val)
    {
        size_t indx=traits::index(*m_graph,node);
        if(indx>=m_value.size())
        {
            m_value.resize(indx+1,m_default);
        }
        m_value[indx]=val;
    }
    const value_t&value_by_index(size_t indx) const
    {
        return indx>=m_value.size()? m_default:m_value[indx];

    }
    value_t& ref_by_index(size_t indx)
    {
        if(indx>=m_value.size())
        {
            m_value.resize(indx+1,m_default);
        }
        return m_value[indx];
    }

    void resize(size_t size)
    {
        if(size>m_value.size())
        {
            m_value.resize(size,m_default);
        }
    }
};

}// liga

#endif






















