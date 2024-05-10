
#ifndef  _test_suite_
#define  _test_suite_

#include <vector>
#include <optional>
#include <utility>
#include <tuple>

class graph_test_suite_t
{
    using size_t=std::size_t;
    using edge_list_t=std::vector<std::tuple<size_t,size_t,size_t>>;

    size_t                   m_size;
    size_t                   m_total_wef;
    std::vector<size_t>      m_delta;
    std::vector<edge_list_t> m_graphs;
    std::size_t m_block_size(size_t)const;
    std::size_t m_block_width(size_t)const;

    public:
    graph_test_suite_t();
    size_t size()const;
    const edge_list_t& edge_list(size_t g)const;

    size_t nodes(size_t g);
    size_t edges(size_t g);

    std::optional<size_t> weighted_path_dir(size_t g,size_t from,size_t to)const;
    size_t weighted_path_undir(size_t g,size_t from,size_t to)const;

    std::optional<size_t> path_dir(size_t g,size_t from,size_t to)const;
    size_t path_undir(size_t g,size_t from,size_t to)const;

    size_t max_flow_dir(size_t g,size_t from,size_t to)const;
    size_t min_cost_dir(size_t g,size_t from,size_t to)const;

    size_t max_flow_undir(size_t g,size_t from,size_t to)const;
    size_t min_cost_undir(size_t g,size_t from,size_t to)const;

    size_t mst_weight(size_t g)const;
    size_t components(size_t g)const;
    bool is_tree(size_t g)const;
    bool is_dag(size_t g)const;
    bool is_bipartite(size_t g)const;

    bool has_undirected_euler_tour(size_t g)const;
    bool has_undirected_euler_cycle(size_t g)const;

    bool has_directed_euler_tour(size_t g)const;
    bool has_directed_euler_cycle(size_t g)const;

    size_t nart_points(size_t g)const;
    size_t nbridges(size_t g)const;
    size_t num_scc(size_t g)const;

    size_t max_cardinality_matching(size_t g)const;
    size_t max_weight_matching(size_t g)const;
    std::optional<size_t> max_perf_weight_matching(size_t g)const;
    size_t min_weight_covering(size_t g)const;

    size_t num_all_pairs_tests()const;
};

std::vector<std::vector<std::size_t>>
to_adjacency_list(bool is_direct,const std::vector<std::tuple<std::size_t,std::size_t,std::size_t>>&edges);

std::vector<std::vector<std::pair<std::size_t,std::size_t>>>
to_indexed_adjacency_list(bool is_direct,const std::vector<std::tuple<std::size_t,std::size_t,std::size_t>>&edges);

template<class weight_t=std::size_t>
std::vector<std::vector<std::pair<std::size_t,weight_t>>>
to_weighted_adjacency_list(bool is_direct,const std::vector<std::tuple<std::size_t,std::size_t,std::size_t>>&edges)
{
    std::vector<std::vector<std::pair<std::size_t,weight_t>>> graph;
    for(auto [source,target,weight]:edges)
    {
        if(std::size_t num_nodes=std::max({source+1,target+1,graph.size()});num_nodes>graph.size())
        {
            graph.resize(num_nodes,{});
        }
        graph[source].push_back({target,weight});
        if(!is_direct) graph[target].push_back({source,weight});
    }
    return graph;
}

#endif













