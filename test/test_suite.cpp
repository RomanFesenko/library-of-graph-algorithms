#include <iostream>
#include <vector>
#include <assert.h>

#include "test_suite.h"

const static int _=-1;

const static int gl_nnode[]=   {2,3,4,4,4,4,4,5,5,5,5, 6,6,6,6,6,8};
const static int gl_nedge[]=   {1,3,3,4,3,4,5,5,5,6,6, 8,6,8,8,8,10};
const static int gl_mst[]=     {2,2,5,4,6,3,4,7,5,5,7, 17,11,10,8,9,11};
const static char gl_is_bip[]= {1,0,1,0,1,1,0,0,1,0,0, 0,0,0,0,0,0};
const static char gl_is_tree[]={1,0,1,0,1,0,0,0,0,0,0, 0,0,0,0,0,0};
const static char gl_is_dag[]= {1,1,1,0,1,0,0,1,1,0,0, 0,0,0,0,0,0};

const static int gl_ncomp[]=   {2,3,4,2,4,1,2,5,5,3,1, 2,4,4,2,1,4};

const static char gl_has_tour[]={1,0,1,1,0,0,1,0,1,0,1, 0,0,1,0,0,0,  // undirect case
                                 1,0,1,1,0,0,0,0,0,0,1, 0,0,0,0,0,0};// direct case

const static char gl_has_cycle[]={0,1,0,0,0,1,0,1,0,0,0, 0,0,0,0,0,0, // undirect case
                                  0,0,0,0,0,1,0,0,0,0,0, 0,0,0,0,0,0};// direct case

const static int gl_nart_point[]={0,0,2,1,1,0,0,0,1,1,0, 0,3,1,1,0,1};
const static int gl_nbridge[]=   {1,0,3,1,3,0,0,0,1,1,0, 0,3,1,1,0,1};
const static int gl_scc[]=       {2,3,4,2,4,1,2,5,5,3,1, 2,4,4,2,1,4};

const static int gl_maxcard[]=      {1,1,2,2,1,2,2,2,2,2,2, 3, 3, 3, 3, 3, 4};
const static int gl_maxweight[]=    {2,3,3,3,3,4,6,7,4,5,8, 17,10,10,15,14,16};
const static int gl_maxperfweight[]={2,_,2,3,_,4,6,_,_,_,_, 14,9, 8, 9, 13,6};
const static int gl_mincov[]=       {2,2,2,3,6,2,4,4,3,4,5, 12,9, 7, 5, 6, 6};

const static int gl_all_pairs_tests=11;




