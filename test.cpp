#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <string>
#include <assert.h>

#include "graph_concepts.h"
#include "graph_traits.h"
#include "breadth_search.h"
#include "graph_utility.h"
#include "euler_tour.h"
#include "depth_search.h"
#include "articulation_point.h"
#include "graph_connectivity.h"
#include "shortest_path.h"
#include "min_spanning_tree.h"
#include "flows.h"
#include "graph_generator.h"
#include "test_suite.h"

namespace
{

/*
template<class value_t>
auto get_range(value_t b,value_t e)
{
    using size_t=std::size_t;
    struct range_t
    {
        value_t m_begin,m_end;
        range_t begin()const;
        range_t end()const;
        void operator++()
        void operator==(const range_t&r)const{return }
    }
    range(b,e);

    return range;
}
*/

struct random_graph_suite_t
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<size_t>>;
    using wgraph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    size_t size;
    size_t num_nodes;
    size_t average_out_edges;
    std::pair<size_t,size_t> weight_range;
    std::vector<wgraph_t>undirected;
    std::vector<graph_t> bipartite;
    std::vector<wgraph_t>dag;
    std::vector<graph_t> tree;

    random_graph_suite_t(size_t s,size_t n,size_t av_out_edges,std::pair<size_t,size_t> wr):
    size(s),num_nodes(n),average_out_edges(av_out_edges),weight_range(wr)
    {
        bipartite.resize(size,{});
        tree.resize(size,{});
        undirected.resize(size,{});
        dag.resize(size,{});

        graph_generator_t  graph_gen;
        for(std::size_t i=0;i<size;++i)
        {
            graph_gen.make_wgraph<std::size_t>(undirected[i],graph_chars(graph_class_t::undirected_graph_id),weight_range);
            graph_gen.make_wgraph<std::size_t>(dag[i],graph_chars(graph_class_t::dag_id),weight_range);
            graph_gen.make_graph(bipartite[i],graph_chars(graph_class_t::bipartite_id));
            graph_gen.make_graph(tree[i],graph_chars(graph_class_t::undirected_tree_id));
        }
    }
    graph_generator_t::graph_chars_t graph_chars(graph_class_t class_)const
    {
        auto in_range=[](size_t s,std::pair<size_t,size_t> range)
        {
            if(s<range.first) return range.first;
            if(s>range.second) return range.second;
            return s;
        };
        graph_generator_t::graph_chars_t res;
        res.g_class=class_;
        res.num_nodes=num_nodes;
        res.num_edges=in_range(num_nodes*average_out_edges,edge_range(num_nodes,class_));
        assert(res.is_valid());
        return res;
    }
}
gl_graph_suite(50,100,10,{1,100});

template<class edge_list_t,class graph_t>
std::size_t edge_list_to_graph(const edge_list_t&edges,graph_t&graph,bool is_direct)
{
    using size_t=std::size_t;
    graph.clear();
    size_t num_nodes=0;
    for(auto edge:edges)
    {
        size_t from=std::get<0>(edge);
        size_t to=std::get<1>(edge);
        if(num_nodes=std::max({from+1,to+1,num_nodes});num_nodes>graph.size())
        {
            graph.resize(num_nodes,{});
        }
        if constexpr(w_vector_list_cpt<graph_t>)
        {
            graph[from].push_back({to,std::get<2>(edge)});
            if(!is_direct) graph[to].push_back({from,std::get<2>(edge)});
        }
        else
        {
            graph[from].push_back(to);
            if(!is_direct)  graph[to].push_back(from);
        }
    }
    return num_nodes;
}


