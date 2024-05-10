#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "../graph_concepts.h"
#include "../adjacency_list_view.h"
#include "../graph.h"
#include "../breadth_search.h"
#include "../tree_search.h"
#include "../dijkstra.h"
#include "../search_settings.h"
#include "../graph_generator.h"
#include "../rand_utility.h"

#include "test_suite.h"


namespace {

using namespace liga;

struct random_graph_suite_t
{
    using size_t=std::size_t;
    using adjacency_list_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    struct target_t
    {
        size_t operator()(typename std::vector<std::pair<size_t,size_t>>::const_iterator iter)const
        {
            return iter->first;
        }
    };
    using adjacency_list_view_t=liga::adjacency_list_view_t<adjacency_list_t,target_t,size_t>;
    size_t size;
    size_t num_nodes;
    size_t average_out_edges;
    std::vector<adjacency_list_t> undirected;
    std::vector<adjacency_list_t> directed;
    static void mapped(const std::vector<std::vector<size_t>>&from,graph_generator_t::request_t req,adjacency_list_t&to)
    {
        to.resize(from.size());
        std::vector<size_t> shuffle(req.num_edges);
        std::iota(shuffle.begin(),shuffle.end(),0);
        std::random_shuffle(shuffle.begin(),shuffle.end());
        size_t e=0;
        for(size_t source=0;source<from.size();++source)
        {
            for(size_t t=0;t<from[source].size();++t)
            {
                size_t target=from[source][t];
                if(source>target&&req.type==graph_generator_t::undirected_graph_id) continue;
                assert(e<req.num_edges);
                to[source].push_back({target,shuffle[e]});
                if(req.type==graph_generator_t::undirected_graph_id)
                {
                    to[target].push_back({source,shuffle[e]});
                }
                ++e;
            }
        }
        assert(e==req.num_edges);
    }

