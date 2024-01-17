
#ifndef  _graph_generator_
#define  _graph_generator_

#include <vector>
#include <algorithm>
#include <optional>
#include <utility>
#include <type_traits>
#include "rand_utility.h"

namespace liga{

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
    template<class weight_t>
    w_graph_t<weight_t>
    wdigraph_without_neg_cycles(size_t nodes,size_t edges,std::vector<weight_t>&dists)
    {
        static_assert(std::is_signed_v<weight_t>);
        if(!graph_chars_t{graph_class_t::directed_graph_id,nodes,edges}.is_valid()) return {};

        const weight_t typical_wht=weight_t{10};
        dists.resize(nodes);
        w_graph_t<weight_t> graph(nodes);
        adj_matrix_t in_graph(nodes,std::vector<char>(nodes,0));
        auto rand_int=rand_generator(size_t{0},nodes,m_init_random_engine++);
        auto rand_wght=rand_generator(-typical_wht,typical_wht,m_init_random_engine++);
        dists[0]=weight_t{0};
        for(size_t to=1;to<nodes;++to)
        {
            size_t from=rand_int()%to;
            weight_t weight=rand_wght();
            graph[from].push_back({to,weight});
            in_graph[from][to]=1;
            dists[to]=dists[from]+weight;
        }
        edges-=nodes-1;

        std::vector<std::pair<size_t,size_t>> free_edges;
        for(size_t i=0;i<nodes;++i)
        {
            for(size_t j=0;j<nodes;++j)
            {
                if(i!=j&&!in_graph[i][j]) free_edges.push_back({i,j});
            }
        }
        assert(free_edges.size()>=edges);
        random_queue_t<std::pair<size_t,size_t>> queue(free_edges,m_init_random_engine++);
        auto rand_double_2=rand_generator(weight_t{0},typical_wht,m_init_random_engine++);
        for(;edges;edges--)
        {
            auto [from,to]=queue.pop();
            weight_t weight=dists[to]-dists[from]+rand_double_2();
            graph[from].push_back({to,weight});
        }
        return graph;
    }

    template<class weight_t>
    w_graph_t<weight_t>
    wdigraph_without_neg_cycles(size_t nodes,size_t edges)
    {
        std::vector<weight_t> dists;
        return wdigraph_without_neg_cycles(nodes,edges,dists);
    }
};


}// liga

#endif













