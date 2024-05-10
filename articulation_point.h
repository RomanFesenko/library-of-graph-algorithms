
/** @file articulation_point.h
 *  Find articulation points and bridges of the undirected graph.
*/

#ifndef _articulation_point_
#define _articulation_point_

#include <vector>
#include <algorithm>

#include "search_settings.h"
#include "graph_utility.h"
#include "property_map.h"
#include "tree_search.h"
#include "depth_search.h"

namespace liga{

/**
 *  @brief  Find articulation points and bridges of the undirected graph.
 *  @param  graph        Undirected graph,that satisfy graph_cpt concept.
 *  @param  source       Any node of graph.
 *  @param  i_map        Index map,which mapped node to unique integer.
 *  @param  out_point    Unary functor for output articulation points.
 *  @param  out_bridge_t Binary functor for output bridges.
 *  @return              Nothing.
 *
 *  This function find all articulation points and bridges of graph,
 *  call out_point(point), out_bridge(node,out_edge).
*/

template<class graph_t,class index_map_t,class out_point_t,class out_bridge_t>
void articulation_point(graph_t graph,
                        typename graph_t::node_handler_t source,
                        index_map_t i_map,
                        out_point_t out_point,
                        out_bridge_t out_bridge)
{
    using node_t=graph_t::node_handler_t;
    using edge_t=graph_t::edge_handler_t;
    using size_t=std::size_t;

    property_map_t<node_t,node_t,index_map_t> ancestor({},i_map);
    property_map_t<node_t,size_t,index_map_t> degrees(0,i_map);
    property_map_t<node_t,char,index_map_t>   is_out(0,i_map);
    tree_search_t<node_t,edge_t,std::pair<size_t,size_t>,index_map_t> tree(i_map);

    auto searcher=full_search_t{}.set_node_preprocess([&](node_t n)
    {
        ancestor.ref(n)=n;
        return true;
    })
    .set_edge_process([&](node_t n,edge_t e)
    {
        node_t target=graph.target(n,e);
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
        if(i_map(n)==i_map(source))
        {
            if(degrees(n)>1)
            {
                out_point(n);
            }
        }
        else
        {
            node_t prev=tree.prev_node(n);
            if(i_map(ancestor(n))==i_map(prev))
            {
                 if(i_map(source)!=i_map(prev)&&!is_out(prev))
                 {
                     out_point(prev);
                     is_out.ref(prev)=1;
                 }
            }
            else if(i_map(n)==i_map(ancestor(n)))
            {
                out_bridge(prev,tree.prev_edge(n));
                assert(degrees(n)==0||is_out(n));
                if(i_map(source)!=i_map(prev)&&!is_out(prev))
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






















