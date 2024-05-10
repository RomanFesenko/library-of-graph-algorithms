
/** @file depth_search.h
 *  Content:
 *  depth_search
 *  is_undirected_tree
 *  partial_topological_sort
 *  total_topological_sort
 *  is_dag
 *  dag_priority_search
 *  dag_shortest_path
*/

#ifndef _depth_search_
#define _depth_search_

#include <vector>

#include "search_settings.h"
#include "graph_utility.h"
#include "tree_search.h"
#include "property_map.h"

namespace liga{

/**
 *  @brief  Depth first search in subgraph, reachable from source node.
 *  @param  is_directed Is directed input graph.
 *  @param  graph       Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  search      Search adapter,that satisfy breadth_search_adapter_cpt concept.
 *  @param  tree        Tree search,that build algorithm.
 *  @return             Search termination cause.
*/

template<class graph_t,class search_t,class index_map_t>
search_termination_t depth_search(bool is_directed,
                                  graph_t graph,
                                  typename graph_t::node_handler_t source,
                                  search_t&&search,
                                  tree_search_t<typename graph_t::node_handler_t,
                                                typename graph_t::edge_handler_t,
                                                std::pair<std::size_t,std::size_t>,
                                                index_map_t>&tree)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    std::size_t time=0;
    tree.drop();
    std::vector<std::pair<node_t,edge_t>> stack;
    auto push_to_stack=[&](node_t to,node_t from,edge_t prev)
    {
        tree.m_resize_for_node(to);
        tree.m_state(to)=node_state_t::discovered_id;
        tree.m_dist(to).first=time++;
        tree.m_prev_node(to)=from;
        tree.m_prev_edge(to)=prev;
        bool continue_=search.node_preprocess(to);
        if(continue_) stack.push_back({to,graph.edge_begin(to)});
        return continue_;
    };
    if(!push_to_stack(source,source,edge_t{})) return search_termination_t::node_preprocess_id;
    while(!stack.empty())
    {
        auto&[from,e]=stack.back();
        if(graph.edge_end(from,e))
        {
            tree.m_state(from)=node_state_t::closed_id;
            tree.m_dist(from).second=time++;
            if(!search.node_postprocess(from)) return search_termination_t::node_postprocess_id;
            stack.pop_back();
        }
        else
        {
            edge_t cur_edge=e;
            graph.edge_inc(from,e);
            if(!search.edge_filter(from,cur_edge)) continue;
            node_t target=graph.target(from,cur_edge);
            if(is_directed||(tree.state(target)!=node_state_t::closed_id&&!tree.is_equal(target,tree.m_prev_node(from))))
            {
                if(!search.edge_process(from,cur_edge)) return search_termination_t::edge_process_id;
            }
            if(tree.state(target)!=node_state_t::undiscovered_id) continue;
            if(!push_to_stack(target,from,cur_edge)) return search_termination_t::node_preprocess_id;
        }
    }
    return search_termination_t::complete_id;
}

/**
 *  @brief  Depth first search in subgraph, reachable from source node.
 *  @param  is_directed Is directed input graph.
 *  @param  graph       Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  search      Search adapter,that satisfy breadth_search_adapter_cpt concept.
 *  @param  state       Node states after algorithm termination.
 *  @return             Search termination cause.
*/

template<class graph_t,class search_t,class index_map_t>
search_termination_t depth_search(bool is_directed,
                                  graph_t graph,
                                  typename graph_t::node_handler_t source,
                                  search_t&&search,
                                  property_map_t<typename graph_t::node_handler_t,
                                                 node_state_t,
                                                 index_map_t>&state)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;

    state.clear();
    state.default_value()=node_state_t::undiscovered_id;
    if(is_directed)
    {
        std::vector<std::pair<node_t,edge_t>> stack;
        auto push_to_stack=[&](node_t to,node_t from)
        {
            state.ref(to)=node_state_t::discovered_id;
            bool continue_=search.node_preprocess(to);
            if(continue_) stack.push_back({to,graph.edge_begin(to)});
            return continue_;
        };
        if(!push_to_stack(source,source)) return search_termination_t::node_preprocess_id;
        while(!stack.empty())
        {
            auto&[from,e]=stack.back();
            if(graph.edge_end(from,e))
            {
                state.ref(from)=node_state_t::closed_id;
                if(!search.node_postprocess(from)) return search_termination_t::node_postprocess_id;
                stack.pop_back();
            }
            else
            {
                edge_t cur_edge=e;
                graph.edge_inc(from,e);
                if(!search.edge_filter(from,cur_edge)) continue;
                node_t target=graph.target(from,cur_edge);
                if(!search.edge_process(from,cur_edge)) return search_termination_t::edge_process_id;
                if(state(target)!=node_state_t::undiscovered_id) continue;
                if(!push_to_stack(target,from)) return search_termination_t::node_preprocess_id;
            }
        }
    }
    else
    {
        std::vector<std::tuple<node_t,edge_t,node_t>> stack;
        auto push_to_stack=[&](node_t to,node_t from)
        {
            state.ref(to)=node_state_t::discovered_id;
            bool continue_=search.node_preprocess(to);
            if(continue_) stack.push_back({to,graph.edge_begin(to),from});
            return continue_;
        };
        if(!push_to_stack(source,source)) return search_termination_t::node_preprocess_id;
        while(!stack.empty())
        {
            auto&[from,e,prev]=stack.back();
            if(graph.edge_end(from,e))
            {
                state.ref(from)=node_state_t::closed_id;
                if(!search.node_postprocess(from)) return search_termination_t::node_postprocess_id;
                stack.pop_back();
            }
            else
            {
                edge_t cur_edge=e;
                graph.edge_inc(from,e);
                if(!search.edge_filter(from,cur_edge)) continue;
                node_t target=graph.target(from,cur_edge);

                if(state(target)!=node_state_t::closed_id&&
                   !state.is_equal_key(target,prev)&&
                   !search.edge_process(from,cur_edge))
                {
                    return search_termination_t::edge_process_id;
                }

                if(state(target)!=node_state_t::undiscovered_id) continue;
                if(!push_to_stack(target,from)) return search_termination_t::node_preprocess_id;
            }
        }
    }
    return search_termination_t::complete_id;
}