const static std::size_t gl_edges[]=
{
    // 0
    /**
          0-|2>-1
    */
    0,1,2,
    // 1
    /**
             1
            / \
          |1>  |1>
        0/__|3>__\2
    */
    0,1,1,
    0,2,3,
    1,2,1,
    //2
    /**
          0-|1>-1-|3>-2-|1>-3
    */
    0,1,1,
    1,2,3,
    2,3,1,
    // 3
    /**
            0
           / \
        <1|  <1|
       1/__|3>__\2_<2|_3
    */
    0,1,1,
    1,2,3,
    2,0,1,
    3,2,2,
    // 4
    /**
            2
             \
             <2|
         1_<1|_\0_<3|_3
    */
    0,1,1,
    0,2,2,
    3,0,3,
    // 5
    /**
        0-|3>-1
        |     |
       /0     1/
        |     |
        3-<1|-2
    */
    0,1,3,
    1,2,2,
    2,3,1,
    3,0,0,
    // 6
    /**
             2
            /|\
          |2>||1>
          /  |  \
         0   1/  3
          \  |  /
          <3|||4>
           \ | /
             1
    */
    1,0,3,
    0,2,2,
    2,1,1,
    2,3,1,
    1,3,4,
    //7
    /**
        0-|2>-1
        |      \
        1/      2
        |      /
        4-<4|-3
    */
    0,1,2,
    1,2,3,
    2,3,1,
    3,4,4,
    0,4,1,
    //8
    /**
        0-<1|-1-<1|-4
        |     |
       /2     2/
        |     |
        2-|1>-3
    */
    1,0,1,
    2,0,2,
    2,3,1,
    1,3,2,
    4,1,1,
    // 9
    /**
             2
            /|\
          |1>||0>
          /  |  \
         0   3\  3-<2|-4
          \  |  /
          |2>|<4|
           \ | /
             1
    */
    0,1,2,
    0,2,1,
    1,2,3,
    2,3,0,
    3,1,4,
    4,3,2,
    // 10
    /**
        0-|1>-2
        |     |\
        2\   /2 4
        |     |/
        1-<3|-3
    */
    0,2,1,
    1,0,2,
    3,1,3,
    3,2,2,
    2,4,5,
    4,3,2,
    //11
    /**
            4-|5>-3
           /|     |\
         <9||     ||4>
         /  |     |  \
        5   5\   /8  2
         \  |     |  /
         |2>|     |<7|
          \ |     |/
            0-|1>-1

    */
    0,1,1,
    2,1,7,
    1,3,8,
    3,2,4,
    0,4,5,
    4,3,5,
    5,0,2,
    4,5,9,
    //12
    /**
                 5
                 |
                 2/
                 |
                 4
                / \
              |1> |1>
       0-<3|-1/_<8|_\2-|4>-3
    */
    1,0,3,
    2,1,8,
    2,3,4,
    1,4,1,
    4,2,1,
    5,4,2,
    //13
    /**
            4-|5>-3-|2>-5
           /|    /|
          / |   / |
         0  8\<3| 2/
          \ |  /  |
          \ | /   |
            1-|4>-2

    */
    0,1,1,
    1,2,4,
    3,2,2,
    3,1,3,
    1,4,8,
    4,3,5,
    0,4,2,
    3,5,2,
    //14
    /**



            4-<2|-3-<1|-5
            |\   / \
            |   /   \
          \ 5 |6>    2
            | /     /
            |/   \ /
            0-|1>-1

    */
    0,1,1,
    1,2,3,
    2,3,1,
    3,4,2,
    4,0,5,
    0,3,6,
    4,1,9,
    5,3,1,
    //15
    /**
          3
        /_|_\
        4 5 2
        \   /
         0-1
    */
    0,1,2,
    1,2,8,
    2,3,5,
    4,3,6,
    4,0,3,
    2,5,1,
    3,5,2,
    5,4,1,
    //16
    /**
            0
            |
            1
            ^
           7 2
          /| |\
         6-5-4-3
    */
    0,1,1,
    1,2,1,
    2,3,1,
    3,4,2,
    5,4,3,
    6,5,9,
    7,6,1,
    1,7,2,
    5,7,4,
    4,2,5,
};