void test_concepts()
{
    using vvi_t=std::vector<std::vector<int>>;
    using vvd_t=std::vector<std::vector<double>>;
    using vvii_t=std::vector<std::vector<std::pair<int,int>>>;
    using vvid_t=std::vector<std::vector<std::pair<int,double>>>;

    using vli_t=std::vector<std::list<int>>;
    using vld_t=std::vector<std::list<double>>;
    using vlii_t=std::vector<std::list<std::pair<int,int>>>;
    using vlid_t=std::vector<std::list<std::pair<int,double>>>;

    struct valid_graph_t
    {
        using node_handler_t=int;
        using edge_handler_t=int;
        int edge_begin(int) {return 0;};
        bool edge_end(int,int) {return true;};
        int edge_inc(int,int&) {return 0;};
        int target(int,int){return true;}
        int index(int){return 0;}
    };

    struct valid_w_graph_t:public valid_graph_t
    {
        using weight_t=int;
        int weight(int,int){return 0;}
    };

    static_assert(graph_handler_cpt<valid_graph_t>);
    static_assert(graph_handler_cpt<valid_w_graph_t>);
    static_assert(!w_graph_handler_cpt<valid_graph_t>);
    static_assert(w_graph_handler_cpt<valid_w_graph_t>);

    static_assert(vector_list_cpt<vvi_t>);
    static_assert(vector_list_cpt<vvd_t>);
    static_assert(vector_list_cpt<vli_t>);
    static_assert(vector_list_cpt<vld_t>);

    static_assert(!w_vector_list_cpt<vvi_t>);
    static_assert(!w_vector_list_cpt<vvd_t>);
    static_assert(!w_vector_list_cpt<vli_t>);
    static_assert(!w_vector_list_cpt<vld_t>);

    static_assert(w_vector_list_cpt<vvii_t>);
    static_assert(w_vector_list_cpt<vvid_t>);
    static_assert(w_vector_list_cpt<vlii_t>);
    static_assert(w_vector_list_cpt<vlid_t>);

    std::cout<<"test_concepts complete\n";
}

void test_traits()
{
    using size_t=std::size_t;
    {
        std::vector<std::vector<size_t>> graph(4,{0,1,2,3});
        using gtr=graph_traits<decltype(graph)>;

        for(size_t n=0;n<graph.size();++n)
        {
            size_t t=0;
            for(auto e=gtr::edge_begin(graph,n);!gtr::edge_end(graph,n,e);gtr::edge_inc(graph,n,e),++t)
            {
                assert(gtr::target(graph,n,e)==t);
            }
        }
    }
    {
        std::vector<std::list<size_t>> graph(4,{0,1,2,3});
        using gtr=graph_traits<decltype(graph)>;
        for(uint32_t n=0;n<graph.size();++n)
        {
            size_t t=0;
            for(auto e=gtr::edge_begin(graph,n);!gtr::edge_end(graph,n,e);gtr::edge_inc(graph,n,e),++t)
            {
                assert(gtr::target(graph,n,e)==t);
            }
        }
    }
    {
        const double w=8.43;
        std::vector<std::vector<std::pair<size_t,double>>> graph(4,{{0,w},{1,w},{2,w},{3,w}});
        using gtr=graph_traits<decltype(graph)>;

        for(uint32_t n=0;n<graph.size();++n)
        {
            size_t t=0;
            for(auto e=gtr::edge_begin(graph,n);!gtr::edge_end(graph,n,e);gtr::edge_inc(graph,n,e),++t)
            {
                assert(gtr::target(graph,n,e)==t);
                assert(gtr::weight(graph,n,e)==w);
            }
        }
    }
    {
        const double w=8.43;
        std::vector<std::list<std::pair<int,double>>> graph(4,{{0,w},{1,w},{2,w},{3,w}});
        using gtr=graph_traits<decltype(graph)>;
        for(uint32_t n=0;n<graph.size();++n)
        {
            size_t t=0;
            for(auto e=gtr::edge_begin(graph,n);!gtr::edge_end(graph,n,e);gtr::edge_inc(graph,n,e),++t)
            {
                assert(gtr::target(graph,n,e)==t);
                assert(gtr::weight(graph,n,e)==w);
            }
        }
    }

    std::cout<<"test_traits complete\n";
}

