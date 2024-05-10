
#ifndef _liga_property_map_
#define _liga_property_map_


namespace liga{

template<class key_t,class value_t,class index_map_t>
class property_map_t
{
    using size_t=std::size_t;

    index_map_t          m_index_map;
    value_t              m_default;
    std::vector<value_t> m_value;
    public:
    property_map_t(value_t val={},index_map_t imap={}):m_index_map(imap),m_default(val)
    {
    }
    const value_t& default_value()const{return m_default;}
    value_t& default_value(){return m_default;}
    const index_map_t&index_map()const{return m_index_map;}
    index_map_t&index_map(){return m_index_map;}
    bool is_equal_key(const key_t& key1,const key_t& key2)const
    {
        return m_index_map(key1)==m_index_map(key2);
    }

    const value_t& cref(const key_t&key) const
    {
        size_t index=m_index_map(key);
        return index>=m_value.size()? m_default:m_value[index];
    }
    const value_t&operator()(const key_t&key) const
    {
        return cref(key);
    }
    value_t& ref(const key_t& key)
    {
        size_t index=m_index_map(key);
        if(index>=m_value.size())
        {
            m_value.resize(index+1,m_default);
        }
        return m_value[index];
    }

    void resize(size_t size)
    {
        if(size>m_value.size())
        {
            m_value.resize(size,m_default);
        }
    }
    void clear()
    {
        m_value.clear();
    }
};

}// liga

#endif






