const static int gl_wef[]=
{
    /**Undirected: Distance-Edge-MaxFlow-MinCostOfMaxFlow between all pairs of nodes*/
    // 0
    0,2, 0,1, _,2, _,0,
    2,0, 1,0, 2,_, 0,_,
    // 1
    0,1,2, 0,1,1, _,2,4, _,3,5,
    1,0,1, 1,0,1, 2,_,2, 3,_,3,
    2,1,0, 1,1,0, 4,2,_, 5,3,_,
    //2
    0,1,4,5, 0,1,2,3, _,1,1,1, _,0,1,3,
    1,0,3,4, 1,0,1,2, 1,_,3,1, 0,_,3,3,
    4,3,0,1, 2,1,0,1, 1,3,_,1, 1,3,_,2,
    5,4,1,0, 3,2,1,0, 1,1,1,_, 3,3,2,_,
    // 3
    0,1,1,3, 0,1,1,2, _,2,2,2, _,3,3,9,
    1,0,2,4, 1,0,1,2, 2,_,4,2, 3,_,5,8,
    1,2,0,2, 1,1,0,1, 2,4,_,2, 3,5,_,6,
    3,4,2,0, 2,2,1,0, 2,2,2,_, 9,8,6,_,
    // 4
    0,1,2,3, 0,1,1,1, _,1,2,3, _,0,2,6,
    1,0,3,4, 1,0,2,2, 1,_,1,1, 0,_,1,2,
    2,3,0,5, 1,2,0,2, 2,1,_,2, 2,1,_,6,
    3,4,5,0, 1,2,2,0, 3,1,2,_, 6,2,6,_,
    // 5
    0,3,1,0, 0,1,2,1, _,3,2,1, _,0,2,3,
    3,0,2,3, 1,0,1,2, 3,_,2,1, 0,_,2,3,
    1,2,0,1, 2,1,0,1, 2,2,_,1, 2,2,_,2,
    0,3,1,0, 1,2,1,0, 1,1,1,_, 3,3,2,_,
    // 6
    0,3,2,3, 0,1,1,2, _,5,4,5, _, 11,11,23,
    3,0,1,2, 1,0,1,1, 5,_,4,5, 11,_, 11,20,
    2,1,0,1, 1,1,0,1, 4,4,_,4, 11,11,_, 19,
    3,2,1,0, 2,1,1,0, 5,5,4,_, 23,20,19,_,
    //7
    0,2,5,5,1, 0,1,2,2,1, _,3,3,2,2, _,10,11,10,10,
    2,0,3,4,3, 1,0,1,2,2, 3,_,4,2,2, 10,_,12,10,10,
    5,3,0,1,5, 2,1,0,1,2, 3,4,_,2,2, 11,12,_,10,10,
    5,4,1,0,4, 2,2,1,0,1, 2,2,2,_,5, 10,10,10,_,19,
    1,3,5,4,0, 1,2,2,1,0, 2,2,2,5,_, 10,10,10,19,_,
    //8
    0,1,2,3,2, 0,1,1,2,2, _,2,3,2,1, _,6,7,6,4,
    1,0,3,2,1, 1,0,2,1,1, 2,_,2,3,1, 6,_,6,9,4,
    2,3,0,1,4, 1,2,0,1,3, 3,2,_,2,1, 7,6,_,6,5,
    3,2,1,0,3, 2,1,1,0,2, 2,3,2,_,1, 6,9,6,_,7,
    2,1,4,3,0, 2,1,3,2,0, 1,1,1,1,_, 4,4,5,7,_,
    // 9
    0,2,1,1,3, 0,1,1,2,3, _,3,3,3,2, _, 3, 5, 15,18,
    2,0,3,3,5, 1,0,1,1,2, 3,_,4,4,2, 3, _, 7, 16,18,
    1,3,0,0,2, 1,1,0,1,2, 3,4,_,4,2, 5, 7, _, 23,21,
    1,3,0,0,2, 2,1,1,0,1, 3,4,4,_,2, 15,16,23,_, 10,
    3,5,2,2,0, 3,2,2,1,0, 2,2,2,2,_, 18,18,21,10,_,
    // 10
    0,2,1,3,5, 0,1,1,2,2, _,3,3,3,3, _, 7, 12,9, 20,
    2,0,3,3,5, 1,0,2,1,2, 3,_,4,4,4, 7, _, 22,10,28,
    1,3,0,2,4, 1,2,0,1,1, 3,4,_,5,7, 12,22,_, 27,36,
    3,3,2,0,2, 2,1,1,0,1, 3,4,5,_,5, 9, 10,27,_, 31,
    5,5,4,2,0, 2,2,1,1,0, 3,4,7,5,_, 20,28,36,31,_,

    /**Directed: Distance-Edge-MaxFlow-MinCostOfMaxFlow between all pairs of nodes*/
    // 0
    0,2, 0,1, _,2, _,0,
    _,0, _,0, 0,_, 0,_,
    // 1
    0,1,2, 0,1,1, _,1,4, _,0,5,
    _,0,1, _,0,1, 0,_,1, 0,_,2,
    _,_,0, _,_,0, 0,0,_, 0,0,_,
    //2
    0,1,4,5, 0,1,2,3, _,1,1,1, _,0,1,3,
    _,0,3,4, _,0,1,2, 0,_,3,1, 0,_,3,3,
    _,_,0,1, _,_,0,1, 0,0,_,1, 0,0,_,2,
    _,_,_,0, _,_,_,0, 0,0,0,_, 0,0,0,_,
    // 3
    0,1,4,_, 0,1,2,_, _,1,1,0, _,0,1,0,
    4,0,3,_, 2,0,1,_, 1,_,3,0, 3,_,3,0,
    1,2,0,_, 1,2,0,_, 1,1,_,0, 2,2,_,0,
    3,4,2,0, 2,3,1,0, 1,1,2,_, 5,5,6,_,
    // 4
    0,1,2,_, 0,1,1,_, _,1,2,0, _,0,2,0,
    _,0,_,_, _,0,_,_, 0,_,0,0, 0,_,0,0,
    _,_,0,_, _,_,0,_, 0,0,_,0, 0,0,_,0,
    3,4,5,0, 1,2,2,0, 3,1,2,_, 6,2,6,_,
    // 5
    0,3,5,6, 0,1,2,3, _,3,2,1, _,0,2,3,
    3,0,2,3, 3,0,1,2, 0,_,2,1, 0,_,2,3,
    1,4,0,1, 2,3,0,1, 0,0,_,1, 0,0,_,2,
    0,3,5,0, 1,2,3,0, 0,0,0,_, 0,0,0,_,
    // 6
    0,3,2,3, 0,2,1,2, _,1,2,2, _,3,2,11,
    3,0,5,4, 1,0,2,1, 3,_,2,5, 0,_,2,20,
    4,1,0,1, 2,1,0,1, 1,1,_,2, 2,2,_, 9,
    _,_,_,0, _,_,_,0, 0,0,0,_, 0,0,0, _,
    //7
    0,2,5,6,1, 0,1,2,3,1, _,2,2,1,2, _,0,2,3,10,
    _,0,3,4,8, _,0,1,2,3, 0,_,3,1,1, 0,_,3,3,6,
    _,_,0,1,5, _,_,0,1,2, 0,0,_,1,1, 0,0,_,2,5,
    _,_,_,0,4, _,_,_,0,1, 0,0,0,_,4, 0,0,0,_,12,
    _,_,_,_,0, _,_,_,_,0, 0,0,0,0,_, 0,0,0,0,_,
    //8
    0,_,_,_,_,  0,_,_,_,_, _,0,0,0,0, _,0,0,0,0,
    1,0,_,2,_,  1,0,_,1,_, 1,_,0,2,0, 0,_,0,6,0,
    2,_,0,1,_,  1,_,0,1,_, 2,0,_,1,0, 2,0,_,2,0,
    _,_,_,0,_,  _,_,_,0,_, 0,0,0,_,0, 0,0,0,_,0,
    2,1,_,3,0,  2,1,_,2,0, 1,1,0,1,_, 4,4,0,7,_,
    // 9
    0,2,1,1,_, 0,1,1,2,_, _,2,3,0,0, _,0,5,0,0,
    _,0,3,3,_, _,0,1,2,_, 0,_,3,0,0, 0,_,6,0,0,
    _,4,0,0,_, _,2,0,1,_, 0,0,_,0,0, 0,0,_,0,0,
    _,4,7,0,_, _,1,2,0,_, 0,4,3,_,0, 0,16,18,_,0,
    _,6,9,2,0, _,2,3,1,0, 0,2,2,2,_, 0,18,22,10,_,
    // 10
    0,11,1,8, 6, 0,4,1,3,2, _,1,1,1,1, _,11, 0, 9,4,
    2, 0,3,10,8, 1,0,2,4,3, 2,_,1,1,1, 2, _, 1,10,5,
    12,10,0,7,5, 4,3,0,2,1, 2,2,_,2,5, 24,22,_,18,20,
    5,  3,2,0,7, 2,1,1,0,2, 2,3,3,_,3,  6, 6,9, _,21,
    7,  5,4,2,0, 3,2,2,1,0, 2,2,2,2,_, 16,14,16,10,_,
};

