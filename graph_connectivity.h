
#ifndef _graph_connectivity_
#define _graph_connectivity_

#include <vector>

#include "graph_traits.h"
#include "search_settings.h"
#include "graph_utility.h"
#include "node_property.h"
#include "tree_search.h"
#include "breadth_search.h"
#include "depth_search.h"


template<class graph_t>
bool is_strongly_connected(graph_t&g,typename graph_traits<graph_t>::node_handler_t source,std::size_t num_nodes)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using size_t=std::size_t;

    size_t total=0;
    std::vector<char> is_detected_forward(num_nodes,0);
    std::vector<char> is_detected_transp(num_nodes,0);
    std::vector<std::vector<size_t>> transp_graph(num_nodes,std::vector<size_t>{});

    auto set=[](std::vector<char>& vec,size_t node)
    {
        if(vec.size()<=node) vec.resize(node+1,0);
        vec[node]=1;
    };

    auto searcher= full_search_t<graph_t>{}.set_node_preprocess([&](node_t node)
    {
        ++total;
        set(is_detected_forward,traits::index(g,node));
        return true;
    })
    .set_edge_process([&](node_t from,edge_t edge)
    {
        size_t from_index=traits::index(g,from);
        node_t target=traits::target(g,from,edge);
        size_t to_index=traits::index(g,target);
        if(transp_graph.size()<=to_index) transp_graph.resize(to_index+1,{});
        transp_graph[to_index].push_back(from_index);
        return true;
    });
    breadth_search(true,g,source,searcher);

    if(total<num_nodes) return false;

    auto transp_searcher= full_search_t<decltype(transp_graph)>{}.set_node_preprocess([&](size_t node)
    {
        set(is_detected_transp,node);
        return true;
    });
    breadth_search(true,transp_graph,traits::index(g,source),transp_searcher);

    return is_detected_forward==is_detected_transp;
}

template<class graph_t,class cont_t>
std::size_t strongly_connected_components(graph_t&g,cont_t&nodes,
                                          node_property_t<graph_t,std::optional<std::size_t>>&cmnt)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using size_t=std::size_t;

    cmnt.init(g,{});
    tree_search_t<graph_t,std::pair<size_t,size_t>> tree;
    node_property_t<graph_t,std::optional<node_t>> ancestor(g,{});
    size_t count=0;
    auto searcher=full_search_t<graph_t>{}.set_node_preprocess([&](node_t n)
    {
        ancestor.ref(n)=n;
        return true;
    })
    .set_edge_filter([&](node_t n,edge_t e)
    {
        auto opt_an=ancestor(traits::target(g,n,e));
        return !opt_an||!cmnt(*opt_an);
    })
    .set_edge_process([&](node_t n,edge_t e)
    {
        node_t target=traits::target(g,n,e);
        if(auto s=tree.state(target);s==node_state_t::discovered_id||s==node_state_t::closed_id)
        {
            if(tree.dist(target).first<tree.dist(*ancestor(n)).first)
            {
                ancestor.ref(n)=target;
            }
        }
        return true;
    })
    .set_node_postprocess([&](node_t n)
    {
        if(is_equal(g,*ancestor(n),n))
        {
            cmnt.ref(n)=traits::index(g,n);
            ++count;
        }

        node_t prev=tree.m_prev_node(n);
        if(tree.dist(*ancestor(n)).first<tree.dist(*ancestor(prev)).first)
        {
            ancestor.ref(prev)=*ancestor(n);
        }
        return true;
    });

    for(node_t source:nodes)
    {
        if(!ancestor(source))
        {
            depth_search(true,g,source,searcher,tree);
        }
        cmnt.ref(source)=traits::index(g,*ancestor(source));
    }
    return count;
}

template<class graph_t,class cont_t>
std::size_t strongly_connected_components(graph_t&g,cont_t&nodes)
{
    node_property_t<graph_t,std::optional<std::size_t>> cmnts(g,{});
    return strongly_connected_components(g,nodes,cmnts);
}

#endif






















