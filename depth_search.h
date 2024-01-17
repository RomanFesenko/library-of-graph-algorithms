
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

#include "graph_traits.h"
#include "search_settings.h"
#include "graph_utility.h"
#include "tree_search.h"
#include "node_property.h"

namespace liga{

/**
 *  @brief  Depth first search in subgraph, reachable from source node.
 *  @param  is_directed Is directed input graph.
 *  @param  g           Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  search      Search adapter,that satisfy breadth_search_adapter_cpt concept.
 *  @param  tree        Tree search,that build algorithm.
 *  @return             Search termination cause.
*/

template<class graph_t,class search_t>
search_termination_t depth_search(bool is_directed,
                                  const graph_t&g,
                                  typename graph_traits<graph_t>::node_handler_t source,
                                  search_t search,
                                  tree_search_t<graph_t,std::pair<std::size_t,std::size_t>>&tree)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;

    std::size_t time=0;
    tree.m_init(g);
    std::vector<std::pair<node_t,edge_t>> stack;
    auto push_to_stack=[&](node_t to,node_t from,edge_t prev)
    {
        tree.m_resize_for_node(to);
        tree.m_state(to)=node_state_t::discovered_id;
        tree.m_dist(to).first=time++;
        tree.m_prev_node(to)=from;
        tree.m_prev_edge(to)=prev;
        bool continue_=search.node_preprocess(to);
        if(continue_) stack.push_back({to,traits::edge_begin(g,to)});
        return continue_;
    };
    if(!push_to_stack(source,source,edge_t{})) return search_termination_t::node_preprocess_id;
    while(!stack.empty())
    {
        auto&[from,e]=stack.back();
        if(traits::edge_end(g,from,e))
        {
            tree.m_state(from)=node_state_t::closed_id;
            tree.m_dist(from).second=time++;
            if(!search.node_postprocess(from)) return search_termination_t::node_postprocess_id;
            stack.pop_back();
        }
        else
        {
            edge_t cur_edge=e;
            traits::edge_inc(g,from,e);
            if(!search.edge_filter(from,cur_edge)) continue;
            node_t target=traits::target(g,from,cur_edge);
            if(is_directed||(tree.state(target)!=node_state_t::closed_id&&
                             !is_equal(g,target,tree.m_prev_node(from))))
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
 *  @param  g           Graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  search      Search adapter,that satisfy breadth_search_adapter_cpt concept.
 *  @param  state       Node states after algorithm termination.
 *  @return             Search termination cause.
*/

template<class graph_t,class search_t>
search_termination_t depth_search(bool is_directed,
                                  const graph_t&g,
                                  typename graph_traits<graph_t>::node_handler_t source,
                                  search_t search,
                                  node_property_t<graph_t,node_state_t>&state)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;

    state.init(g,node_state_t::undiscovered_id);
    if(is_directed)
    {
        std::vector<std::pair<node_t,edge_t>> stack;
        auto push_to_stack=[&](node_t to,node_t from)
        {
            state(to,node_state_t::discovered_id);
            bool continue_=search.node_preprocess(to);
            if(continue_) stack.push_back({to,traits::edge_begin(g,to)});
            return continue_;
        };
        if(!push_to_stack(source,source)) return search_termination_t::node_preprocess_id;
        while(!stack.empty())
        {
            auto&[from,e]=stack.back();
            if(traits::edge_end(g,from,e))
            {
                state(from,node_state_t::closed_id);
                if(!search.node_postprocess(from)) return search_termination_t::node_postprocess_id;
                stack.pop_back();
            }
            else
            {
                edge_t cur_edge=e;
                traits::edge_inc(g,from,e);
                if(!search.edge_filter(from,cur_edge)) continue;
                node_t target=traits::target(g,from,cur_edge);
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
            state(to,node_state_t::discovered_id);
            bool continue_=search.node_preprocess(to);
            if(continue_) stack.push_back({to,traits::edge_begin(g,to),from});
            return continue_;
        };
        if(!push_to_stack(source,source)) return search_termination_t::node_preprocess_id;
        while(!stack.empty())
        {
            auto&[from,e,prev]=stack.back();
            if(traits::edge_end(g,from,e))
            {
                state(from,node_state_t::closed_id);
                if(!search.node_postprocess(from)) return search_termination_t::node_postprocess_id;
                stack.pop_back();
            }
            else
            {
                edge_t cur_edge=e;
                traits::edge_inc(g,from,e);
                if(!search.edge_filter(from,cur_edge)) continue;
                node_t target=traits::target(g,from,cur_edge);

                if(state(target)!=node_state_t::closed_id&&
                   traits::index(g,target)!=traits::index(g,prev)&&
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

template<class graph_t,class search_t>
search_termination_t depth_search(bool is_directed,
                                  const graph_t&graph,
                                  typename graph_traits<graph_t>::node_handler_t source,
                                  search_t search)
{
    node_property_t<graph_t,node_state_t> state(graph,node_state_t::undiscovered_id);
    return depth_search(is_directed,graph,source,search,state);
}

/**
 *  @brief  Is undirected graph just tree.
 *  @param  g           Undirected graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @return             True if input graph is tree.
*/

template<class graph_t>
bool is_undirected_tree(const graph_t&g,typename graph_traits<graph_t>::node_handler_t source)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    node_property_t<graph_t,node_state_t> state(g,node_state_t::undiscovered_id);

    auto search_cycle=full_search_t<graph_t>().set_edge_process([&](node_t out_node,edge_t edge)
    {
        return state(traits::target(g,out_node,edge))!=node_state_t::discovered_id;
    });
    return depth_search(false,g,source,search_cycle,state)!=search_termination_t::edge_process_id;
}

/**
 *  @brief  Reverse topological sort of subgraph, reachable from source node.
 *  @param  g           Directed graph,that satisfy graph_cpt concept.
 *  @param  source      Any node of graph.
 *  @param  out_ftr     Output functor, that call with output nodes.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class out_t>
bool partial_topological_sort(const graph_t&g,
                              typename graph_traits<graph_t>::node_handler_t source,
                              out_t out_ftr)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    node_property_t<graph_t,node_state_t> state(g,node_state_t::undiscovered_id);

    auto search_cycle=full_search_t<graph_t>().set_edge_process([&](node_t out_node,edge_t edge)
    {
        return state(traits::target(g,out_node,edge))!=node_state_t::discovered_id;
    })
    .set_node_postprocess([&](node_t n)
    {
        out_ftr(n);
        return true;
    });
    return depth_search(true,g,source,search_cycle,state)==search_termination_t::complete_id;
}

/**
 *  @brief  Reverse topological sort of graph.
 *  @param  g           Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of graph.
 *  @param  out_ftr     Output functor, that call with output nodes.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class cont_t,class out_t>
bool total_topological_sort(const graph_t&g,const cont_t& nodes,out_t out_ftr)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    node_property_t<graph_t,node_state_t> state(g,node_state_t::undiscovered_id);
    node_property_t<graph_t,char> is_output(g,0);

    auto search_cycle=full_search_t<graph_t>().set_edge_filter([&](node_t out_node,edge_t edge)
    {
        return !is_output(traits::target(g,out_node,edge));
    })
    .set_edge_process([&](node_t out_node,edge_t edge)
    {
        return state(traits::target(g,out_node,edge))!=node_state_t::discovered_id;
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
            auto res=depth_search(true,g,source,search_cycle,state);
            if(res!=search_termination_t::complete_id) return false;
        }
    }
    return true;
}

/**
 *  @brief  Is input directed graph is acyclic.
 *  @param  g           Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes       All nodes of graph.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class cont_t>
bool is_dag(const graph_t&g,const cont_t&nodes)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    auto void_=[](node_t){};
    return total_topological_sort(g,nodes,void_);
}

/**
 *  @brief  Priority search in direct acyclic graph.
 *  @param  g           Directed graph,that satisfy graph_cpt concept.
 *  @param  sourse      Any node of graph.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of generic shortest paths,which build algorithm.
 *  @param  updater     Weight updater, which satisfy weight_updater_cpt.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class search_t,class upd_t>
bool dag_priority_search(const graph_t&g,
                         typename graph_traits<graph_t>::node_handler_t sourse,
                         search_t search,
                         tree_search_t<graph_t>&tree,
                         upd_t  updater)
{
    using gts=graph_traits<graph_t>;
    using node_t=gts::node_handler_t;
    using edge_t=gts::edge_handler_t;
    using weight_t=gts::weight_t;
    std::vector<node_t> sorted;
    if(!partial_topological_sort(g,sourse,[&](node_t n){sorted.push_back(n);})) return false;
    std::reverse(sorted.begin(),sorted.end());
    tree.m_init(g);
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
        for(edge_t e=gts::edge_begin(g,from);!gts::edge_end(g,from,e);gts::edge_inc(g,from,e))
        {
             if(!search.edge_filter(from,e)) continue;
             node_t target=gts::target(g,from,e);
             tree.m_resize_for_node(target);
             weight_t new_dist=updater.weight_update(tree.dist(from),gts::weight(g,from,e));
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
 *  @param  g           Directed graph,that satisfy graph_cpt concept.
 *  @param  sourse      Any node of graph.
 *  @param  search      Search adapter,that satisfy priority_search_adapter_cpt concept.
 *  @param  tree        Tree of shortest paths,which build algorithm.
 *  @return             True if input graph is acyclic.
*/

template<class graph_t,class search_t>
bool dag_shortest_path(const graph_t&g,
                       typename graph_traits<graph_t>::node_handler_t sourse,
                       search_t search,
                       tree_search_t<graph_t>&tree)
{
    return dag_priority_search(g,sourse,search,tree,minimize_distance_t<graph_t>{});
}

template<class graph_t>
bool dag_shortest_path(const graph_t&g,
                       typename graph_traits<graph_t>::node_handler_t sourse,
                       typename graph_traits<graph_t>::node_handler_t dest,
                       tree_search_t<graph_t>&tree)
{
    return dag_priority_search(g,sourse,search_node_t<graph_t>{g,dest},tree,minimize_distance_t<graph_t>{});
}

template<class graph_t>
bool dag_shortest_path(const graph_t&g,
                       typename graph_traits<graph_t>::node_handler_t sourse,
                       tree_search_t<graph_t>&tree)
{
    return dag_priority_search(g,sourse,full_search_t<graph_t>{},tree,minimize_distance_t<graph_t>{});
}

}// liga

#endif






