void test_heap()
{
    std::vector<double> doubles;
    std::vector<int>    sorted;
    std::default_random_engine dre;
    std::uniform_real_distribution<double> distr(0,1.0);
    const int elements=1000;
    for(int i=0;i<elements;++i)
    {
        doubles.push_back(distr(dre));
    }
    auto comp=[&](int i,int j){return doubles[i]<doubles[j];};
    auto indx=[](int i){return i;};
    heap_priority_queue_t<int,decltype(comp),decltype(indx)> queue(comp,indx);
    for(int i=0;i<elements;++i)
    {
        queue.push(i);
    }
    sorted.push_back(queue.pop());
    assert(std::is_sorted(sorted.begin(),sorted.end(),comp));

    std::cout<<"test_heap complete\n";
}


void test_breadth_search()
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    {
        graph_t undir_graph,dir_graph;
        tree_search_t<graph_t> undir_tree,dir_tree;
        graph_test_suite_t suite;
        for(size_t test=0;test<suite.size();++test)
        {
            edge_list_to_graph(suite.edge_list(test),undir_graph,false);
            edge_list_to_graph(suite.edge_list(test),dir_graph,true);
            auto [n,e]=traverse_graph(false,undir_graph,0);
            assert(n==suite.nodes(test));
            assert(e==suite.edges(test));
            for(size_t source=0;source<suite.nodes(test);++source)
            {
                breadth_search(false,undir_graph,source,full_search_t<graph_t>{},undir_tree);
                breadth_search(true,dir_graph,source,full_search_t<graph_t>{},dir_tree);
                for(size_t dest=0;dest<suite.nodes(test);++dest)
                {
                    assert(undir_tree.opt_dist(dest)==suite.path_undir(test,source,dest));
                    assert(dir_tree.opt_dist(dest)==suite.path_dir(test,source,dest));
                }
                assert(is_bipartite(undir_graph,source)==suite.is_bipartite(test));
            }
        }
    }
    {
        for(size_t test=0;test<gl_graph_suite.size;++test)
        {
            auto [n,e]=traverse_graph(false,gl_graph_suite.undirected[test],0);
            auto g_char=gl_graph_suite.graph_chars(graph_class_t::undirected_graph_id);
            assert(n==g_char.num_nodes);
            assert(e==g_char.num_edges);

            auto [n2,e2]=traverse_graph(false,gl_graph_suite.tree[test],0);
            g_char=gl_graph_suite.graph_chars(graph_class_t::undirected_tree_id);
            assert(n2==g_char.num_nodes);
            assert(e2==g_char.num_edges);
            assert(is_bipartite(gl_graph_suite.tree[test],0));

            auto [n3,e3]=traverse_graph(false,gl_graph_suite.bipartite[test],0);
            g_char=gl_graph_suite.graph_chars(graph_class_t::bipartite_id);
            assert(n3==g_char.num_nodes);
            assert(e3==g_char.num_edges);
            assert(is_bipartite(gl_graph_suite.bipartite[test],0));
        }
    }

    std::cout<<"test_breadth_search complete\n";
}

void test_euler_tour()
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    graph_t undir_graph,dir_graph;
    graph_test_suite_t suite;
    for(size_t test=0;test<suite.size();++test)
    {
        //std::cout<<"test:"<<test<<'\n';
        edge_list_to_graph(suite.edge_list(test),undir_graph,false);
        edge_list_to_graph(suite.edge_list(test),dir_graph,true);
        std::vector<size_t> nodes(dir_graph.size());
        std::iota(nodes.begin(),nodes.end(),0);

        //std::cout<<"1\n";
        assert(has_direct_euler_tour(dir_graph,nodes)==suite.has_direct_euler_tour(test));
        //std::cout<<"2\n";
        assert(has_direct_euler_cycle(dir_graph,nodes)==suite.has_direct_euler_cycle(test));
        //std::cout<<"3\n";
        assert(has_undirect_euler_tour(undir_graph,nodes)==suite.has_undirect_euler_tour(test));
        //std::cout<<"4\n";
        assert(has_undirect_euler_cycle(undir_graph,nodes)==suite.has_undirect_euler_cycle(test));
        //std::cout<<"5\n";
        assert((!euler_tour(false,undir_graph,nodes).empty())==suite.has_undirect_euler_tour(test));
        //std::cout<<"6\n";
        assert((!euler_tour(true,dir_graph,nodes).empty())==suite.has_direct_euler_tour(test));
        //std::cout<<"7\n";
        assert((!euler_cycle(false,undir_graph,nodes).empty())==suite.has_undirect_euler_cycle(test));
        //std::cout<<"8\n";
        assert((!euler_cycle(true,dir_graph,nodes).empty())==suite.has_direct_euler_cycle(test));
    }
    std::cout<<"test_euler_tour complete\n";
}