graph_test_suite_t::graph_test_suite_t()
{
    m_size=std::size(gl_nnode);
    m_delta.push_back(0);
    for(size_t s=1;s<gl_all_pairs_tests;++s)
    {
        m_delta.push_back(m_delta[s-1]+4*gl_nnode[s-1]*gl_nnode[s-1]);
    }
    m_total_wef=m_delta.back()+4*gl_nnode[gl_all_pairs_tests-1]*gl_nnode[gl_all_pairs_tests-1];

    m_graphs.resize(m_size);
    size_t indx=0;
    for(size_t s=0;s<m_size;++s)
    {
        for(int edge=0;edge<gl_nedge[s];++edge)
        {
            m_graphs[s].push_back({gl_edges[indx],gl_edges[indx+1],gl_edges[indx+2]});
            indx+=3;
        }
    }
}


std::size_t graph_test_suite_t::m_block_size(size_t graph)const
{
    return gl_nnode[graph]*gl_nnode[graph]*4;
}

std::size_t graph_test_suite_t::m_block_width(size_t graph)const
{
     return gl_nnode[graph]*4;
}

std::size_t graph_test_suite_t::size()const
{
    return m_size;
}

const graph_test_suite_t::edge_list_t& graph_test_suite_t::edge_list(size_t g)const
{
    return m_graphs[g];
}

std::size_t graph_test_suite_t::nodes(size_t g)
{
    return gl_nnode[g];
}

std::size_t graph_test_suite_t::edges(size_t g)
{
    return gl_nedge[g];
}

// Undirected graphs

std::size_t graph_test_suite_t::weighted_path_undir(size_t g,size_t from,size_t to)const
{
    return gl_wef[m_delta[g]+from*m_block_width(g)+to];
}

std::size_t graph_test_suite_t::path_undir(size_t g,size_t from,size_t to)const
{
    return gl_wef[m_delta[g]+from*m_block_width(g)+gl_nnode[g]+to];
}

std::size_t graph_test_suite_t::max_flow_undir(size_t g,size_t from,size_t to)const
{
    assert(from!=to);
    return gl_wef[m_delta[g]+from*m_block_width(g)+2*gl_nnode[g]+to];
}

std::size_t graph_test_suite_t::min_cost_undir(size_t g,size_t from,size_t to)const
{
    assert(from!=to);
    return gl_wef[m_delta[g]+from*m_block_width(g)+3*gl_nnode[g]+to];
}

