
#ifndef _articulation_point_
#define _articulation_point_

#include <vector>

#include "graph_traits.h"
#include "search_settings.h"
#include "graph_utility.h"
#include "node_property.h"
#include "tree_search.h"
#include "depth_search.h"


template<class graph_t,class out_point_t,class out_bridge_t>
void articulation_point(graph_t&g,
                        typename graph_traits<graph_t>::node_handler_t source,
                        out_point_t out_point,
                        out_bridge_t out_bridge)
{
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    using size_t=std::size_t;

    node_property_t<graph_t,node_t> ancestor(g,{});
    node_property_t<graph_t,size_t> degrees(g,0);
    node_property_t<graph_t,char>   is_out(g,0);
    tree_search_t<graph_t,std::pair<size_t,size_t>> tree;

    auto searcher=full_search_t<graph_t>{}.set_node_preprocess([&](node_t n)
    {
        ancestor.ref(n)=n;
        return true;
    })
    .set_edge_process([&](node_t n,edge_t e)
    {
        node_t target=traits::target(g,n,e);
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
        if(is_equal(g,n,source))
        {
            if(degrees(n)>1)
            {
                out_point(n);
            }
        }
        else
        {
            node_t prev=tree.prev_node(n);
            if(is_equal(g,ancestor(n),prev))
            {
                 if(!is_equal(g,source,prev)&&!is_out(prev))
                 {
                     out_point(prev);
                     is_out.ref(prev)=1;
                 }
            }
            else if(is_equal(g,ancestor(n),n))
            {
                out_bridge(prev,tree.prev_edge(n));
                assert(degrees(n)==0||is_out(n));
                if(!is_equal(g,source,prev)&&!is_out(prev))
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

    depth_search(false,g,source,searcher,tree);
}

#endif






















