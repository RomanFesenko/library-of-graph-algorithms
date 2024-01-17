
#ifndef _rand_utility_
#define _rand_utility_

#include <utility>
#include <vector>
#include <random>


template<class value_t>
class random_queue_t
{
    using size_t=std::size_t;
    std::default_random_engine dre;
    std::uniform_int_distribution<size_t> distr;
    std::vector<value_t>&m_val;
    public:
    random_queue_t(std::vector<value_t>&v,size_t init):
    dre(init),distr(0,v.size()),m_val(v)
    {
    }
    size_t size()const{return m_val.size();}
    bool   empty()const{return m_val.empty();}
    value_t pop()
    {
        size_t indx=distr(dre)%m_val.size();
        value_t ret=m_val[indx];
        std::swap(m_val[indx],m_val.back());
        m_val.pop_back();
        return ret;
    }
};

template<class val_t>
auto rand_generator(val_t low,val_t up,std::size_t init)
{
    if constexpr(std::is_integral_v<val_t>)
    {
        std::default_random_engine dre(init);
        std::uniform_int_distribution<val_t> distr(low,up);
        return [dre,distr]()mutable
        {
            return distr(dre);
        };
    }
    else
    {
        std::default_random_engine dre(init);
        std::uniform_real_distribution<val_t> distr(low,up);
        return [dre,distr]()mutable
        {
            return distr(dre);
        };
    }
}

#endif






