template<class graph_t,class search_t,class index_map_t>
search_termination_t depth_search(bool is_directed,
                                  graph_t graph,
                                  typename graph_t::node_handler_t source,
                                  search_t&&search,
                                  index_map_t imap)
{
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,node_state_t,index_map_t> state(node_state_t::undiscovered_id,imap);
    return depth_search(is_directed,graph,source,std::forward<search_t>(search),state);
}

/**
 *  @brief  Is undirected graph just tree.
 *  @param  graph       Undirected graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  imap        Index map,which mapped graph nodes to unique integer.
 *  @return             True if input graph is tree.
*/

template<class graph_t,class index_map_t>
bool is_undirected_tree(graph_t graph,
                        typename graph_t::node_handler_t source,
                        index_map_t imap)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    property_map_t<node_t,node_state_t,index_map_t> state(node_state_t::undiscovered_id,imap);
    auto search_cycle=full_search_t().set_edge_process([&](node_t out_node,edge_t edge)
    {
        return state(graph.target(out_node,edge))!=node_state_t::discovered_id;
    });
    return depth_search(false,graph,source,search_cycle,state)!=search_termination_t::edge_process_id;
}

/**
 *  @brief  Reverse topological sort of subgraph, reachable from source node.
 *  @param  graph       Directed graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  imap        Index map,which mapped graph nodes to unique integer.
 *  @param  out_ftr     Output functor, that call with output nodes.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class index_map_t,class out_t>
bool partial_topological_sort(graph_t graph,
                              typename graph_t::node_handler_t source,
                              index_map_t imap,
                              out_t out_ftr)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    property_map_t<node_t,node_state_t,index_map_t> state(node_state_t::undiscovered_id,imap);

    auto search_cycle=full_search_t().set_edge_process([&](node_t out_node,edge_t edge)
    {
        return state(graph.target(out_node,edge))!=node_state_t::discovered_id;
    })
    .set_node_postprocess([&](node_t n)
    {
        out_ftr(n);
        return true;
    });
    return depth_search(true,graph,source,search_cycle,state)==search_termination_t::complete_id;
}

/**
 *  @brief  Reverse topological sort of graph.
 *  @param  graph       Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of graph.
 *  @param  imap        Index map,which mapped graph nodes to unique integer.
 *  @param  out_ftr     Output functor, that call with output nodes.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class cont_t,class index_map_t,class out_t>
bool total_topological_sort(graph_t graph,const cont_t& nodes,index_map_t imap,out_t out_ftr)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    property_map_t<node_t,node_state_t,index_map_t> state(node_state_t::undiscovered_id,imap);
    property_map_t<node_t,char,index_map_t>         is_output(0,imap);

    auto search_cycle=full_search_t().set_edge_filter([&](node_t out_node,edge_t edge)
    {
        return !is_output(graph.target(out_node,edge));
    })
    .set_edge_process([&](node_t out_node,edge_t edge)
    {
        return state(graph.target(out_node,edge))!=node_state_t::discovered_id;
    })
    .set_node_postprocess([&](node_t n)
    {
        out_ftr(n);
        is_output.ref(n)=1;
        return true;
    });
    for(node_t source:nodes)
    {
        if(!is_output(source))
        {
            auto res=depth_search(true,graph,source,search_cycle,state);
            if(res!=search_termination_t::complete_id) return false;
        }
    }
    return true;
}

/**
 *  @brief  Is input directed graph is acyclic.
 *  @param  graph       Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of graph.
 *  @param  imap        Index map,which mapped graph nodes to unique integer.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class cont_t,class index_map_t>
bool is_dag(graph_t graph,const cont_t&nodes,index_map_t imap)
{
    using node_t=graph_t::node_handler_t;
    auto void_=[](node_t){};
    return total_topological_sort(graph,nodes,imap,void_);
}

/**
 *  @brief  Priority search in direct acyclic graph.
 *  @param  g           Directed graph,that satisfy graph_cpt concept.
 *  @param  sourse      Any node of graph.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of generic shortest paths,which build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @param  updater     Weight updater, which satisfy weight_updater_cpt.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t,class upd_t>
bool dag_priority_search(graph_t graph,
                         typename graph_t::node_handler_t sourse,
                         search_t&&search,
                         tree_search_t<typename graph_t::node_handler_t,
                                       typename graph_t::edge_handler_t,
                                       weight_t,
                                       index_map_t>&tree,
                         weight_map_t weight_map,
                         upd_t        updater)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    std::vector<node_t> sorted;
    if(!partial_topological_sort(graph,sourse,tree.index_map(),[&](node_t n){sorted.push_back(n);})) return false;
    std::reverse(sorted.begin(),sorted.end());
    tree.drop();
    tree.m_resize_for_node(sourse);
    tree.m_state(sourse)=node_state_t::discovered_id;
    tree.m_dist(sourse)=updater.init_weight();
    tree.m_prev_node(sourse)=sourse;
    assert(tree.in_tree(sourse));
    for(node_t from:sorted)
    {
        if(!tree.in_tree(from)) continue;
        tree.m_state(from)=node_state_t::closed_id;
        if(!search.node_preprocess(from)) return true;
        for(edge_t e=graph.edge_begin(from);!graph.edge_end(from,e);graph.edge_inc(from,e))
        {
             if(!search.edge_filter(from,e)) continue;
             node_t target=graph.target(from,e);
             tree.m_resize_for_node(target);
             weight_t new_dist=updater.weight_update(tree.dist(from),weight_map(from,e));
             if(auto s=tree.state(target);s==node_state_t::undiscovered_id)
             {
                 tree.m_dist(target)=new_dist;
                 tree.m_prev_node(target)=from;
                 tree.m_prev_edge(target)=e;
                 tree.m_state(target)=node_state_t::discovered_id;
             }
             else if(s==node_state_t::discovered_id&&updater.priority(new_dist,tree.m_dist(target)))
             {
                 tree.m_dist(target)=new_dist;
                 tree.m_prev_node(target)=from;
                 tree.m_prev_edge(target)=e;
             }
        }
    }
    return true;
}

/*
    Call list grammar for all dag_shortest_path functions:
    call_list:=graph_t node_t (search_adapter_t|node_t)? tree_search_t
*/

