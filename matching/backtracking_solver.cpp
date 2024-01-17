#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "../graph_utility.h"
#include "matching.h"

namespace liga{

namespace {// utility

template<class vec_t,class int_t>
void swap_remove(vec_t&vec,int_t i)
{
    std::swap(vec[i],vec.back());
    vec.pop_back();
}

}// utility


/******************************************************************************
*******************************************************************************
                        backtracking_solver_t
*******************************************************************************
******************************************************************************/
namespace matching{


std::size_t backtracking_solver_t::max_cardinality(const graph_t&graph,std::vector<size_t>&match)
{
    const char in_match=2;
    const char out_of_match=1;
    const char out_of_stack=0;
    std::vector<size_t> cur_match;
    for(size_t i=0;i<graph.size();++i) cur_match.push_back(i);
    size_t cur_num_match=0;
    size_t num_match_record=0;

    auto set_edge=[&](const std::pair<size_t,size_t>&edge)
    {
        size_t from=edge.first;
        size_t to=graph[from][edge.second];
        if(cur_match[from]!=from||cur_match[to]!=to) return false;
        cur_match[from]=to;
        cur_match[to]=from;
        ++cur_num_match;
        if(cur_num_match>num_match_record)
        {
            num_match_record=cur_num_match;
            match=cur_match;
        }
        return true;
    };
    auto remove_edge=[&](const std::pair<size_t,size_t>&edge)
    {
        size_t from=edge.first;
        size_t to=graph[from][edge.second];
        if(cur_match[from]==to)
        {
            assert(cur_match[to]==from);
            cur_match[from]=from;
            cur_match[to]=to;
            --cur_num_match;
        }
    };

    std::vector<std::pair<size_t,size_t>> edges;
    for(size_t i=0;i<graph.size();++i)
    {
        for(size_t j=0;j<graph[i].size();++j)
        {
            if(i<graph[i][j]) edges.push_back({i,j});
        }
    }

    std::vector<std::pair<size_t,char>> stack;
    stack.push_back({0,in_match});
    while(!stack.empty())
    {
        size_t last=stack.size()-1;
        auto [edge,state]=stack.back();
        if(state==in_match)
        {
            if(set_edge(edges[edge])&&edge<edges.size()-1)
            {
                stack.push_back({edge+1,in_match});
            }
            stack[last].second=out_of_match;
            if(cur_num_match==graph.size()/2) break;
        }
        else if(state==out_of_match)
        {
            remove_edge(edges[edge]);
            if(edge<edges.size()-1)
            {
                stack.push_back({edge+1,in_match});
            }
            stack[last].second=out_of_stack;
        }
        else stack.pop_back();
    }
    assert(is_valid_matching(graph,match));
    return num_match_record;
}

std::size_t backtracking_solver_t::max_cardinality(const graph_t&graph)
{
    std::vector<size_t> match;
    return max_cardinality(graph,match);
}


std::optional<int> backtracking_solver_t::m_max_weighted(const w_graph_t&graph,std::vector<size_t>&match,size_t min_match)
{
    const char in_match=2;
    const char out_of_match=1;
    const char out_of_stack=0;
    std::vector<size_t> cur_match;
    for(size_t i=0;i<graph.size();++i) cur_match.push_back(i);
    size_t cur_num_match=0;
    int cur_weight=0;
    std::optional<int> weight_record;

    auto set_edge=[&](const std::pair<size_t,size_t>&edge)
    {
        size_t from=edge.first;
        size_t to=graph[from][edge.second].first;
        if(cur_match[from]!=from||cur_match[to]!=to) return false;
        cur_match[from]=to;
        cur_match[to]=from;
        ++cur_num_match;
        cur_weight+=graph[from][edge.second].second;
        if(cur_num_match>=min_match&&(!weight_record||cur_weight>*weight_record))
        {
            weight_record=cur_weight;
            match=cur_match;
        }
        return true;
    };
    auto remove_edge=[&](const std::pair<size_t,size_t>&edge)
    {
        size_t from=edge.first;
        size_t to=graph[from][edge.second].first;
        if(cur_match[from]==to)
        {
            assert(cur_match[to]==from);
            cur_match[from]=from;
            cur_match[to]=to;
            --cur_num_match;
            cur_weight-=graph[from][edge.second].second;
        }
    };

    std::vector<std::pair<size_t,size_t>> edges;
    for(size_t i=0;i<graph.size();++i)
    {
        for(size_t j=0;j<graph[i].size();++j)
        {
            if(i<graph[i][j].first) edges.push_back({i,j});
        }
    }

    std::vector<std::pair<size_t,char>> stack;
    stack.push_back({0,in_match});
    while(!stack.empty())
    {
        size_t last=stack.size()-1;
        auto [edge,state]=stack.back();
        if(state==in_match)
        {
            if(set_edge(edges[edge])&&edge<edges.size()-1)
            {
                stack.push_back({edge+1,in_match});
            }
            stack[last].second=out_of_match;
        }
        else if(state==out_of_match)
        {
            remove_edge(edges[edge]);
            if(edge<edges.size()-1)
            {
                stack.push_back({edge+1,in_match});
            }
            stack[last].second=out_of_stack;
        }
        else stack.pop_back();
    }
    assert(!weight_record||is_valid_matching(graph,match));
    return weight_record;
}

int backtracking_solver_t::max_weighted(const w_graph_t&graph)
{
    std::vector<size_t> match;
    auto res=m_max_weighted(graph,match,0);
    assert(res);
    return *res;
}

int backtracking_solver_t::max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    return *m_max_weighted(graph,match,0);
}

std::optional<int> backtracking_solver_t::perfect_max_weighted(const w_graph_t&graph)
{
    if(graph.size()%2) return {};
    std::vector<size_t> match;
    return m_max_weighted(graph,match,graph.size()/2);
}

std::optional<int> backtracking_solver_t::perfect_max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    if(graph.size()%2) return {};
    return m_max_weighted(graph,match,graph.size()/2);
}