void test_depth_search()
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    using edge_t=graph_traits<graph_t>::edge_handler_t;
    size_t trav_nodes,trav_edges;
    auto depth_search_traverse=full_search_t<graph_t>{}.set_node_preprocess([&](size_t s)
    {
        ++trav_nodes;
        return true;
    })
    .set_edge_process([&](size_t n,edge_t e)
    {
        ++trav_edges;
        return true;
    });
    {
        graph_t undir_graph,dir_graph;
        tree_search_t<graph_t,std::pair<size_t,size_t>> tree;
        graph_test_suite_t suite;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"test suite:"<<test<<'\n';
            edge_list_to_graph(suite.edge_list(test),undir_graph,false);
            edge_list_to_graph(suite.edge_list(test),dir_graph,true);
            const size_t num_nodes=undir_graph.size();
            std::vector<size_t> nodes(num_nodes);
            std::iota(nodes.begin(),nodes.end(),0);
            assert(is_dag(dir_graph,nodes)==suite.is_dag(test));
            for(size_t source=0;source<num_nodes;++source)
            {
                //std::cout<<"source suite:"<<source<<'\n';
                assert(is_undirected_tree(undir_graph,source)==suite.is_tree(test));
                trav_nodes=trav_edges=0;
                depth_search(false,undir_graph,source,depth_search_traverse,tree);
                assert(trav_nodes==num_nodes);
                assert(trav_edges==suite.edges(test));
            }
        }
    }
    {
        tree_search_t<graph_t> tree1,tree2;
        tree_search_t<graph_t,std::pair<size_t,size_t>> ds_tree;
        for(size_t test=0;test<gl_graph_suite.size;++test)
        {
            //std::cout<<"test random:"<<test<<'\n';
            std::vector<size_t> nodes(gl_graph_suite.num_nodes);
            std::iota(nodes.begin(),nodes.end(),0);
            assert(is_dag(gl_graph_suite.dag[test],nodes));
            for(size_t source=0;source<gl_graph_suite.num_nodes;++source)
            {
                //std::cout<<"source random:"<<source<<'\n';
                assert(is_undirected_tree(gl_graph_suite.tree[test],source));
                bool is_dag=dag_shortest_path(gl_graph_suite.dag[test],source,tree1);
                assert(is_dag);
                sparse_dijkstra(gl_graph_suite.dag[test],source,tree2);
                for(size_t dest=0;dest<gl_graph_suite.num_nodes;++dest)
                {
                    assert(tree1.opt_dist(dest)==tree2.opt_dist(dest));
                }
                trav_nodes=trav_edges=0;
                auto res=depth_search(false,gl_graph_suite.undirected[test],source,depth_search_traverse,ds_tree);
                assert(res==search_termination_t::complete_id);
                assert(trav_nodes==gl_graph_suite.num_nodes);
                assert(trav_edges==gl_graph_suite.graph_chars(graph_class_t::undirected_graph_id).num_edges);
            }
        }
    }
    std::cout<<"test_depth_search complete\n";
}

