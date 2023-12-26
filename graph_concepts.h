
#ifndef _graph_concepts_
#define _graph_concepts_

#include <concepts>
#include <type_traits>

template<class hgraph_t>
concept graph_handler_cpt = requires(hgraph_t g,typename hgraph_t::node_handler_t n,
                                                typename hgraph_t::edge_handler_t e)
{
    typename hgraph_t::node_handler_t;
    typename hgraph_t::edge_handler_t;

    {g.edge_begin(n)}->std::same_as<typename hgraph_t::edge_handler_t>;
    {g.edge_inc(n,e)};
    {g.edge_end(n,e)}->std::same_as<bool>;
    {g.target(n,e)}->std::same_as<typename hgraph_t::node_handler_t>;
    {g.index(n)}->std::convertible_to<int>;
};

template<class hgraph_t>
concept w_graph_handler_cpt = requires(hgraph_t g,typename hgraph_t::node_handler_t n,
                                                  typename hgraph_t::edge_handler_t e)
{
    graph_handler_cpt<hgraph_t>;
    typename hgraph_t::weight_t;
    {g.weight(n,e)}->std::same_as<typename hgraph_t::weight_t>;
};

template<class vec_list_t>
concept vector_list_cpt=requires(vec_list_t vec_list)
{
    {vec_list[0]};
    {vec_list[0].begin()};
    {vec_list[0].end()};
    {++vec_list[0].begin()};
    {*(vec_list[0].begin())}->std::convertible_to<std::size_t>;
};

template<class vec_list_t>
concept w_vector_list_cpt=requires(vec_list_t w_vec_list)
{
    {w_vec_list[0]};
    {w_vec_list[0].begin()};
    {w_vec_list[0].end()};
    {++w_vec_list[0].begin()};
    {std::get<0>(*w_vec_list[0].begin())}->std::convertible_to<std::size_t>;
    {std::get<1>(*w_vec_list[0].begin())};
};

template<class upd_t,class weight_t>
concept weight_updater_cpt= requires(upd_t upd,weight_t w)
{
    {upd.priority(w,w)}->std::same_as<bool>;
    {upd.weight_update(w,w)}->std::same_as<weight_t>;
    {upd.init_weight()}->std::same_as<weight_t>;
};

template<class search_adapter_t,class hnode_t,class hedge_t>
concept priority_search_adapter_cpt = requires(search_adapter_t sad,hnode_t n,hedge_t e)
{
    {sad.node_preprocess(n)}->std::same_as<bool>;
    {sad.edge_filter(n,e)}->std::same_as<bool>;
};

template<class search_adapter_t,class hnode_t,class hedge_t>
concept breadth_search_adapter_cpt = requires(search_adapter_t sad,hnode_t n,hedge_t e)
{
    {sad.node_preprocess(n)}->std::same_as<bool>;
    {sad.edge_filter(n,e)}->std::same_as<bool>;
    {sad.edge_process(n,e)}->std::same_as<bool>;
};

template<class search_adapter_t,class hnode_t,class hedge_t>
concept depth_search_adapter_cpt = requires(search_adapter_t sad,hnode_t n,hedge_t e)
{
    {sad.node_preprocess(n)}->std::same_as<bool>;
    {sad.edge_filter(n,e)}->std::same_as<bool>;
    {sad.edge_process(n,e)}->std::same_as<bool>;
    {sad.node_postprocess(n)}->std::same_as<bool>;
};


#endif // _graph_concepts_