int backtracking_solver_t::min_covering(const w_graph_t&graph,graph_t&cover)
{
    const char in_match=2;
    const char out_of_match=1;
    const char out_of_stack=0;
    graph_t cur_cover(graph.size(),std::vector<size_t>{});
    int cur_weight=0;
    std::optional<int> weight_record;
    size_t num_matched=0;

    auto set_edge=[&](const std::pair<size_t,size_t>&edge)
    {
        size_t from=edge.first;
        size_t to=graph[from][edge.second].first;
        assert(!find_edge(cur_cover,from,to)&&!find_edge(cur_cover,to,from));
        cur_cover[from].push_back(to);
        cur_cover[to].push_back(from);
        if(cur_cover[from].size()==1) ++num_matched;
        if(cur_cover[to].size()==1)   ++num_matched;
        assert(num_matched<=graph.size());
        cur_weight+=graph[from][edge.second].second;
        if(num_matched==graph.size()&&(!weight_record||cur_weight<*weight_record))
        {
            cover=cur_cover;
            weight_record=cur_weight;
        }
    };
    auto remove_edge=[&](const std::pair<size_t,size_t>&edge)
    {
        size_t from=edge.first;
        size_t to=graph[from][edge.second].first;
        if(auto from_to=find_edge(cur_cover,from,to);from_to)
        {
            auto to_from=find_edge(cur_cover,to,from);
            assert(to_from);
            swap_remove(cur_cover[from],*from_to-cur_cover[from].begin());
            swap_remove(cur_cover[to],*to_from-cur_cover[to].begin());
            if(cur_cover[from].empty()) --num_matched;
            if(cur_cover[to].empty())   --num_matched;
            cur_weight-=graph[from][edge.second].second;
        }
    };

    std::vector<std::pair<size_t,size_t>> edges;
    for(size_t i=0;i<graph.size();++i)
    {
        for(size_t j=0;j<graph[i].size();++j)
        {
            if(i<graph[i][j].first) edges.push_back({i,j});
        }
    }

    std::vector<std::pair<size_t,char>> stack;
    stack.push_back({0,in_match});
    while(!stack.empty())
    {
        size_t last=stack.size()-1;
        auto [edge,state]=stack.back();
        if(state==in_match)
        {
            set_edge(edges[edge]);
            if(edge<edges.size()-1) stack.push_back({edge+1,in_match});
            stack[last].second=out_of_match;
        }
        else if(state==out_of_match)
        {
            remove_edge(edges[edge]);
            if(edge<edges.size()-1) stack.push_back({edge+1,in_match});
            stack[last].second=out_of_stack;
        }
        else stack.pop_back();
    }
    assert(weight_record);
    return *weight_record;
}

int backtracking_solver_t::min_covering(const w_graph_t&graph)
{
    graph_t cover;
    return min_covering(graph,cover);
}

}//matching

}// liga
