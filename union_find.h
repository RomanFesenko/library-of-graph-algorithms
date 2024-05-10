#ifndef _weighted_unions_t_
#define _weighted_unions_t_

#include <vector>

namespace liga{

class weighted_unions_t
{
    using size_t=std::size_t;
    std::vector<size_t> m_weights;
    std::vector<size_t> m_parents;
    size_t m_top(size_t n)const
    {
        while(m_parents[n]!=n) n=m_parents[n];
        return n;
    }
    public:
    explicit weighted_unions_t(size_t size)
    {
        for(size_t i=0;i<size;++i)
        {
            m_weights.push_back(1);
            m_parents.push_back(i);
        }
    }
    bool is_united(size_t i,size_t j)const{return m_top(i)==m_top(j);}
    bool unite(size_t i,size_t j)
    {
        size_t top_i=m_top(i);
        size_t top_j=m_top(j);
        if(top_i==top_j) return false;
        if(m_weights[top_i]<m_weights[top_j])
        {
            m_parents[top_i]=top_j;
            m_weights[top_j]+=m_weights[top_i];
        }
        else
        {
            m_parents[top_j]=top_i;
            m_weights[top_i]+=m_weights[top_j];
        }
        return true;
    }
};

}// liga

#endif


























