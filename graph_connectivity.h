
/** @file graph_connectivity.h
 *  Content:
 *  is_strongly_connected
 *  scc_tarjan
 *  scc_kosaraju
 *  scc_brutforce
*/

#ifndef _liga_graph_connectivity_
#define _liga_graph_connectivity_

#include <vector>
#include <optional>

#include "adjacency_list_view.h"
#include "search_settings.h"
#include "graph_utility.h"
#include "property_map.h"
#include "tree_search.h"
#include "breadth_search.h"
#include "depth_search.h"

namespace liga{

/**
 *  @brief  O(E+V) complexity check for strong connectivity of input graph.
 *  @param  graph        Directed graph,that satisfy graph_cpt concept.
 *  @param  source       Any node of graph.
 *  @param  num_nodes    Number of nodes in the graph.
 *  @param  index_map    Index map,which mapped graph nodes to unique integers.
 *  @return              True if graph is strongly connected.
 *
*/

template<class graph_t,class index_map_t>
bool is_strongly_connected(graph_t graph,
                           typename graph_t::node_handler_t source,
                           std::size_t num_nodes,
                           index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    using size_t=std::size_t;
    using list_t=std::vector<std::vector<size_t>>;
    auto target=[](auto iter){return *iter;};
    using view_t=adjacency_list_view_t<list_t,decltype(target),size_t>;

    size_t total=0;
    std::vector<char> is_detected_forward(num_nodes,0);
    std::vector<char> is_detected_transp(num_nodes,0);
    list_t transp_graph(num_nodes,std::vector<size_t>{});

    auto set=[](std::vector<char>& vec,size_t node)
    {
        if(vec.size()<=node) vec.resize(node+1,0);
        vec[node]=1;
    };

    auto searcher= full_search_t{}.set_node_preprocess([&](node_t node)
    {
        ++total;
        set(is_detected_forward,index_map(node));
        return true;
    })
    .set_edge_process([&](node_t from,edge_t edge)
    {
        size_t from_index=index_map(from);
        node_t target=graph.target(from,edge);
        size_t to_index=index_map(target);
        if(transp_graph.size()<=to_index) transp_graph.resize(to_index+1,{});
        transp_graph[to_index].push_back(from_index);
        return true;
    });
    breadth_search(true,graph,source,searcher,index_map);

    if(total<num_nodes) return false;

    auto transp_searcher= full_search_t{}.set_node_preprocess([&](size_t node)
    {
        set(is_detected_transp,node);
        return true;
    });
    breadth_search(true,view_t(transp_graph),index_map(source),transp_searcher,default_index_map_t{});

    return is_detected_forward==is_detected_transp;
}

/**
 *  @brief  Tarjan algorithm for search strongly connected components in digraph.
 *  @param  graph        Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of graph.
 *  @param  cmnt         Node property map,which contain "provoked" component's node.
 *  @return              Number of strong connected components.
 *
*/

template<class graph_t,class cont_t,class index_map_t>
std::size_t scc_tarjan(graph_t graph,
                       const cont_t&nodes,
                       property_map_t<typename graph_t::node_handler_t,
                                      std::optional<typename graph_t::node_handler_t>,
                                      index_map_t>&cmnt)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    using size_t=std::size_t;

    cmnt.default_value()={};
    tree_search_t<node_t,edge_t,std::pair<size_t,size_t>,index_map_t> tree(cmnt.index_map());
    property_map_t<node_t,node_t,index_map_t> ancestor({},cmnt.index_map());
    std::vector<node_t> comp_stack;
    size_t count=0;
    auto searcher=full_search_t{}.set_node_preprocess([&](node_t n)
    {
        ancestor.ref(n)=n;
        comp_stack.push_back(n);
        return true;
    })
    .set_edge_filter([&](node_t n,edge_t e)
    {
        node_t target=graph.target(n,e);
        if(cmnt(target)) return false;
        if(auto s=tree.state(target);s!=node_state_t::undiscovered_id)
        {
            if(tree.dist(target).first<tree.dist(ancestor(n)).first)
            {
                ancestor.ref(n)=target;
            }
        }
        return true;
    })
    .set_node_postprocess([&](node_t n)
    {
        if(tree.is_equal(ancestor(n),n))
        {
            assert(!comp_stack.empty());
            while(!tree.is_equal(comp_stack.back(),n))
            {
                cmnt.ref(comp_stack.back())=n;
                comp_stack.pop_back();
            }
            comp_stack.pop_back();
            cmnt.ref(n)=n;
            ++count;
        }

        node_t prev=tree.m_prev_node(n);
        if(tree.dist(ancestor(n)).first<tree.dist(ancestor(prev)).first)
        {
            ancestor.ref(prev)=ancestor(n);
        }
        return true;
    });

    for(node_t source:nodes)
    {
        if(!cmnt(source))
        {
            depth_search(true,graph,source,searcher,tree);
        }
    }
    return count;
}