void test_articulation_point()
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    size_t points=0;
    size_t bridges=0;
    auto inc_points=[&](size_t p)
    {
        ++points;
    };
    auto inc_bridges=[&](auto n,auto e)
    {
        ++bridges;
    };

    graph_t undir_graph;
    graph_test_suite_t suite;
    for(size_t test=0;test<suite.size();++test)
    {
        //std::cout<<"test suite:"<<test<<'\n';
        edge_list_to_graph(suite.edge_list(test),undir_graph,false);
        for(size_t source=0;source<undir_graph.size();++source)
        {
            //std::cout<<"source:"<<source<<'\n';
            points=bridges=0;
            articulation_point(undir_graph,source,inc_points,inc_bridges);
            //std::cout<<"points,bridges:"<<points<<','<<bridges<<'\n';
            assert(suite.nart_points(test)==points);
            assert(suite.nbridges(test)==bridges);

        }
    }
    std::cout<<"test_articulation_point complete\n";
}

void test_connectivity()
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    graph_t graph;
    graph_test_suite_t suite;
    for(size_t test=0;test<suite.size();++test)
    {
        edge_list_to_graph(suite.edge_list(test),graph,true);
        std::vector<size_t> nodes(graph.size());
        std::iota(nodes.begin(),nodes.end(),0);
        assert(strongly_connected_components(graph,nodes)==suite.num_scc(test));
        assert(is_strongly_connected(graph,0,graph.size())==(suite.num_scc(test)==1));
    }
    std::cout<<"test_connectivity complete\n";
}

void test_shortest_path()
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    {
        graph_t undir_graph,dir_graph;
        tree_search_t<graph_t> undir_tree,dir_tree;
        graph_test_suite_t suite;
        for(size_t i=0;i<suite.size();++i)
        {
            //std::cout<<"i:"<<i<<'\n';
            edge_list_to_graph(suite.edge_list(i),undir_graph,false);
            edge_list_to_graph(suite.edge_list(i),dir_graph,true);
            const size_t num_nodes=undir_graph.size();
            for(size_t j=0;j<num_nodes;++j)
            {
                dense_dijkstra(undir_graph,j,undir_tree);
                dense_dijkstra(dir_graph,j,dir_tree);
                for(size_t k=0;k<num_nodes;++k)
                {
                    assert(undir_tree.opt_dist(k)==suite.weighted_path_undir(i,j,k));
                    assert(dir_tree.opt_dist(k)==suite.weighted_path_dir(i,j,k));

                }

                sparse_dijkstra(undir_graph,j,undir_tree);
                sparse_dijkstra(dir_graph,j,dir_tree);
                for(size_t k=0;k<num_nodes;++k)
                {
                    assert(undir_tree.opt_dist(k)==suite.weighted_path_undir(i,j,k));
                    assert(dir_tree.opt_dist(k)==suite.weighted_path_dir(i,j,k));
                }

                bool no_cycle=bellman_ford(num_nodes,undir_graph,j,undir_tree)&&
                              bellman_ford(num_nodes,dir_graph,j,dir_tree);
                assert(no_cycle);

                for(size_t k=0;k<num_nodes;++k)
                {
                    assert(undir_tree.opt_dist(k)==suite.weighted_path_undir(i,j,k));
                    assert(dir_tree.opt_dist(k)==suite.weighted_path_dir(i,j,k));
                }
            }

            std::vector<size_t> nodes(num_nodes);
            std::iota(nodes.begin(),nodes.end(),0);
            std::vector<tree_search_t<graph_t>> undir_trees,dir_trees;
            floyd_warshall(dir_graph,nodes,dir_trees);
            floyd_warshall(undir_graph,nodes,undir_trees);
            for(size_t j=0;j<num_nodes;++j)
            {
                for(size_t k=0;k<num_nodes;++k)
                {
                    assert(undir_trees[j].opt_dist(k)==suite.weighted_path_undir(i,j,k));
                    assert(dir_trees[j].opt_dist(k)==suite.weighted_path_dir(i,j,k));
                }
            }
        }
    }
    {
        // test random graphs
        tree_search_t<graph_t> tree{},tree1{},tree2{};

        for(size_t t=0;t<gl_graph_suite.size;++t)
        {
            for(size_t source=0;source<gl_graph_suite.num_nodes;++source)
            {
                dense_dijkstra(gl_graph_suite.undirected[t],source,tree);
                sparse_dijkstra(gl_graph_suite.undirected[t],source,tree1);
                bellman_ford(gl_graph_suite.num_nodes,gl_graph_suite.undirected[t],source,tree2);

                for(size_t dest=0;dest<gl_graph_suite.num_nodes;++dest)
                {
                    assert(tree.opt_dist(dest));
                    assert(tree1.opt_dist(dest));
                    assert(tree2.opt_dist(dest));

                    size_t min_dist=*tree.opt_dist(dest);
                    assert(min_dist==*tree1.opt_dist(dest));
                    assert(min_dist==*tree2.opt_dist(dest));
                }
            }
        }
    }
    std::cout<<"test_shortest_path complete\n";
}

