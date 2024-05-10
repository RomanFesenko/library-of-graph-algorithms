#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "../../math/common/scalar_traits.h"
#include "../../exts/timing.h"

#include "../graph_generator.h"
#include "../dijkstra.h"
#include "../astar.h"

static bool is_equal(double x,double y)
{
    return scalar_traits<double>::almost_equal(x,y);
}

void test_euclidian()
{
    using namespace liga;
    using egraph_t=graph_generator_t::euclidian_graph_t<double>;
    using node_t=egraph_t::view_t::node_handler_t;
    using edge_t=egraph_t::view_t::edge_handler_t;
    using size_t=std::size_t;
    const size_t num_nodes=600;
    const size_t num_edges=num_nodes*40;
    const size_t source=1;
    bool valid;
    egraph_t graph;
    graph_generator_t ggen;
    graph_generator_t::request_t request;
    request.type=graph_generator_t::undirected_graph_id;
    request.num_nodes=num_nodes;
    request.num_edges=num_edges;
    assert(request.is_valid());
    valid=ggen.make_euclidian_graph<double>(graph,request,{{-1,-1},{1,1}});
    assert(valid);

    tree_search_t<node_t,edge_t,double,default_index_map_t> tree,tree2;
    auto weight_map=[](node_t n,edge_t e)
    {
        return e->second;
    };

    std::vector<double> dists(num_nodes),dists2(num_nodes);
    double time,time2;
    exts::CTimer timer;
    for(size_t dest=0;dest<num_nodes;++dest)
    {
        sparse_dijkstra(graph.view(),source,search_node_t(dest),tree,weight_map);
        assert(tree.opt_dist(dest));
        dists[dest]=*tree.opt_dist(dest);
    }
    time=timer.Pass<std::chrono::milliseconds>();

    auto hec=[&](size_t node,size_t dest)
    {
        return egraph_t::distance(graph.nodes[node],graph.nodes[dest]);
    };
    timer.Restart();
    for(size_t dest=0;dest<num_nodes;++dest)
    {
        valid=sparse_astar_shortest_path(graph.view(),source,dest,tree2,weight_map,hec);
        assert(valid);
        assert(tree2.opt_dist(dest));
        dists2[dest]=*tree2.opt_dist(dest);
    }
    time2=timer.Pass<std::chrono::milliseconds>();
    for(size_t node=0;node<num_nodes;++node)
    {
        assert(is_equal(dists[node],dists2[node]));
    }
    std::cout<<"Dijkstra time,ms:"<<time<<"\nA* time,ms:"<<time2<<'\n';
    std::cout<<"test_euclidian complete\n";
}









