
#ifndef  _graph_generator_
#define  _graph_generator_

#include <vector>
#include <algorithm>
#include <optional>
#include <utility>
#include "rand_utility.h"

enum class graph_class_t:char
{
    undirected_graph_id=0,
    directed_graph_id=1,
    undirected_tree_id=2,
    directed_tree_id=3,
    bipartite_id=4,
    dag_id=5,
    strongly_connected_id=9
};

inline bool is_direct(graph_class_t gc)
{
    return static_cast<char>(gc)&1;
}

inline bool is_tree(graph_class_t gc)
{
    return static_cast<char>(gc)&2;
}

inline bool is_acyclic(graph_class_t gc)
{
    return is_tree(gc)||gc==graph_class_t::dag_id;
}

std::pair<std::size_t,std::size_t> edge_range(std::size_t nodes,graph_class_t);


class graph_generator_t
{
    public:
    using graph_t=std::vector<std::vector<size_t>>;// adjacency list
    private:
    using size_t=std::size_t;
    using adj_matrix_t=std::vector<std::vector<char>>;

    int m_init_random_engine=0;

    void m_make_undirected_tree(graph_t&,size_t nodes);
    void m_make_undirected_tree(graph_t&,size_t nodes,adj_matrix_t&);

    void m_make_directed_tree(graph_t&,size_t nodes);
    void m_make_directed_tree(graph_t&,size_t nodes,adj_matrix_t&);

    void m_make_undirected_graph(graph_t&,size_t nodes,size_t edges);
    void m_make_directed_graph(graph_t&,size_t nodes,size_t edges);

    void m_make_dag(graph_t&,size_t nodes,size_t edges);

    void m_make_bipartite(graph_t&,size_t nodes,size_t edges);


    public:
    template<class weight_t>
    using w_graph_t=std::vector<std::vector<std::pair<size_t,weight_t>>>;// adjacency list
    struct graph_chars_t
    {
        graph_class_t g_class;
        size_t num_nodes;
        size_t num_edges;
        bool is_valid()const;
    };
    graph_generator_t(){}
    bool make_graph(graph_t&,const graph_chars_t&);
    template<class weight_t>
    bool make_wgraph(w_graph_t<weight_t>&w_graph,
                     const graph_chars_t&chars_,
                     const std::pair<weight_t,weight_t>&range)
    {
        assert(range.second>=range.first);
        graph_t graph;
        if(!make_graph(graph,chars_)) return false;
        auto gen=rand_generator(range.first,range.second,m_init_random_engine++);
        w_graph.clear();
        w_graph.resize(graph.size(),{});
        if(is_direct(chars_.g_class))
        {
            for(size_t from=0;from<graph.size();++from)
            {
                for(size_t i=0;i<graph[from].size();++i)
                {
                    w_graph[from].push_back({graph[from][i],gen()});
                }
            }
        }
        else
        {
            for(size_t from=0;from<graph.size();++from)
            {
                for(size_t i=0;i<graph[from].size();++i)
                {
                    size_t to=graph[from][i];
                    assert(to!=from);
                    if(to>from) continue;
                    weight_t weight=gen();
                    w_graph[from].push_back({to,weight});
                    w_graph[to].push_back({from,weight});
                }
            }
        }
        return true;
    }
};









#endif













