
#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "../breadth_search.h"
#include "matching.h"
#include "../graph_generator.h"

namespace{

bool is_valid(const matching::bipartite_solver_t::graph_t &graph)
{
    using namespace matching;
    for(std::size_t from=0;from<graph.size();from++)
    {
        for(std::size_t to:graph[from])
        {
            if(from==to||to>=graph.size()||!find_edge(to,from,graph)) return false;
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
            if(auto e=find_edge(to,from,graph);!e||graph[to][*e].second!=we) return false;
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

}

void test_cardinality_matching()
{
    using namespace matching;
    using size_t=std::size_t;
    using graph_t=bipartite_solver_t::graph_t;
    std::vector<graph_t> graphs=
    {
        /**
            0-1
        */
        {
            {1},
            {0},
        },
        /**
            0-1
            \ /
             2
        */
        {
            {1,2},
            {0,2},
            {0,1}
        },
        /**   2_3
            0<  |
              1_4
        */
        {
            {1,2},
            {0,4},
            {0,3},
            {2,4},
            {1,3}
        },
        /**     3
            0-1<|
                2
        */
        {
            {1},
            {0,2,3},
            {1,3},
            {2,1}
        },
        /**   2_3
            0<| |
              1_4
        */
        {
            {1,2},
            {0,2,4},
            {0,1,3},
            {2,4},
            {1,3}
        },
        /**
              5
              |
              4_1
            3< X|
              2_0
        */
        {
            {1,2,4},
            {0,2,4},
            {0,1,3},
            {2,4},
            {0,1,3,5},
            {4}
        },
        /**
              1_4_5
            0<|/|
              2_3
        */
        {
            {1,2},
            {0,2,4},
            {4,0,1,3},
            {2,4},
            {2,1,3,5},
            {4}
        },
        /**
             0
             |
             1
            / \
         4_2 _ 3_5

        */
        {
            {1},
            {2,3,0},
            {3,1,4},
            {2,1,5},
            {2},
            {3}
        },
        /**
              2
            /_|_\
            0 1 3
            \   /
             5-4
        */
        {
            {1,5,2},
            {2,3,0},
            {3,1,0},
            {2,1,4},
            {3,5},
            {0,4}
        },
        /**
              7
              |
              6
              ^
             0 5
            /|_|\
           1_2 3_4
        */
        {
            {6,2,1},
            {2,0},
            {0,1,3},
            {5,4,2},
            {3,5},
            {3,4,6},
            {0,5,7},
            {6}
        },
    };
    std::vector<size_t> matches={1,1,2,2,2,3,3,3,3,4};

    nonbipartite_solver_t nonbsol;
    bipartite_solver_t    bsol;
    backtracking_solver_t tracksol;

    for(size_t i=0;i<graphs.size();++i)
    {
        //std::cout<<"---graph---:"<<i<<'\n';
        auto g=graphs[i];
        assert(is_valid(g));
        for(int j=0;j<10;++j)
        {
            if(is_bipartite(g,0))
            {
                assert(bsol.max_cardinality(g)==matches[i]);
            }
            assert(nonbsol.max_cardinality(g)==matches[i]);
            assert(tracksol.max_cardinality(g)==matches[i]);
        }
    }

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
        assert(is_valid(graph));
        assert(nonbsol.max_cardinality(graph)==tracksol.max_cardinality(graph));
    }
    std::cout<<"test_cardinality_matching complete\n";
}

void test_weighted_matching()
{
    using namespace matching;
    using size_t=std::size_t;
    using w_graph_t=bipartite_solver_t::w_graph_t;
    std::vector<w_graph_t> graphs=
    {
        /**
            0-(2)-1
            |     |
            (1)  (1)
            |     |
            2-(1)-3
        */
        {
            {{1,2},{2,1}},
            {{0,2},{3,1}},
            {{0,1},{3,1}},
            {{2,1},{1,1}}
        },
        /**
            0-(1)-1-(3)-2-(1)-3
        */
        {
            {{1,1}},
            {{0,1},{2,3}},
            {{1,3},{3,1}},
            {{2,1}}
        },
        /**
                    0
                   / \
                  (1) (1)
                2/_(2)__\ 1
        */
        {
            {{1,1},{2,1}},
            {{0,1},{2,2}},
            {{0,1},{1,2}}
        },
        /**
                    0
                   / \
                  (1) (1)
                2/_(3)__\ 1_(1)_3
        */
        {
            {{1,1},{2,1}},
            {{0,1},{2,3},{3,1}},
            {{0,1},{1,3}},
            {{1,1}}
        },
        /**
                 4-(5)-3
                /|     |\
              (9)|     |(4)
              /  |     |  \
             5  (5)   (8)  2
              \  |     |  /
              (2)|     |(7)
               \ |     |/
                 0-(1)-1

        */
        {
            {{1,1},{5,2},{4,5}},
            {{0,1},{2,7},{3,8}},
            {{1,7},{3,4}},
            {{1,8},{2,4},{4,5}},
            {{3,5},{0,5},{5,9}},
            {{0,2},{4,9}},
        }
    };
    std::vector<std::pair<int,int>> matches={{3,3},{2,3},{0,2},{2,3},{14,17}};
    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;

    for(size_t i=0;i<graphs.size();++i)
    {
        //std::cout<<"---graph---:"<<i<<'\n';
        auto g=graphs[i];
        assert(is_valid(g));

        for(size_t j=0;j<10;++j)
        {
            assert(is_valid(g));
            if(is_bipartite(g,0))
            {
                assert(*bsol.perfect_max_weighted(g)==matches[i].first);
                assert(bsol.max_weighted(g)==matches[i].second);
            }
            //std::cout<<"nonbsol.perfect_max_weighted(g)...\n";
            assert(nonbsol.perfect_max_weighted(g).value_or(0)==matches[i].first);
            //std::cout<<"nonbsol.max_weighted(g)...\n";
            assert(nonbsol.max_weighted(g)==matches[i].second);
            //std::cout<<"tracksol.perfect_max_weighted(g)...\n";
            assert(tracksol.perfect_max_weighted(g).value_or(0)==matches[i].first);
            //std::cout<<"tracksol.max_weighted(g)...\n";
            assert(tracksol.max_weighted(g)==matches[i].second);
        }
    }

    graph_generator_t graph_gen;
    w_graph_t g;
    graph_generator_t::graph_chars_t graph_chars;
    graph_chars.num_nodes=11;
    graph_chars.num_edges=20;
    graph_chars.g_class=graph_class_t::bipartite_id;;
    assert(graph_chars.is_valid());
    for(int i=0;i<1000;++i)
    {
        //std::cout<<"*****graph*****:"<<i<<'\n';
        bool is_gen=graph_gen.make_wgraph(g,graph_chars,{1,100});
        //out(g);
        assert(is_gen);
        assert(is_valid(g));
        assert(nonbsol.max_weighted(g)==tracksol.max_weighted(g));
        assert(nonbsol.perfect_max_weighted(g)==tracksol.perfect_max_weighted(g));

    }
    std::cout<<"test_weighted_matching complete\n";
}

void test_min_covering()
{
    using namespace matching;
    using size_t=std::size_t;
    using w_graph_t=bipartite_solver_t::w_graph_t;
    std::vector<w_graph_t> graphs=
    {
        /**
            0-(2)-1
            |     |
            (1)  (1)
            |     |
            2-(1)-3
        */
        {
            {{1,2},{2,1}},
            {{0,2},{3,1}},
            {{0,1},{3,1}},
            {{2,1},{1,1}}
        },
        /**
            0-(1)-1-(3)-2-(1)-3
        */
        {
            {{1,1}},
            {{0,1},{2,3}},
            {{1,3},{3,1}},
            {{2,1}}
        },
        /**
            0-(2)-1-(2)-4
            |     |
            (1)  (2)
            |     |
            2-(1)-3
        */
        {
            {{1,2},{2,1}},
            {{0,2},{3,2},{4,2}},
            {{0,1},{3,1}},
            {{2,1},{1,2}},
            {{1,2}}
        },
        /**
            0-(1)-1-(1)-4
            |     |
            (2)  (2)
            |     |
            2-(1)-3
        */
        {
            {{1,1},{2,2}},
            {{0,1},{3,2},{4,1}},
            {{0,2},{3,1}},
            {{2,1},{1,2}},
            {{1,1}}
        },
    };
    std::vector<int> min_cov={2,2,4,3};

    bipartite_solver_t    bsol;
    nonbipartite_solver_t nonbsol;
    backtracking_solver_t tracksol;

    for(size_t i=0;i<graphs.size();++i)
    {
        auto&g=graphs[i];
        assert(is_valid(g));
        if(is_bipartite(g,0))
        {
            assert(bsol.min_covering(g)==min_cov[i]);
        }
        assert(tracksol.min_covering(g)==min_cov[i]);
        int check=nonbsol.min_covering(g);
        //std::cout<<"nonbsol.min_covering(g):"<<check<<'\n';
        assert(check==min_cov[i]);
    }

    graph_generator_t graph_gen;
    w_graph_t g;
    graph_generator_t::graph_chars_t graph_chars;
    graph_chars.num_nodes=8;
    graph_chars.num_edges=13;
    graph_chars.g_class=graph_class_t::bipartite_id;
    assert(graph_chars.is_valid());
    for(int i=0;i<1000;++i)
    {
        //std::cout<<"+++++graph+++++:"<<i<<'\n';
        bool is_gen=graph_gen.make_wgraph(g,graph_chars,{1,10});
        //out(g);
        assert(is_gen);
        assert(is_valid(g));
        assert(is_bipartite(g,0));
        int weight=tracksol.min_covering(g);
        assert(bsol.min_covering(g)==weight);
        assert(nonbsol.min_covering(g)==weight);
    }
    std::cout<<"test_min_covering complete\n";
}

void test_matching()
{
    test_cardinality_matching();
    test_weighted_matching();
    test_min_covering();
}