void test_minimal_spanning_tree()
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;

    graph_t graph;
    tree_search_t<graph_t> tree;
    graph_test_suite_t suite;
    auto void_=[](auto node,auto edge){};

    for(size_t test=0;test<suite.size();++test)
    {
        //std::cout<<"s:"<<test<<'\n';
        edge_list_to_graph(suite.edge_list(test),graph,false);
        size_t size_case=suite.nodes(test);
        for(size_t source=0;source<size_case;++source)
        {
            assert(dense_prim(graph,source,tree).first==suite.mst_weight(test));
            assert(sparse_prim(graph,source,tree).first==suite.mst_weight(test));
            assert(kruskal(graph,source,void_).first==suite.mst_weight(test));
        }
    }
    {
        // test random graphs
        tree_search_t<graph_t> tree;
        for(size_t test=0;test<gl_graph_suite.size;++test)
        {
            auto [weight,nodes_tree]=dense_prim(gl_graph_suite.undirected[test],0,tree);
            assert(nodes_tree==gl_graph_suite.num_nodes);
            for(size_t source=0;source<gl_graph_suite.num_nodes;++source)
            {
                {
                    auto [w,n]=dense_prim(gl_graph_suite.undirected[test],source,tree);
                    assert(n==gl_graph_suite.num_nodes);
                    assert(w==weight);
                }
                {
                    auto [w,n]=sparse_prim(gl_graph_suite.undirected[test],source,tree);
                    assert(n==gl_graph_suite.num_nodes);
                    assert(w==weight);
                }
                {
                    auto [w,n]=kruskal(gl_graph_suite.undirected[test],source,void_);
                    assert(w==weight);
                    assert(n==gl_graph_suite.num_nodes);
                }
            }
        }
    }
    std::cout<<"test_minimal_spanning_tree complete\n";
}


