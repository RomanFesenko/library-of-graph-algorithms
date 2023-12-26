
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
    size_t max_flow_undir(size_t g,size_t from,size_t to)const;

    size_t mst_weight(size_t g)const;
    size_t components(size_t g)const;
    bool is_tree(size_t g)const;
    bool is_dag(size_t g)const;
    bool is_bipartite(size_t g)const;

};

#endif