/**
 *  @brief  Find all shortest paths from one source in direct acyclic graph.
 *  @param  graph       Directed graph,that satisfy graph_cpt concept.
 *  @param  sourse      Any node of graph.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of shortest paths,which build algorithm.
 *  @param  weight_map  Weight map,which mapped edge to it weight.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class search_t,class weight_t,class index_map_t,class weight_map_t>
bool dag_shortest_path(graph_t graph,
                       typename graph_t::node_handler_t sourse,
                       search_t&&search,
                       tree_search_t<typename graph_t::node_handler_t,
                                     typename graph_t::edge_handler_t,
                                     weight_t,
                                     index_map_t>&tree,
                       weight_map_t weight_map)
{
    return dag_priority_search(graph,sourse,std::forward<search_t>(search),tree,weight_map,minimize_distance_t<weight_t>{});
}

template<class graph_t,class weight_t,class index_map_t,class weight_map_t>
bool dag_shortest_path(graph_t graph,
                       typename graph_t::node_handler_t sourse,
                       typename graph_t::node_handler_t dest,
                       tree_search_t<typename graph_t::node_handler_t,
                                     typename graph_t::edge_handler_t,
                                     weight_t,
                                     index_map_t>&tree,
                       weight_map_t weight_map)
{
    using node_t=graph_t::node_handler_t;
    auto search_node=search_node_t<node_t,index_map_t>{dest,tree.index_map()};
    return dag_priority_search(graph,sourse,search_node,tree,weight_map,minimize_distance_t<graph_t>{});
}

template<class graph_t,class weight_t,class index_map_t,class weight_map_t>
bool dag_shortest_path(graph_t graph,
                       typename graph_t::node_handler_t sourse,
                       tree_search_t<typename graph_t::node_handler_t,
                                     typename graph_t::edge_handler_t,
                                     weight_t,
                                     index_map_t>&tree,
                       weight_map_t weight_map)
{
    return dag_priority_search(graph,sourse,full_search_t{},tree,weight_map,minimize_distance_t<weight_t>{});
}

}// liga

#endif






















