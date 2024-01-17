/** @file matching.h
 *  Matching algorithms declarations.
*/
#ifndef  _matching_
#define  _matching_

#include <vector>
#include <algorithm>
#include <optional>
#include <utility>

namespace liga{

namespace matching{

int matching_weight(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                    const std::vector<std::size_t>&match);

int covering_weight(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                    const std::vector<std::size_t>&cov);

bool is_valid_matching(const std::vector<std::vector<std::size_t>>&graph,
                       const std::vector<std::size_t>&match);

bool is_valid_matching(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                       const std::vector<std::size_t>&match);

bool is_valid_covering(const std::vector<std::vector<std::size_t>>&graph,
                       const std::vector<std::vector<std::size_t>>&covering);

bool is_valid_covering(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                       const std::vector<std::vector<std::size_t>>&covering);

class bipartite_solver_t
{
    public:
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::size_t>>;// adjacency list
    using w_graph_t=std::vector<std::vector<std::pair<std::size_t,int>>>;// adjacency list
    private:
    static void m_augment(size_t root,size_t tail,std::vector<size_t>&,std::vector<size_t>&);

    template<bool is_perfect>
    std::optional<int> m_max_weighted(const w_graph_t&,std::vector<size_t>&match);

    public:
    size_t max_cardinality(const graph_t&);
    size_t max_cardinality(const graph_t&,std::vector<size_t>&match);

    int max_weighted(const w_graph_t&);
    int max_weighted(const w_graph_t&,std::vector<size_t>&match);

    std::optional<int> perfect_max_weighted(const w_graph_t&);
    std::optional<int> perfect_max_weighted(const w_graph_t&,std::vector<size_t>&match);

    int min_covering(const w_graph_t&);
    int min_covering(const w_graph_t&,graph_t&covering);
};

class nonbipartite_solver_t
{
    public:
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::size_t>>;// adjacency list
    using w_graph_t=std::vector<std::vector<std::pair<std::size_t,int>>>;// adjacency list
    private:
    template<bool is_perfect>
    std::optional<int> m_max_weighted(const w_graph_t&,std::vector<size_t>&match);
    public:
    size_t max_cardinality(const graph_t&);
    size_t max_cardinality(const graph_t&,std::vector<size_t>&match);

    int max_weighted(const w_graph_t&);
    int max_weighted(const w_graph_t&,std::vector<size_t>&match);

    std::optional<int> perfect_max_weighted(const w_graph_t&);
    std::optional<int> perfect_max_weighted(const w_graph_t&,std::vector<size_t>&match);

    //int min_covering(const w_graph_t&);
    //int min_covering(const w_graph_t&,graph_t&covering);
};

class backtracking_solver_t
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::size_t>>;// adjacency list
    using w_graph_t=std::vector<std::vector<std::pair<std::size_t,int>>>;// adjacency list
    private:
    std::optional<int> m_max_weighted(const w_graph_t&,std::vector<size_t>&match,size_t min_match);
    public:
    size_t max_cardinality(const graph_t&);
    size_t max_cardinality(const graph_t&,std::vector<size_t>&match);

    int max_weighted(const w_graph_t&);
    int max_weighted(const w_graph_t&,std::vector<size_t>&match);

    std::optional<int> perfect_max_weighted(const w_graph_t&);
    std::optional<int> perfect_max_weighted(const w_graph_t&,std::vector<size_t>&match);

    int min_covering(const w_graph_t&);
    int min_covering(const w_graph_t&,graph_t&covering);
};

}//matching

}// liga

#endif













