
#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "../breadth_search.h"
#include "../graph_generator.h"
#include "../test_suite.h"

#include "matching.h"


namespace{

using namespace liga;

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
/*
bool is_valid(const matching::bipartite_solver_t::graph_t &graph)
{
    using namespace matching;
    for(std::size_t from=0;from<graph.size();from++)
    {
        for(std::size_t to:graph[from])
        {
            if(from==to||to>=graph.size()||!find_edge(graph,to,from)) return false;
        }
    }
    return true;
}


bool is_valid(const matching::bipartite_solver_t::w_graph_t&graph)
{
    using namespace matching;
    for(std::size_t from=0;from<graph.size();from++)
    {
        for(auto [to,we]:graph[from])
        {
            if(from==to||to>=graph.size()) return false;
            if(auto e=find_edge(graph,to,from);!e||(*e)->second!=we) return false;
        }
    }
    return true;
}

void out(const matching::bipartite_solver_t::graph_t&g)
{
    std::cout<<"---------------\n";
    for(std::size_t from=0;from<g.size();++from)
    {
        for(std::size_t to:g[from])
        {
            std::cout<<g[from][to]<<',';
        }
        std::cout<<'\n';
    }
    std::cout<<"---------------\n";
}

void out(const matching::bipartite_solver_t::w_graph_t&g)
{
    std::cout<<"---------------\n";
    for(std::size_t from=0;from<g.size();++from)
    {
        for(auto [to,w]:g[from])
        {
            std::cout<<'['<<to<<','<<w<<']'<<',';
        }
        std::cout<<'\n';
    }
    std::cout<<"---------------\n";
}
*/

void test_cardinality_matching()
{
    using namespace matching;
    using size_t=std::size_t;
    using graph_t=bipartite_solver_t::graph_t;
    graph_t graph;
    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;

    {
        graph_test_suite_t suite;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"s:"<<test<<'\n';
            edge_list_to_graph(suite.edge_list(test),graph,false);
            assert(is_bipartite(graph,0)==suite.is_bipartite(test));
            if(is_bipartite(graph,0))
            {

                assert(bsol.max_cardinality(graph)==suite.max_cardinality_matching(test));
            }
            assert(nonbsol.max_cardinality(graph)==suite.max_cardinality_matching(test));
            assert(tracksol.max_cardinality(graph)==suite.max_cardinality_matching(test));
        }
    }
    {
        graph_generator_t graph_gen;
        graph_t graph;
        graph_generator_t::graph_chars_t graph_chars;
        graph_chars.num_nodes=10;
        graph_chars.num_edges=18;
        graph_chars.g_class=graph_class_t::bipartite_id;
        assert(graph_chars.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"---graph---:"<<i<<'\n';
            bool is_gen=graph_gen.make_graph(graph,graph_chars);
            assert(is_gen);
            assert(is_bipartite(graph,0));
            size_t card=tracksol.max_cardinality(graph);
            assert(bsol.max_cardinality(graph)==card);
            assert(nonbsol.max_cardinality(graph)==card);
        }
        graph_chars.g_class=graph_class_t::undirected_graph_id;
        assert(graph_chars.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"---graph---:"<<i<<'\n';
            bool is_gen=graph_gen.make_graph(graph,graph_chars);
            assert(is_gen);
            assert(nonbsol.max_cardinality(graph)==tracksol.max_cardinality(graph));
        }
    }

    std::cout<<"test_cardinality_matching complete\n";
}

void test_weighted_matching()
{
    using namespace matching;
    using size_t=std::size_t;
    using w_graph_t=bipartite_solver_t::w_graph_t;

    w_graph_t             graph;
    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;
    {
        graph_test_suite_t    suite;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"---test---:"<<test<<'\n';
            edge_list_to_graph(suite.edge_list(test),graph,false);
            if(is_bipartite(graph,0))
            {
                assert(bsol.perfect_max_weighted(graph)==suite.max_perf_weight_matching(test));
                assert(bsol.max_weighted(graph)==suite.max_weight_matching(test));
            }

            assert(nonbsol.perfect_max_weighted(graph)==suite.max_perf_weight_matching(test));
            assert(nonbsol.max_weighted(graph)==suite.max_weight_matching(test));
            assert(tracksol.perfect_max_weighted(graph)==suite.max_perf_weight_matching(test));
            assert(tracksol.max_weighted(graph)==suite.max_weight_matching(test));
        }
    }
    {
        graph_generator_t graph_gen;
        graph_generator_t::graph_chars_t graph_chars;
        graph_chars.num_nodes=11;
        graph_chars.num_edges=20;
        graph_chars.g_class=graph_class_t::bipartite_id;;
        assert(graph_chars.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,graph_chars,{1,100});
            //out(g);
            assert(is_gen);
            assert(is_bipartite(graph,0));
            size_t wht=tracksol.max_weighted(graph);
            auto perfect_wht=tracksol.perfect_max_weighted(graph);

            assert(bsol.max_weighted(graph)==wht);
            assert(bsol.perfect_max_weighted(graph)==perfect_wht);

            assert(nonbsol.max_weighted(graph)==wht);
            assert(nonbsol.perfect_max_weighted(graph)==perfect_wht);
        }
        graph_chars.g_class=graph_class_t::undirected_graph_id;
        assert(graph_chars.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,graph_chars,{1,100});
            //out(g);
            assert(is_gen);
            size_t wht=tracksol.max_weighted(graph);
            auto perfect_wht=tracksol.perfect_max_weighted(graph);

            assert(nonbsol.max_weighted(graph)==wht);
            assert(nonbsol.perfect_max_weighted(graph)==perfect_wht);
        }
    }
    std::cout<<"test_weighted_matching complete\n";
}

void test_min_covering()
{
    using namespace matching;
    using size_t=std::size_t;
    using w_graph_t=bipartite_solver_t::w_graph_t;

    w_graph_t             graph;
    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;
    {
        graph_test_suite_t    suite;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"---test---:"<<test<<'\n';
            edge_list_to_graph(suite.edge_list(test),graph,false);
            if(is_bipartite(graph,0))
            {
                assert(bsol.min_covering(graph)==(int)suite.min_weight_covering(test));
            }
            //assert(nonbsol.min_covering(graph)==(int)suite.min_weight_covering(test));
            assert(tracksol.min_covering(graph)==(int)suite.min_weight_covering(test));
        }
    }
    {
        graph_generator_t graph_gen;
        graph_generator_t::graph_chars_t graph_chars;
        graph_chars.num_nodes=11;
        graph_chars.num_edges=20;
        graph_chars.g_class=graph_class_t::bipartite_id;
        assert(graph_chars.is_valid());
        for(int i=0;i<100;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,graph_chars,{1,100});
            //out(g);
            assert(is_gen);
            assert(is_bipartite(graph,0));
            int wht=tracksol.min_covering(graph);

            assert(bsol.min_covering(graph)==wht);
            //assert(nonbsol.min_covering(graph)==wht);
        }
        graph_chars.g_class=graph_class_t::undirected_graph_id;
        assert(graph_chars.is_valid());
        for(int i=0;i<100;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,graph_chars,{1,100});
            //out(g);
            assert(is_gen);
            //assert(nonbsol.min_covering(graph)==tracksol.min_covering(graph));
        }
    }
    std::cout<<"test_min_covering complete\n";
}

}// namespace

void test_matching()
{
    test_cardinality_matching();
    test_weighted_matching();
    test_min_covering();
}