    random_graph_suite_t(size_t s,size_t n,size_t av_out_edges):
    size(s),num_nodes(n),average_out_edges(av_out_edges)
    {
        undirected.resize(size,{});
        directed.resize(size,{});
        graph_generator_t gen;
        graph_generator_t::graph_t generated;
        for(std::size_t i=0;i<size;++i)
        {
            graph_generator_t::request_t req=request(graph_generator_t::undirected_graph_id);
            gen.make_graph(generated,req);
            mapped(generated,req,undirected[i]);

            req=request(graph_generator_t::directed_graph_id);
            gen.make_graph(generated,req);
            mapped(generated,req,directed[i]);
        }
    }
    graph_generator_t::request_t request(graph_generator_t::graph_type_t type)const
    {
        auto in_range=[](size_t s,std::pair<size_t,size_t> range)
        {
            if(s<range.first) return range.first;
            if(s>range.second) return range.second;
            return s;
        };
        graph_generator_t::request_t res;
        res.type=type;
        res.num_nodes=num_nodes;
        res.num_edges=in_range(num_nodes*average_out_edges,graph_generator_t::edge_range(num_nodes,type));
        assert(res.is_valid());
        return res;
    }
}
gl_graph_suite(50,100,10);


void sort_adjacency_list(random_graph_suite_t::adjacency_list_t&list)
{
    for(auto&alist:list)
    {
        std::sort(alist.begin(),alist.end(),[](auto&pair1,auto&pair2)
        {
            return pair1.second<pair2.second;
        });
    }
}
/*
void out(const random_graph_suite_t::graph_t&graph)
{
    for(const auto&list:graph)
    {
        for(auto [node,edge]:list)
        {
            std::cout<<'['<<node<<','<<edge<<']';
        }
        std::cout<<'\n';
    }
}
*/
void test_concepts()
{
    using namespace liga;
    using node_t=graph_definitions::base_node_t;
    using edge_t=graph_definitions::base_edge_t<node_t>;

    using dview_t=graph_t<node_t,edge_t,graph_chars_t::directed>::adjacency_list_view_t;
    using uview_t=graph_t<node_t,edge_t,graph_chars_t::undirected>::adjacency_list_view_t;
    using mview_t=graph_t<node_t,edge_t,graph_chars_t::mixed>::adjacency_list_view_t;

    static_assert(liga::graph_cpt<dview_t>);
    static_assert(liga::graph_cpt<uview_t>);
    static_assert(liga::graph_cpt<mview_t>);
}

void test_undirected_graph(random_graph_suite_t::adjacency_list_t&list)
{
    using namespace liga;
    using size_t=std::size_t;
    using node_t=graph_definitions::base_node_t;
    struct edge_t:public graph_definitions::base_edge_t<node_t>
    {
        size_t edge_index;
        edge_t(size_t s):edge_index(s){}
    };
    using graph_t=graph_t<node_t,edge_t,graph_chars_t::undirected>;
    using list_t=random_graph_suite_t::adjacency_list_t;
    auto deleter=[](auto*ptr){ delete ptr; };
    sort_adjacency_list(list);
    graph_t graph;
    graph.add_node(new node_t);
    auto from_list_to_graph=full_search_t().set_edge_process([&](size_t source,auto edge)
    {
        size_t target=edge->first;
        while(std::max(source,target)>=graph.nodes().size())
        {
            graph.add_node(new node_t);
        }
        node_t* source_graph=graph.nodes()[source];
        node_t* target_graph=graph.nodes()[target];
        graph.check_consistency();
        bool added=graph.add_edge(new edge_t(edge->second),target_graph,source_graph);
        assert(added);
        graph.check_consistency();
        return true;
    });
    breadth_search(false,
                   random_graph_suite_t::adjacency_list_view_t(list),
                   0,
                   from_list_to_graph,
                   default_index_map_t());
    graph.check_consistency();
    list_t copy;
    auto from_graph_to_list=full_search_t().set_edge_process([&](node_t*source,auto edge_it)
    {
        node_t* target=(edge_it->first);
        edge_t* edge=  edge_it->second;
        if(size_t max=std::max(target->index(),source->index());max>=copy.size())
        {
            copy.resize(max+1);
        }
        copy[source->index()].push_back({target->index(),edge->edge_index});
        copy[target->index()].push_back({source->index(),edge->edge_index});
        return true;
    });
    breadth_search(false,
                   graph_t::adjacency_list_view_t(graph.undirected_adj_list()),
                   graph.nodes()[0],
                   from_graph_to_list,
                   default_index_map_t());
    sort_adjacency_list(copy);
    assert(copy==list);
    graph.clear(deleter,deleter);
    graph.check_consistency();
}

void test_directed_graph(random_graph_suite_t::adjacency_list_t&list)
{
    using namespace liga;
    using size_t=std::size_t;
    using node_t=graph_definitions::base_node_t;
    struct edge_t:public graph_definitions::base_edge_t<node_t>
    {
        size_t edge_index;
        edge_t(size_t s):edge_index(s){}
    };
    using graph_t=graph_t<node_t,edge_t,graph_chars_t::directed|graph_chars_t::has_multiedges>;
    using list_t=random_graph_suite_t::adjacency_list_t;
    auto deleter=[](auto*ptr){ delete ptr; };
    sort_adjacency_list(list);
    graph_t graph;
    graph.add_node(new node_t);
    auto from_list_to_graph=full_search_t().set_edge_process([&](size_t source,auto edge)
    {
        size_t target=edge->first;
        while(std::max(source,target)>=graph.nodes().size())
        {
            graph.add_node(new node_t);
        }
        node_t* source_graph=graph.nodes()[source];
        node_t* target_graph=graph.nodes()[target];
        graph.check_consistency();
        bool added=graph.add_edge(new edge_t(edge->second),source_graph,target_graph);
        assert(added);
        graph.check_consistency();
        return true;
    });
    breadth_search(true,
                   random_graph_suite_t::adjacency_list_view_t(list),
                   0,
                   from_list_to_graph,
                   default_index_map_t());
    graph.check_consistency();
    list_t copy;
    auto from_graph_to_list=full_search_t().set_edge_process([&](node_t*source,auto edge_it)
    {
        node_t* target=edge_it->first;
        edge_t* edge=  edge_it->second;
        if(size_t max=std::max(target->index(),source->index());max>=copy.size())
        {
            copy.resize(max+1);
        }
        copy[source->index()].push_back({target->index(),edge->edge_index});
        return true;
    });
    breadth_search(true,
                   graph_t::adjacency_list_view_t(graph.out_adj_list()),
                   graph.nodes()[0],
                   from_graph_to_list,default_index_map_t());
    sort_adjacency_list(copy);
    assert(copy==list);
    graph.clear(deleter,deleter);
}

void test_suite()
{
    using namespace liga;
    using size_t=std::size_t;
    using node_t=graph_definitions::base_node_t;
    struct edge_t:public graph_definitions::base_edge_t<node_t>
    {
        size_t length;
        edge_t(size_t len):length(len){}
    };
    using graph_t=graph_t<node_t,edge_t,graph_chars_t::directed>;
    using adj_list_t=graph_t::adjacency_list_t;
    using adj_list_view_t=graph_t::adjacency_list_view_t;
    using tree_t=tree_search_t<typename adj_list_view_t::node_handler_t,
                               typename adj_list_view_t::edge_handler_t,
                               size_t,
                               default_index_map_t>;
    graph_test_suite_t   suite;
    graph_t              graph;
    auto weight_map=[](typename adj_list_view_t::node_handler_t node,
                       typename adj_list_view_t::edge_handler_t edge)
    {
        return edge->second->length;
    };
    auto deleter=[](auto*ptr){ delete ptr; };

    for(size_t test=0;test<suite.num_all_pairs_tests();++test)
    {
        auto&elist=suite.edge_list(test);
        for(size_t n=0;n<suite.nodes(test);++n)
        {
            node_t* node=new node_t;
            graph.add_node(node);
        }
        graph.check_consistency();
        for(size_t e=0;e<elist.size();++e)
        {
            auto [source,target,length]=elist[e];
            edge_t* edge=new edge_t(length);
            graph.add_edge(edge,graph.nodes()[source],graph.nodes()[target]);
        }
        graph.check_consistency();
        adj_list_t undir_list=graph_t::join(graph.in_adj_list(),graph.out_adj_list());
        tree_t  dir_tree,transp_tree,undirected_tree;
        for(size_t source_ind=0;source_ind<graph.nodes().size();++source_ind)
        {
            auto source=graph.nodes()[source_ind];
            dense_dijkstra(adj_list_view_t(graph.out_adj_list()),source,full_search_t{},dir_tree,weight_map);
            dense_dijkstra(adj_list_view_t(graph.in_adj_list()),source,full_search_t{},transp_tree,weight_map);
            dense_dijkstra(adj_list_view_t(undir_list),source,full_search_t{},undirected_tree,weight_map);
            for(size_t targ_ind=0;targ_ind<graph.nodes().size();++targ_ind)
            {
                auto target=graph.nodes()[targ_ind];
                assert(dir_tree.opt_dist(target)==suite.weighted_path_dir(test,source_ind,targ_ind));
                assert(transp_tree.opt_dist(target)==suite.weighted_path_dir(test,targ_ind,source_ind));
                assert(undirected_tree.opt_dist(target));
                assert(undirected_tree.opt_dist(target)==suite.weighted_path_undir(test,targ_ind,source_ind));
            }
        }
        graph.clear(deleter,deleter);
        graph.check_consistency();
    }

}

}

void test_graph()
{
    test_concepts();
    for(size_t s=0;s<gl_graph_suite.size;++s)
    {
        //std::cout<<s<<'\n';
        test_undirected_graph(gl_graph_suite.undirected[s]);
        test_directed_graph(gl_graph_suite.directed[s]);
    }
    test_suite();
    std::cout<<"test_graph complete\n";
}









