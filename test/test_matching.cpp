
#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "../adjacency_list_view.h"
#include "../breadth_search.h"
#include "../graph_generator.h"
#include "../matching/matching.h"

#include "test_suite.h"



namespace{

using namespace liga;

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
    auto target=[](auto iter){return *iter;};
    using view_t=adjacency_list_view_t<graph_t,decltype(target),size_t>;

    graph_t graph;
    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;
    default_index_map_t imap;
    {
        graph_test_suite_t suite;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"s:"<<test<<'\n';
            graph=to_adjacency_list(false,suite.edge_list(test));
            assert(is_bipartite(view_t(graph),0,imap)==suite.is_bipartite(test));
            if(is_bipartite(view_t(graph),0,imap))
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
        graph_generator_t::request_t request;
        request.num_nodes=10;
        request.num_edges=18;
        request.type=graph_generator_t::bipartite_id;
        assert(request.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"---graph---:"<<i<<'\n';
            bool is_gen=graph_gen.make_graph(graph,request);
            assert(is_gen);
            assert(is_bipartite(view_t(graph),0,imap));
            size_t card=tracksol.max_cardinality(graph);
            assert(bsol.max_cardinality(graph)==card);
            assert(nonbsol.max_cardinality(graph)==card);
        }
        request.type=graph_generator_t::undirected_graph_id;
        assert(request.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"---graph---:"<<i<<'\n';
            bool is_gen=graph_gen.make_graph(graph,request);
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
    auto target=[](auto iter){return iter->first;};
    using view_t=adjacency_list_view_t<w_graph_t,decltype(target),size_t>;

    w_graph_t             graph;
    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;
    default_index_map_t imap;
    {
        graph_test_suite_t    suite;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"---test---:"<<test<<'\n';
            graph=to_weighted_adjacency_list<int>(false,suite.edge_list(test));
            if(is_bipartite(view_t(graph),0,imap))
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
        graph_generator_t::request_t request;
        request.num_nodes=11;
        request.num_edges=20;
        request.type=graph_generator_t::bipartite_id;;
        assert(request.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,request,{1,100});
            //out(g);
            assert(is_gen);
            assert(is_bipartite(view_t(graph),0,imap));
            size_t wht=tracksol.max_weighted(graph);
            auto perfect_wht=tracksol.perfect_max_weighted(graph);

            assert(bsol.max_weighted(graph)==wht);
            assert(bsol.perfect_max_weighted(graph)==perfect_wht);

            assert(nonbsol.max_weighted(graph)==wht);
            assert(nonbsol.perfect_max_weighted(graph)==perfect_wht);
        }
        request.type=graph_generator_t::undirected_graph_id;
        assert(request.is_valid());
        for(int i=0;i<1000;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,request,{1,100});
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
    auto target=[](auto iter){return iter->first;};
    using view_t=adjacency_list_view_t<w_graph_t,decltype(target),size_t>;

    w_graph_t             graph;
    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;
    default_index_map_t imap;
    {
        graph_test_suite_t    suite;
        for(size_t test=0;test<suite.size();++test)
        {
            //std::cout<<"---test---:"<<test<<'\n';
            graph=to_weighted_adjacency_list<int>(false,suite.edge_list(test));
            if(is_bipartite(view_t(graph),0,imap))
            {
                assert(bsol.min_covering(graph)==(int)suite.min_weight_covering(test));
            }
            //assert(nonbsol.min_covering(graph)==(int)suite.min_weight_covering(test));
            assert(tracksol.min_covering(graph)==(int)suite.min_weight_covering(test));
        }
    }
    {
        graph_generator_t graph_gen;
        graph_generator_t::request_t request;
        request.num_nodes=11;
        request.num_edges=20;
        request.type=graph_generator_t::bipartite_id;
        assert(request.is_valid());
        for(int i=0;i<100;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,request,{1,100});
            //out(g);
            assert(is_gen);
            assert(is_bipartite(view_t(graph),0,imap));
            int wht=tracksol.min_covering(graph);

            assert(bsol.min_covering(graph)==wht);
            //assert(nonbsol.min_covering(graph)==wht);
        }
        request.type=graph_generator_t::undirected_graph_id;
        assert(request.is_valid());
        for(int i=0;i<100;++i)
        {
            //std::cout<<"*****graph*****:"<<i<<'\n';
            bool is_gen=graph_gen.make_wgraph(graph,request,{1,100});
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