template<class graph_t,class cont_t,class index_map_t>
std::size_t scc_tarjan(graph_t graph,const cont_t&nodes,index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,std::optional<node_t>,index_map_t> cmnts({},index_map);
    return scc_tarjan(graph,nodes,cmnts);
}

/**
 *  @brief  Kosaraju algorithm for search strongly connected components in digraph.
 *  @param  graph        Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of graph.
 *  @param  cmnt         Node property map,which contain "provoked" component's node.
 *  @return              Number of strong connected components.
 *
*/

template<class graph_t,class cont_t,class index_map_t>
std::size_t scc_kosaraju(graph_t graph,
                         const cont_t&nodes,
                         property_map_t<typename graph_t::node_handler_t,
                                        std::optional<typename graph_t::node_handler_t>,
                                        index_map_t>&cmnt)
{
    using size_t=std::size_t;
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    using tr_list_t=std::vector<std::vector<node_t>>;
    auto target=[](auto iter){return *iter;};
    using view_t=adjacency_list_view_t<tr_list_t,decltype(target),node_t,index_map_t>;
    using tr_edge_t=view_t::edge_handler_t;

    const index_map_t imap=cmnt.index_map();
    property_map_t<node_t,char,index_map_t> visited(0,imap);
    std::vector<node_t> top_sorted;
    tr_list_t transposed;

    // topological sort and building of transposed graph at the same time
    auto search=full_search_t{}.set_edge_filter([&,graph,imap](node_t from,edge_t edge)
    {
        node_t target=graph.target(from,edge);
        transposed[imap(target)].push_back(from);
        return !visited(target);
    })
    .set_node_postprocess([&](node_t n)
    {
        top_sorted.push_back(n);
        visited.ref(n)=1;
        return true;
    });
    size_t max_index=0;
    for(node_t node:nodes)
    {
        max_index=std::max(max_index,imap(node));
    }
    transposed.resize(max_index+1,{});
    for(node_t source:nodes)
    {
        if(!visited(source)) depth_search(true,graph,source,search,imap);
    }
    std::reverse(top_sorted.begin(),top_sorted.end());

    // traverse of transposed graph
    view_t transposed_view(transposed,target,imap);
    node_t parent;
    auto search_in_transpose=full_search_t{}.set_node_preprocess([&](node_t n)
    {
        cmnt.ref(n)=parent;
        return true;
    })
    .set_edge_filter([&cmnt,transposed_view](node_t n,tr_edge_t e)
    {
        return !cmnt(transposed_view.target(n,e));
    });

    size_t num_cmpnt=0;
    for(size_t source:top_sorted)
    {
        parent=source;
        if(!cmnt(source))
        {
            ++num_cmpnt;
            depth_search(true,transposed_view,source,search_in_transpose,cmnt.index_map());
        }
    }
    return num_cmpnt;
}

template<class graph_t,class cont_t,class index_map_t>
std::size_t scc_kosaraju(graph_t graph,const cont_t&nodes,index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,std::optional<node_t>,index_map_t> cmnts({},index_map);
    return scc_kosaraju(graph,nodes,cmnts);
}

/**
 *  @brief  Brute force O(V*E) algorithm for search strongly connected components in digraph.
 *  @param  graph        Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of graph.
 *  @param  cmnt         Node property map,which contain "provoked" component's node.
 *  @return              Number of strong connected components.
 *
 *  This function for debug only.
*/

template<class graph_t,class cont_t,class index_map_t>
std::size_t scc_brutforce(graph_t graph,
                          const cont_t&nodes,
                          property_map_t<typename graph_t::node_handler_t,
                                         std::optional<typename graph_t::node_handler_t>,
                                         index_map_t>&cmnt)
{
    using size_t=std::size_t;
    using node_t=graph_t::node_handler_t;
    std::vector<property_map_t<node_t,node_state_t,index_map_t>> transitivity;
    std::vector<node_t> sources;
    for(node_t source:nodes)
    {
        sources.push_back(source);
        transitivity.push_back({node_state_t::undiscovered_id,cmnt.index_map()});
        breadth_search(true,graph,source,full_search_t{},transitivity.back());
    }

    size_t num=0;
    for(size_t i=0;i<sources.size();++i)
    {
        node_t from=sources[i];
        if(!cmnt(from))
        {
            ++num;
            for(size_t j=0;j<sources.size();++j)
            {
                node_t to=sources[j];
                if(transitivity[i](to)!=node_state_t::undiscovered_id&&
                   transitivity[j](from)!=node_state_t::undiscovered_id)
                {
                    cmnt.ref(to)=from;
                }
            }
        }
    }
    return num;
}

template<class graph_t,class cont_t,class index_map_t>
std::size_t scc_brutforce(graph_t graph,const cont_t&nodes,index_map_t index_map)
{
    using node_t=graph_t::node_handler_t;
    property_map_t<node_t,std::optional<node_t>,index_map_t> cmnts({},index_map);
    return scc_brutforce(graph,nodes,cmnts);
}

}// liga

#endif






















