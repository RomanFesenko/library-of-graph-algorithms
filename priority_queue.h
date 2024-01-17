#ifndef _graph_priority_queue_
#define _graph_priority_queue_

#include <vector>
#include <algorithm>
#include <assert.h>

namespace liga{

template<class element_t,class comp_t>
class array_priority_queue_t
{
    public:
    std::vector<element_t> m_elems;
    comp_t                 m_comp;

    public:
    array_priority_queue_t(comp_t c={}):m_comp(c)
    {
    }
    bool empty()const{return m_elems.empty();}
    void clear(){m_elems.clear();}
    element_t pop()
    {
        auto iter=std::min_element(m_elems.begin(),m_elems.end(),m_comp);
        element_t res=*iter;
        std::iter_swap(iter,m_elems.end()-1);
        m_elems.pop_back();
        return res;
    }
    void push(element_t n)
    {
        m_elems.push_back(n);
    }
    void rebuild_after_mod(element_t n)
    {

    }
};

template<class element_t,class comp_t,class indexer_t>
class heap_priority_queue_t
{
    using size_t=std::size_t;
    public:

    // m_indxs[m_indexer(m_elems[i])]==i for all i

    std::vector<element_t> m_elems;
    comp_t                 m_comp;
    indexer_t              m_indexer;
    std::vector<int>       m_indxs;

    int&m_indx_in_heap(element_t node)
    {
        int&res=m_indxs[m_indexer(node)];
        return res;
    }
    void m_swap(size_t i,size_t j)
    {
        std::swap(m_elems[i],m_elems[j]);
        m_indxs[m_indexer(m_elems[i])]=i;
        m_indxs[m_indexer(m_elems[j])]=j;
    }
    void m_down_up(size_t indx)
    {
        while(indx!=0)
        {
            size_t parent=(indx-1)/2;
            if(m_comp(m_elems[indx],m_elems[parent]))
            {
                m_swap(indx,parent);
                indx=parent;
            }
            else break;
        }
    }

    public:
    heap_priority_queue_t(comp_t c={},indexer_t i={}):m_comp(c),m_indexer(i)
    {
    }

    bool empty()const{return m_elems.empty();}
    void clear()
    {
        m_elems.clear();
        m_indxs.clear();
    }
    element_t pop()
    {
        element_t res=m_elems[0];
        m_elems[0]=m_elems.back();
        m_indxs[m_indexer(m_elems[0])]=0;
        m_elems.pop_back();
        size_t top=0;
        for(size_t left=top*2+1;left<m_elems.size();left=top*2+1)
        {
            size_t right=left+1;
            if(right<m_elems.size()&&m_comp(m_elems[right],m_elems[left]))
            {
                if(m_comp(m_elems[right],m_elems[top]))
                {
                    m_swap(top,right);
                    top=right;
                }
                else break;
            }
            else
            {
                if(m_comp(m_elems[left],m_elems[top]))
                {
                    m_swap(top,left);
                    top=left;
                }
                else break;
            }
        }
        return res;
    }
    void push(element_t n)
    {
        if(size_t i=m_indexer(n);i>=m_indxs.size()) m_indxs.resize(i+1);
        m_elems.push_back(n);
        size_t down=m_indx_in_heap(n)=m_elems.size()-1;
        m_down_up(down);
    }
    void rebuild_after_mod(element_t n)
    {
        m_down_up(m_indx_in_heap(n));
    }

};

}// liga

#endif


























