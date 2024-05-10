
#ifndef _graph_concepts_
#define _graph_concepts_

#include <concepts>
#include <utility>
#include <type_traits>

namespace liga{

template<class graph_t>
concept graph_cpt = requires(graph_t g,typename graph_t::node_handler_t n,
                                       typename graph_t::edge_handler_t e)
{
    // node_handler_t- lightweight object-reference,
    // that present node of the graph,
    // for example pointer to node or index/iterator in nodes list.
    typename graph_t::node_handler_t;

    // edge_handler_t- lightweight object-reference,
    // that present edge of the graph,
    // for example pointer to edge or index/iterator in edges list.
    typename graph_t::edge_handler_t;

    // edge_begin(node_handler_t node)-return beginning edge handler in
    // adjacency list of node.
    {g.edge_begin(n)}->std::same_as<typename graph_t::edge_handler_t>;

    // graph_t::edge_inc(node_handler_t n,edge_handler_t&e) -
    // incrementing edge e in adjacency list of n.
    {g.edge_inc(n,e)};

    // graph_t::edge_end(node_handler_t n,edge_handler_t e) -
    // return true if e is "end iterator" in adjacency list of n.
    {g.edge_end(n,e)}->std::same_as<bool>;

    // graph_t::target(node_handler_t n,edge_handler_t edge) -
    // return adjacent node with n by edge.
    {g.target(n,e)}->std::same_as<typename graph_t::node_handler_t>;
};

template<class list_t>
concept adjacency_list_cpt=requires(list_t list)
{
    {list[0]};
    {list[0].begin()};
    {list[0].end()};
    {++list[0].begin()};
};

template<class upd_t,class weight_t>
concept weight_updater_cpt= requires(upd_t upd,weight_t w)
{
    // upd_t::priority(weight_t w1,weight_t w2)-
    // return true if w1 have highest priority then w2,
    // in edge relaxation process.
    // ==std::less<weight_t> for minimization problem or
    // ==std::greater<weight_t> for maximization problem.
    {upd.priority(w,w)}->std::same_as<bool>;

    // upd_t::weight_update(weight_t node_weight,weight_t edge_weight)-
    // return weight of adjacent node if it include in
    // optimal path.
    // ==std::plus<weight_t> for shortest path problem or
    // ==std::min()  for bottleneck problem or
    // ==edge_weight for minimal spanning tree problem.
    {upd.weight_update(w,w)}->std::same_as<weight_t>;

    // upd_t::init_weight - initial weight of start node.
    {upd.init_weight()}->std::same_as<weight_t>;
};

template<class search_adapter_t,class node_t,class edge_t>
concept priority_search_adapter_cpt = requires(search_adapter_t sad,node_t n,edge_t e)
{
    {sad.node_preprocess(n)}->std::same_as<bool>;
    {sad.edge_filter(n,e)}->std::same_as<bool>;
};

template<class search_adapter_t,class node_t,class edge_t>
concept breadth_search_adapter_cpt = requires(search_adapter_t sad,node_t n,edge_t e)
{
    {sad.node_preprocess(n)}->std::same_as<bool>;
    {sad.edge_filter(n,e)}->std::same_as<bool>;
    {sad.edge_process(n,e)}->std::same_as<bool>;
};

template<class search_adapter_t,class node_t,class edge_t>
concept depth_search_adapter_cpt = requires(search_adapter_t sad,node_t n,edge_t e)
{
    // search_adapter_t::node_preprocess(node_t n)-
    // call, when node n is first time visited.
    // If return false, algorithm preterm terminate;
    {sad.node_preprocess(n)}->std::same_as<bool>;

    // search_adapter_t::edge_filter(node_t n,edge_t edge)-
    // if return false, this edge ignored in the search.
    {sad.edge_filter(n,e)}->std::same_as<bool>;

    // search_adapter_t::edge_process(node_t n,edge_t edge)-
    // if return false, algorithm preterm terminate;
    {sad.edge_process(n,e)}->std::same_as<bool>;

    // search_adapter_t::node_postprocess(node_t n)-
    // call, when node n is last time visited.
    // if return false, algorithm preterm terminate;
    {sad.node_postprocess(n)}->std::same_as<bool>;
};

}// liga

#endif // _graph_concepts_