std::size_t graph_test_suite_t::mst_weight(size_t g)const
{
    return gl_mst[g];
}

bool graph_test_suite_t::is_tree(size_t g)const
{
    return gl_is_tree[g];
}

bool graph_test_suite_t::is_bipartite(size_t g)const
{
    return gl_is_bip[g];
}

// Directed graphs

std::optional<std::size_t> graph_test_suite_t::weighted_path_dir(size_t g,size_t from,size_t to)const
{
    int res=gl_wef[m_total_wef+m_delta[g]+from*m_block_width(g)+to];
    if(res!=_)
    {
        return res;
    }
    else
    {
        return {};
    }
}

std::optional<std::size_t> graph_test_suite_t::path_dir(size_t g,size_t from,size_t to)const
{
    int res=gl_wef[m_total_wef+m_delta[g]+from*m_block_width(g)+gl_nnode[g]+to];
    if(res!=_)
    {
        return res;
    }
    else
    {
        return {};
    }
}

std::size_t graph_test_suite_t::max_flow_dir(size_t g,size_t from,size_t to)const
{
    assert(from!=to);
    return gl_wef[m_total_wef+m_delta[g]+from*m_block_width(g)+2*gl_nnode[g]+to];
}

std::size_t graph_test_suite_t::min_cost_dir(size_t g,size_t from,size_t to)const
{
    assert(from!=to);
    return gl_wef[m_total_wef+m_delta[g]+from*m_block_width(g)+3*gl_nnode[g]+to];
}

std::size_t graph_test_suite_t::components(size_t g)const
{
    return gl_ncomp[g];
}

bool graph_test_suite_t::is_dag(size_t g)const
{
    return gl_is_dag[g];
}

bool graph_test_suite_t::has_undirected_euler_tour(size_t g)const
{
    return gl_has_tour[g];
}

bool graph_test_suite_t::has_undirected_euler_cycle(size_t g)const
{
    return gl_has_cycle[g];
}

bool graph_test_suite_t::has_directed_euler_tour(size_t g)const
{
    return gl_has_tour[g+m_size];
}

bool graph_test_suite_t::has_directed_euler_cycle(size_t g)const
{
    return gl_has_cycle[g+m_size];
}

std::size_t graph_test_suite_t::nart_points(size_t g)const
{
    return gl_nart_point[g];
}

std::size_t graph_test_suite_t::nbridges(size_t g)const
{
    return gl_nbridge[g];
}

std::size_t graph_test_suite_t::num_scc(size_t g)const
{
    return gl_scc[g];
}

size_t graph_test_suite_t::max_cardinality_matching(size_t g)const
{
    return gl_maxcard[g];
}

std::size_t graph_test_suite_t::max_weight_matching(size_t g)const
{
    return gl_maxweight[g];
}

std::optional<std::size_t> graph_test_suite_t::max_perf_weight_matching(size_t g)const
{
    return gl_maxperfweight[g]!=_? gl_maxperfweight[g]: std::optional<size_t>{} ;
}

std::size_t graph_test_suite_t::min_weight_covering(size_t g)const
{
    return gl_mincov[g];
}

std::size_t graph_test_suite_t::num_all_pairs_tests()const
{
    return gl_all_pairs_tests;
}


std::vector<std::vector<std::size_t>>
to_adjacency_list(bool is_direct,const std::vector<std::tuple<std::size_t,std::size_t,std::size_t>>&edges)
{
    std::vector<std::vector<std::size_t>> graph;
    for(auto [source,target,weight]:edges)
    {
        if(std::size_t num_nodes=std::max({source+1,target+1,graph.size()});num_nodes>graph.size())
        {
            graph.resize(num_nodes,{});
        }
        graph[source].push_back(target);
        if(!is_direct) graph[target].push_back(source);
    }
    return graph;
}

std::vector<std::vector<std::pair<std::size_t,std::size_t>>>
to_indexed_adjacency_list(bool is_direct,const std::vector<std::tuple<std::size_t,std::size_t,std::size_t>>&edges)
{
    using size_t=std::size_t;
    std::vector<std::vector<std::pair<size_t,size_t>>> graph;
    for(size_t index=0;index<edges.size();++index)
    {
        auto [source,target,weight]=edges[index];
        if(size_t num_nodes=std::max({source+1,target+1,graph.size()});num_nodes>graph.size())
        {
            graph.resize(num_nodes,{});
        }
        graph[source].push_back({target,index});
        if(!is_direct) graph[target].push_back({source,index});
    }
    return graph;
}