void test_max_flows()
{
    using size_t=std::size_t;
    {
        graph_test_suite_t suite;
        networks_traits_t<direct_pipe_t<size_t>> dir_traits;
        networks_traits_t<bidirect_pipe_t<size_t>> bidir_traits;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"F:"<<test<<'\n';
            dir_traits.clear();bidir_traits.clear();
            for(auto [from,to,weight]:suite.edge_list(test))
            {
                bidir_traits.pipes.push_back({from,to,weight,weight});
                dir_traits.pipes.push_back({from,to,weight});
            }
            for(size_t source=0;source<suite.nodes(test);++source)
            {
                //std::cout<<"source:"<<source<<'\n';
                for(size_t sink=0;sink<suite.nodes(test);++sink)
                {
                    //std::cout<<"sink:"<<sink<<'\n';
                    if(source==sink) continue;
                    size_t undir_flow=suite.max_flow_undir(test,source,sink);
                    size_t dir_flow=suite.max_flow_dir(test,source,sink);

                    bidir_traits.set_source(source).set_sink(sink);
                    dir_traits.set_source(source).set_sink(sink);

                    assert(min_edges_augmentation(bidir_traits)==undir_flow);
                    assert(sparse_max_flow_augmentation(bidir_traits)==undir_flow);
                    assert(dense_max_flow_augmentation(bidir_traits)==undir_flow);
                    assert(preflow_push_relabel_to_front(bidir_traits)==undir_flow);
                    assert(preflow_push_fifo(bidir_traits)==undir_flow);
                    assert(preflow_push_highest_label(bidir_traits)==undir_flow);
                    assert(dinic_max_flow(bidir_traits)==undir_flow);

                    assert(min_edges_augmentation(dir_traits)==dir_flow);
                    assert(sparse_max_flow_augmentation(dir_traits)==dir_flow);
                    assert(dense_max_flow_augmentation(dir_traits)==dir_flow);
                    assert(preflow_push_relabel_to_front(dir_traits)==dir_flow);
                    assert(preflow_push_fifo(dir_traits)==dir_flow);
                    assert(preflow_push_highest_label(dir_traits)==dir_flow);
                    assert(dinic_max_flow(dir_traits)==dir_flow);
                }
            }
        }
    }
    {
        using graph_t=random_graph_suite_t::wgraph_t;
        networks_traits_t<bidirect_pipe_t<size_t>> traits;
        auto graph_to_capacities=full_search_t<graph_t>().set_edge_process([&](auto n,auto e)
        {
            traits.pipes.push_back({n,e->first,e->second,e->second});
            return true;
        });
        for(size_t test=0;test<std::min((size_t)10,gl_graph_suite.size);++test)
        {
            traits.clear();
            breadth_search(false,gl_graph_suite.undirected[test],0,graph_to_capacities);
            for(size_t source=0;source<std::min((size_t)10,gl_graph_suite.num_nodes);++source)
            {
                for(size_t sink=0;sink<std::min((size_t)10,gl_graph_suite.num_nodes);++sink)
                {
                    if(source==sink) continue;
                    traits.set_source(source).set_sink(sink);
                    size_t max_flow=min_edges_augmentation(traits);//2.3(100 V,1000 E)
                    //assert(max_flow>=1);
                    assert(sparse_max_flow_augmentation(traits)==max_flow);//2.2(100 V,1000 E)
                    assert(dense_max_flow_augmentation(traits)==max_flow);//2.2(100 V,1000 E)
                    assert(preflow_push_relabel_to_front(traits)==max_flow);//10(100 V,1000 E)
                    assert(preflow_push_fifo(traits)==max_flow);//5.2(100 V,1000 E)
                    assert(preflow_push_highest_label(traits)==max_flow);//5.3(100 V,1000 E)
                    assert(dinic_max_flow(traits)==max_flow);// <1 s (100 V,1000 E)
                }
            }
        }
    }
    std::cout<<"test_max_flows complete\n";
}

void test_min_cost_flows()
{
    networks_traits_t<costed_direct_pipe_t<size_t,size_t>> traits;
    traits.set_pipes({{0,1,1,2}}).set_source(0).set_sink(1);
    auto cost=sparse_min_cost_flow<size_t,size_t>(traits,1);
    assert(cost==2);

    traits.set_pipes({{0,1,1,2},{0,2,1,1},{2,1,1,3}});
    assert((sparse_min_cost_flow<size_t,size_t>(traits,2)==6));
    assert((sparse_min_cost_flow<size_t,size_t>(traits,1)==2));

    traits.set_pipes({{0,1,2,6},{0,2,1,1},{2,1,1,3}});
    assert((sparse_min_cost_flow<size_t,size_t>(traits,1)==4));
    assert((sparse_min_cost_flow<size_t,size_t>(traits,2)==10));
    assert((sparse_min_cost_flow<size_t,size_t>(traits,3)==16));

    std::cout<<"test_min_cost_flows complete\n";
}

}


void gtl_test()
{
    test_concepts();
    test_traits();
    test_heap();
    test_breadth_search();
    test_euler_tour();
    test_depth_search();
    test_articulation_point();
    test_connectivity();
    test_shortest_path();
    test_minimal_spanning_tree();
    test_max_flows();
    test_min_cost_flows();
}




















