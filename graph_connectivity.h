
/** @file graph_connectivity.h
 *  Content:
 *  is_strongly_connected
 *  scc_tarjan
 *  scc_kosaraju
 *  scc_brutforce
*/

#ifndef _graph_connectivity_
#define _graph_connectivity_

#include <vector>
#include <optional>

#include "graph_traits.h"
#include "search_settings.h"
#include "graph_utility.h"
#include "node_property.h"
#include "tree_search.h"
#include "breadth_search.h"
#include "depth_search.h"

namespace liga{

/**
 *  @brief  O(E+V) complexity check for strong connectivity of input graph.
 *  @param  g            Directed graph,that satisfy graph_cpt concept.
 *  @param  source       Any node of graph.
 *  @param  num_nodes    Number of nodes in the graph.
 *  @return              True if graph is strongly connected.
 *
*/

template<class graph_t>
bool is_strongly_connected(const graph_t&g,typename graph_traits<graph_t>::node_handler_t source,std::size_t num_nodes)
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

/**
 *  @brief  Tarjan algorithm for search strongly connected components in digraph.
 *  @param  g            Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of graph.
 *  @param  cmnt         Node property,which contain "provoked" component's node.
 *  @return              Number of strong connected components.
 *
*/

template<class graph_t,class cont_t>
std::size_t scc_tarjan(const graph_t&g,
                       const cont_t&nodes,
                       node_property_t<graph_t,std::optional<typename graph_traits<graph_t>::node_handler_t>>&cmnt)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using size_t=std::size_t;

    cmnt.init(g,{});
    tree_search_t<graph_t,std::pair<size_t,size_t>> tree;
    node_property_t<graph_t,node_t> ancestor(g,{});
    std::vector<node_t> comp_stack;
    size_t count=0;
    auto searcher=full_search_t<graph_t>{}.set_node_preprocess([&](node_t n)
    {
        ancestor.ref(n)=n;
        comp_stack.push_back(n);
        return true;
    })
    .set_edge_filter([&](node_t n,edge_t e)
    {
        node_t target=traits::target(g,n,e);
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
        if(is_equal(g,ancestor(n),n))
        {
            assert(!comp_stack.empty());
            while(!is_equal(g,comp_stack.back(),n))
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
            depth_search(true,g,source,searcher,tree);
        }
    }
    return count;
}

template<class graph_t,class cont_t>
std::size_t scc_tarjan(const graph_t&g,const cont_t&nodes)
{
    node_property_t<graph_t,std::optional<std::size_t>> cmnts(g,{});
    return scc_tarjan(g,nodes,cmnts);
}

namespace __detail_connectivity
{

template<class base_graph_t>
struct mapped_graph_t
{
    static constexpr bool is_weighted=false;
    using base_traits=graph_traits<base_graph_t>;
    using node_handler_t=base_traits::node_handler_t;
    using edge_handler_t=std::vector<node_handler_t>::const_iterator;
    using weight_t=void;

    const base_graph_t* base=nullptr;
    node_property_t<base_graph_t,std::vector<node_handler_t>> adj_list;

    mapped_graph_t(const base_graph_t&bg):base(&bg),adj_list(bg,{})
    {
    }

    edge_handler_t edge_begin(node_handler_t n)const
    {
        return adj_list(n).begin();
    }
    void edge_inc(node_handler_t n,edge_handler_t&e)const
    {
        ++e;
    }
    bool edge_end(node_handler_t n,edge_handler_t e)const
    {
        return e==adj_list(n).end();
    }
    node_handler_t target(node_handler_t n,edge_handler_t e)const
    {
        return *e;
    }
    std::size_t index(node_handler_t n)const
    {
        return graph_traits<base_graph_t>::index(*base,n);
    }
};

}// __detail_connectivity


/**
 *  @brief  Kosaraju algorithm for search strongly connected components in digraph.
 *  @param  g            Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of graph.
 *  @param  cmnt         Node property,which contain "provoked" component's node.
 *  @return              Number of strong connected components.
 *
*/

template<class graph_t,class cont_t>
std::size_t scc_kosaraju(const graph_t&g,
                         const cont_t&nodes,
                         node_property_t<graph_t,std::optional<typename graph_traits<graph_t>::node_handler_t>>&cmnt)
{
    using namespace __detail_connectivity;

    using size_t=std::size_t;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using tr_edge_t=mapped_graph_t<graph_t>::edge_handler_t;

    node_property_t<graph_t,char> visited(g,0);
    std::vector<node_t> top_sorted;
    mapped_graph_t<graph_t> transposed(g);

    // topological sort and building of transposed graph at the same time
    auto search=full_search_t<graph_t>{}.set_edge_filter([&](node_t from,edge_t edge)
    {
        node_t target=traits::target(g,from,edge);
        transposed.adj_list.ref(target).push_back(from);
        return !visited(traits::target(g,from,edge));
    })
    .set_node_postprocess([&](node_t n)
    {
        top_sorted.push_back(n);
        visited.ref(n)=1;
        return true;
    });
    for(node_t source:nodes)
    {
        if(!visited(source)) depth_search(true,g,source,search);
    }
    std::reverse(top_sorted.begin(),top_sorted.end());

    // traverse of transposed graph
    node_t parent;
    auto search_in_transpose=full_search_t<mapped_graph_t<graph_t>>{}.set_node_preprocess([&](node_t n)
    {
        cmnt.ref(n)=parent;
        return true;
    })
    .set_edge_filter([&](node_t n,tr_edge_t e)
    {
        return !cmnt(transposed.target(n,e));
    });

    size_t num_cmpnt=0;
    for(size_t source:top_sorted)
    {
        parent=source;
        if(!cmnt(source))
        {
            ++num_cmpnt;
            depth_search(true,transposed,source,search_in_transpose);
        }
    }
    return num_cmpnt;
}

template<class graph_t,class cont_t>
std::size_t scc_kosaraju(const graph_t&g,const cont_t&nodes)
{
    node_property_t<graph_t,std::optional<std::size_t>> cmnts(g,{});
    return scc_kosaraju(g,nodes,cmnts);
}

/**
 *  @brief  Brute force O(V*E) algorithm for search strongly connected components in digraph.
 *  @param  g            Directed graph,that satisfy graph_cpt concept.
 *  @param  nodes        All nodes of graph.
 *  @param  cmnt         Node property,which contain "provoked" component's node.
 *  @return              Number of strong connected components.
 *
 *  This function for debug only.
*/

template<class graph_t,class cont_t>
std::size_t scc_brutforce(const graph_t&g,
                          const cont_t&nodes,
                          node_property_t<graph_t,std::optional<typename graph_traits<graph_t>::node_handler_t>>&cmnt)
{
    using size_t=std::size_t;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    std::vector<node_property_t<graph_t,node_state_t>> transitivity;
    std::vector<node_t> sources;
    for(node_t source:nodes)
    {
        sources.push_back(source);
        transitivity.push_back({g,node_state_t::undiscovered_id});
        breadth_search(true,g,source,full_search_t<graph_t>{},transitivity.back());
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

template<class graph_t,class cont_t>
std::size_t scc_brutforce(const graph_t&g,const cont_t&nodes)
{
    node_property_t<graph_t,std::optional<std::size_t>> cmnts(g,{});
    return scc_brutforce(g,nodes,cmnts);
}

}// liga

#endif






















