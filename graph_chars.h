
#ifndef _liga_graph_chars_
#define _liga_graph_chars_


namespace liga{

struct graph_chars_t
{
    static const char directed=          1<<0;
    static const char undirected=        1<<1;
    static const char has_multiedges=    1<<2;
    static const char has_loops=         1<<3;
    static const char acyclic=           1<<4;

    static const char dag=  directed|acyclic;
    static const char mixed=directed|undirected;
    static const char tree= undirected|acyclic;

    static bool is_mixed(char type)
    {
        return (type&graph_chars_t::mixed)==graph_chars_t::mixed;
    }
    template<char type>
    static constexpr bool is_mixed()
    {
        return (type&graph_chars_t::mixed)==graph_chars_t::mixed;
    }

    static bool is_strongly_directed(char type)
    {
        return !(type&graph_chars_t::undirected);
    }
    template<char type>
    static constexpr bool is_strongly_directed()
    {
        return !(type&graph_chars_t::undirected);
    }

    static bool is_strongly_undirected(char type)
    {
        return !(type&graph_chars_t::directed);
    }
    template<char type>
    static constexpr bool is_strongly_undirected()
    {
        return !(type&graph_chars_t::directed);
    }
};

}// liga

#endif






















