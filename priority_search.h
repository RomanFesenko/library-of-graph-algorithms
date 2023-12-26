
#ifndef _graph_priority_search_
#define _graph_priority_search_

#include <vector>
#include <optional>
#include <algorithm>

#include "priority_queue.h"
#include "graph_traits.h"
#include "search_settings.h"
#include "tree_search.h"


template<class graph_t,class search_t,class upt_t,class queue_t>
search_termination_t
__priority_search(graph_t&g,
                  typename graph_traits<graph_t>::node_handler_t sourse,
                  search_t search,
                  tree_search_t<graph_t>&tree,
                  upt_t updater,
                  queue_t&queue)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using weight_t=traits::weight_t;

    tree.m_init(g);
    tree.m_resize_for_node(sourse);
    tree.m_state(sourse)=node_state_t::discovered_id;
    tree.m_dist(sourse)=updater.init_weight();
    tree.m_prev_node(sourse)=sourse;

    queue.clear();
    queue.push(sourse);
    while(!queue.empty())
    {
        node_t opt_node=queue.pop();
        tree.m_state(opt_node)=node_state_t::closed_id;
        if(!search.node_preprocess(opt_node)) return search_termination_t::node_preprocess_id;
        for(edge_t e=traits::edge_begin(g,opt_node);
                    !traits::edge_end(g,opt_node,e);
                     traits::edge_inc(g,opt_node,e))
        {
            if(!search.edge_filter(opt_node,e)) continue;
            node_t target=traits::target(g,opt_node,e);
            tree.m_resize_for_node(target);
            weight_t new_dist=updater.weight_update(tree.dist(opt_node),traits::weight(g,opt_node,e));
            if(auto s=tree.state(target);s==node_state_t::undiscovered_id)
            {
                tree.m_dist(target)=new_dist;
                tree.m_prev_node(target)=opt_node;
                tree.m_prev_edge(target)=e;
                tree.m_state(target)=node_state_t::discovered_id;
                queue.push(target);
            }
            else if(s==node_state_t::discovered_id&&updater.priority(new_dist,tree.m_dist(target)))
            {
                tree.m_dist(target)=new_dist;
                tree.m_prev_node(target)=opt_node;
                tree.m_prev_edge(target)=e;
                queue.rebuild_after_mod(target);
            }
        }
    }
    return search_termination_t::complete_id;
}


/**
   Priority search in graphs with O(E)=O(V^2)
*/

template<class graph_t,class search_t,class upt_t>
search_termination_t
dense_priority_search(graph_t&g,
                       typename graph_traits<graph_t>::node_handler_t sourse,
                       search_t search,
                       tree_search_t<graph_t>&tree,
                       upt_t  updater)
{
    using node_t=graph_traits<graph_t>::node_handler_t;
    auto comp=[&](node_t n1,node_t n2)
    {
        return updater.priority(tree.dist(n1),tree.dist(n2));
    };
    array_priority_queue_t<node_t,decltype(comp)> queue(comp);
    return __priority_search(g,sourse,search,tree,updater,queue);
}


/**
   Priority search in graphs with O(E)=O(V)
*/

template<class graph_t,class search_t,class upt_t>
search_termination_t
sparse_priority_search(graph_t&g,
                            typename graph_traits<graph_t>::node_handler_t sourse,
                            search_t search,
                            tree_search_t<graph_t>&tree,
                            upt_t  updater)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    auto comp=[&](node_t n1,node_t n2)
    {
        return updater.priority(tree.dist(n1),tree.dist(n2));
    };
    auto indexer=[&](node_t n){return traits::index(g,n);};
    heap_priority_queue_t<node_t,decltype(comp),decltype(indexer)> queue(comp,indexer);
    return __priority_search(g,sourse,search,tree,updater,queue);
}

#endif






















