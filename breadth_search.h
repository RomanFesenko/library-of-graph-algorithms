
#ifndef _breadth_search_
#define _breadth_search_

#include <vector>

#include "graph_traits.h"
#include "search_settings.h"
#include "tree_search.h"
#include "node_property.h"


/**
    Call list for all breadth_search functions:
    call_list:= boolean graph_t node_t search_adapter_t (tree_search_t|node_state_t)?
*/

template<class graph_t,class search_t>
search_termination_t breadth_search(bool is_directed,
                                    graph_t&g,
                                    typename graph_traits<graph_t>::node_handler_t source,
                                    search_t search,
                                    tree_search_t<graph_t,std::size_t>&tree)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;

    tree.m_init(g);
    std::vector<node_t> queue,aux_queue;
    tree.m_resize_for_node(source);
    tree.m_state(source)=node_state_t::discovered_id;
    tree.m_dist(source)=0;
    tree.m_prev_node(source)=source;
    queue.push_back(source);
    while(!queue.empty())
    {
        while(!queue.empty())
        {
            node_t from=queue.back();
            queue.pop_back();
            tree.m_state(from)=node_state_t::closed_id;
            if(!search.node_preprocess(from)) return search_termination_t::node_preprocess_id;
            for(edge_t e=traits::edge_begin(g,from);
                        !traits::edge_end(g,from,e);
                         traits::edge_inc(g,from,e))
            {
                if(!search.edge_filter(from,e)) continue;
                node_t target=traits::target(g,from,e);
                if(is_directed||tree.m_state(target)!=node_state_t::closed_id)
                {
                    if(!search.edge_process(from,e)) return search_termination_t::edge_process_id;
                }
                if(tree.in_tree(target)) continue;
                tree.m_resize_for_node(target);
                tree.m_dist(target)=tree.m_dist(from)+1;
                tree.m_prev_node(target)=from;
                tree.m_prev_edge(target)=e;
                tree.m_state(target)=node_state_t::discovered_id;
                aux_queue.push_back(target);
            }
        }
        std::swap(aux_queue,queue);
        aux_queue.clear();
    }
    return search_termination_t::complete_id;
}

template<class graph_t,class search_t>
search_termination_t breadth_search(bool is_directed,
                                    graph_t&g,
                                    typename graph_traits<graph_t>::node_handler_t source,
                                    search_t search,
                                    node_property_t<graph_t,node_state_t>&state)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;

    state.init(g,node_state_t::undiscovered_id);
    std::vector<node_t> queue,aux_queue;
    state(source,node_state_t::discovered_id);
    queue.push_back(source);
    while(!queue.empty())
    {
        while(!queue.empty())
        {
            node_t from=queue.back();
            queue.pop_back();
            state(from,node_state_t::closed_id);
            if(!search.node_preprocess(from)) return search_termination_t::node_preprocess_id;
            for(edge_t e=traits::edge_begin(g,from);
                        !traits::edge_end(g,from,e);
                         traits::edge_inc(g,from,e))
            {
                if(!search.edge_filter(from,e)) continue;
                node_t target=traits::target(g,from,e);
                if(is_directed||state(target)!=node_state_t::closed_id)
                {
                    if(!search.edge_process(from,e)) return search_termination_t::edge_process_id;
                }
                if(state(target)!=node_state_t::undiscovered_id) continue;
                state(target,node_state_t::discovered_id);
                aux_queue.push_back(target);
            }
        }
        std::swap(aux_queue,queue);
        aux_queue.clear();
    }
    return search_termination_t::complete_id;
}

template<class graph_t,class search_t>
search_termination_t breadth_search(bool is_directed,
                                    graph_t&g,
                                    typename graph_traits<graph_t>::node_handler_t source,
                                    search_t search)
{
    node_property_t<graph_t,node_state_t> state(g,node_state_t::undiscovered_id);
    return breadth_search(is_directed,g,source,search,state);
}

template<class graph_t>
bool is_bipartite(const graph_t&g,typename graph_traits<graph_t>::node_handler_t source)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    node_property_t<graph_t,char> color(g,2);
    color(source,0);
    auto search=full_search_t<graph_t>().set_edge_process([&](node_t out_node,edge_t edge)
    {
        assert(color(out_node)!=2);
        node_t t=traits::target(g,out_node,edge);
        if(color(t)==2)
        {
            color(t,color(out_node)==0? 1:0);
        }
        return color(t)!=color(out_node);
    });
    return breadth_search(false,g,source,search)!=search_termination_t::edge_process_id;
}

template<class graph_t>
std::pair<std::size_t,std::size_t> graph_quantities(bool is_directed,
                                                   graph_t&g,
                                                   typename graph_traits<graph_t>::node_handler_t source)
{
    using traits=graph_traits<graph_t>;
    using size_t=std::size_t;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    std::pair<size_t,size_t> res;
    auto search=full_search_t<graph_t>{}.set_node_preprocess([&](node_t out_node)
    {
        ++res.first;
        return true;
    }).
    set_edge_process([&](node_t out_node,edge_t edge)
    {
        ++res.second;
        return true;
    });
    breadth_search(is_directed,g,source,search);
    return res;
}

#endif






















