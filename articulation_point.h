
/** @file articulation_point.h
 *  Find articulation points and bridges of the undirected graph.
*/

#ifndef _articulation_point_
#define _articulation_point_

#include <vector>
#include <algorithm>


#include "graph_traits.h"
#include "search_settings.h"
#include "graph_utility.h"
#include "node_property.h"
#include "tree_search.h"
#include "depth_search.h"

namespace liga{

/**
 *  @brief  Find articulation points and bridges of the undirected graph.
 *  @param  graph        Undirected graph,that satisfy graph_cpt concept.
 *  @param  source       Any node of graph.
 *  @param  out_point    Unary functor for output articulation points.
 *  @param  out_bridge_t Binary functor for output bridges.
 *  @return              Nothing.
 *
 *  This function find all articulation points and bridges of graph,
 *  call out_point(point), out_bridge(node,out_edge).
*/

template<class graph_t,class out_point_t,class out_bridge_t>
void articulation_point(const graph_t&graph,
                        typename graph_traits<graph_t>::node_handler_t source,
                        out_point_t out_point,
                        out_bridge_t out_bridge)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using size_t=std::size_t;

    node_property_t<graph_t,node_t> ancestor(graph,{});
    node_property_t<graph_t,size_t> degrees(graph,0);
    node_property_t<graph_t,char>   is_out(graph,0);
    tree_search_t<graph_t,std::pair<size_t,size_t>> tree;

    auto searcher=full_search_t<graph_t>{}.set_node_preprocess([&](node_t n)
    {
        ancestor.ref(n)=n;
        return true;
    })
    .set_edge_process([&](node_t n,edge_t e)
    {
        node_t target=traits::target(graph,n,e);
        if(tree.in_tree(target))
        {
            if(tree.dist(target).first<tree.dist(ancestor(n)).first)
            {
                ancestor.ref(n)=target;
            }
        }
        else
        {
            ++degrees.ref(n);
        }
        return true;
    })
    .set_node_postprocess([&](node_t n)
    {
        if(is_equal(graph,n,source))
        {
            if(degrees(n)>1)
            {
                out_point(n);
            }
        }
        else
        {
            node_t prev=tree.prev_node(n);
            if(is_equal(graph,ancestor(n),prev))
            {
                 if(!is_equal(graph,source,prev)&&!is_out(prev))
                 {
                     out_point(prev);
                     is_out.ref(prev)=1;
                 }
            }
            else if(is_equal(graph,ancestor(n),n))
            {
                out_bridge(prev,tree.prev_edge(n));
                assert(degrees(n)==0||is_out(n));
                if(!is_equal(graph,source,prev)&&!is_out(prev))
                {
                    out_point(prev);
                    is_out.ref(prev)=1;
                }
            }

            if(tree.dist(ancestor(n)).first<tree.dist(ancestor(prev)).first)
            {
                ancestor.ref(prev)=ancestor(n);
            }
        }
        return true;
    });

    depth_search(false,graph,source,searcher,tree);
}

}// liga

#endif






















