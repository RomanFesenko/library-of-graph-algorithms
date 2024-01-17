#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <string>
#include <assert.h>
#include <math.h>

#include "../rand_utility.h"
#include "../graph_concepts.h"
#include "../graph_traits.h"
#include "../breadth_search.h"
#include "../graph_utility.h"
#include "../euler_tour.h"
#include "../depth_search.h"
#include "../articulation_point.h"
#include "../graph_connectivity.h"
#include "../dijkstra.h"
#include "../bellman_ford.h"
#include "../floyd_warshall.h"
#include "../min_spanning_tree.h"
#include "../graph_generator.h"

namespace{

using namespace liga;

template<class edge_list_t,class graph_t>
void edge_list_to_graph(std::size_t nodes,const edge_list_t&edges,graph_t&graph,bool is_direct)
{
    using size_t=std::size_t;
    graph.clear();
    graph.resize(nodes);
    for(auto&edge:edges)
    {
        size_t from=edge[0];
        size_t to=edge[1];
        if constexpr(w_vector_list_cpt<graph_t>)
        {
            graph[from].push_back({to,edge[2]});
            if(!is_direct) graph[to].push_back({from,edge[2]});
        }
        else
        {
            graph[from].push_back(to);
            if(!is_direct)  graph[to].push_back(from);
        }
    }
}



/*
207 Course Shedule
Medium
There are a total of numCourses courses you have to take,labeled from 0 to numCourses - 1.
You are given an array prerequisites where prerequisites[i] = [ai, bi]
indicates that you must take course bi first if you want to take course ai.
For example, the pair [0, 1], indicates that to take course 0 you have to first take course 1.
Return true if you can finish all courses. Otherwise, return false.

Example 1:
Input: numCourses = 2, prerequisites = [[1,0]]
Output: true
Explanation: There are a total of 2 courses to take.
To take course 1 you should have finished course 0. So it is possible.

Example 2:
Input: numCourses = 2, prerequisites = [[1,0],[0,1]]
Output: false
Explanation: There are a total of 2 courses to take.
To take course 1 you should have finished course 0, and to take course 0 you
should also have finished course 1. So it is impossible.

Constraints:

1 <= numCourses <= 2000
0 <= prerequisites.length <= 5000
prerequisites[i].length == 2
0 <= ai, bi < numCourses
All the pairs prerequisites[i] are unique.
*/



class Solution_207
{
    using size_t=std::size_t;
    public:
    bool canFinish(int numCourses, std::vector<std::vector<int>>& prerequisites)
    {
        std::vector<size_t> nodes(numCourses);
        std::iota(nodes.begin(),nodes.end(),0);
        std::vector<std::vector<size_t>> graph;
        edge_list_to_graph(numCourses,prerequisites,graph,true);
        return is_dag(graph,nodes);
    }
};

void Test_207()
{
    Solution_207 sol;
    std::vector<std::vector<int>> prereq;
    prereq={{0,1}};
    assert(sol.canFinish(2,prereq));
    prereq={{0,1},{1,0}};
    assert(!sol.canFinish(2,prereq));
    std::cout<<"207 problem complete\n";
}

/*
210 Course Shedule 2
Medium
There are a total of numCourses courses you have to take,
labeled from 0 to numCourses - 1.
You are given an array prerequisites where prerequisites[i] = [ai, bi]
indicates that you must take course bi first if you want to take course ai.
For example, the pair [0, 1], indicates that to take course 0 you have to
first take course 1.
Return the ordering of courses you should take to finish all courses.
If there are many valid answers, return any of them. If it is impossible to
finish all courses, return an empty array.

Example 1:
Input: numCourses = 2, prerequisites = [[1,0]]
Output: [0,1]
Explanation: There are a total of 2 courses to take. To take course 1 you
should have finished course 0. So the correct course order is [0,1].

Example 2:
Input: numCourses = 4, prerequisites = [[1,0],[2,0],[3,1],[3,2]]
Output: [0,2,1,3]
Explanation: There are a total of 4 courses to take. To take course 3 you
should have finished both courses 1 and 2.
Both courses 1 and 2 should be taken after you finished course 0.
So one correct course order is [0,1,2,3]. Another correct ordering is [0,2,1,3].

Example 3:
Input: numCourses = 1, prerequisites = []
Output: [0]

Constraints:
1 <= numCourses <= 2000
0 <= prerequisites.length <= numCourses * (numCourses - 1)
prerequisites[i].length == 2
0 <= ai, bi < numCourses
ai != bi
All the pairs [ai, bi] are distinct.
*/


class Solution_210
{
    using size_t=std::size_t;
    public:
    std::vector<int> findOrder(int numCourses, std::vector<std::vector<int>>& prerequisites)
    {
        std::vector<size_t> nodes(numCourses);
        std::iota(nodes.begin(),nodes.end(),0);
        std::vector<std::vector<size_t>> graph;
        edge_list_to_graph(numCourses,prerequisites,graph,true);
        std::vector<int> sorted;
        if(!total_topological_sort(graph,nodes,[&](size_t s){sorted.push_back(s);})) return {};
        return sorted;
    }
};

void Test_210()
{
    Solution_210 sol;
    std::vector<std::vector<int>> edges;
    edges={{1,0}};
    assert(sol.findOrder(2,edges).size()==2);
    edges={{1,0},{2,0},{3,1},{3,2}};
    assert(sol.findOrder(4,edges).size()==4);
    edges={};
    assert(sol.findOrder(1,edges).size()==1);
    std::cout<<"210 problem complete\n";
}

/*
329 Longest Increasing path in matrix
Given an m x n integers matrix, return the length of the longest increasing path in matrix.
From each cell, you can either move in four directions: left, right, up, or down.
You may not move diagonally or move outside the boundary (i.e., wrap-around is not allowed).

Example 1:
Input: matrix = [[9,9,4],[6,6,8],[2,1,1]]
Output: 4
Explanation: The longest increasing path is [1, 2, 6, 9].

Example 2:
Input: matrix = [[3,4,5],[3,2,6],[2,2,1]]
Output: 4
Explanation: The longest increasing path is [3, 4, 5, 6]. Moving diagonally is not allowed.

Example 3:
Input: matrix = [[1]]
Output: 1

Constraints:
m == matrix.length
n == matrix[i].length
1 <= m, n <= 200
0 <= matrix[i][j] <= 2^31 - 1
*/

class Solution_329
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    public:
    int longestIncreasingPath(const std::vector<std::vector<int>>& matrix)
    {
        const size_t rows=matrix.size();
        const size_t cols=matrix[0].size();
        auto cell_to_indx=[&](size_t r,size_t c){return r*cols+c+1;};
        graph_t graph(rows*cols+1);
        for(size_t i=0;i<rows;++i)
        {
            for(size_t j=0;j<cols;++j)
            {
                size_t from=cell_to_indx(i,j);
                graph[0].push_back({from,1});
                if(i!=0&&matrix[i-1][j]<matrix[i][j])
                {
                    graph[from].push_back({cell_to_indx(i-1,j),1});
                }
                if(i!=rows-1&&matrix[i+1][j]<matrix[i][j])
                {
                    graph[from].push_back({cell_to_indx(i+1,j),1});
                }
                if(j!=0&&matrix[i][j-1]<matrix[i][j])
                {
                    graph[from].push_back({cell_to_indx(i,j-1),1});
                }
                if(j!=cols-1&&matrix[i][j+1]<matrix[i][j])
                {
                    graph[from].push_back({cell_to_indx(i,j+1),1});
                }

            }
        }
        tree_search_t<graph_t> tree;
        auto updater=minimize_distance_t<graph_t,std::greater<>>{};
        bool res=dag_priority_search(graph,(size_t)0,full_search_t<graph_t>{},tree,updater);
        assert(res);
        int max_dist=0;
        for(size_t node=1;node<graph.size();++node)
        {
            assert(tree.opt_dist(node));
            max_dist=std::max(max_dist,(int)*tree.opt_dist(node));
        }
        return max_dist;
    }
};

void Test_329()
{
    Solution_329 sol;
    assert(sol.longestIncreasingPath({{9,9,4},{6,6,8},{2,1,1}})==4);
    assert(sol.longestIncreasingPath({{3,4,5},{3,2,6},{2,2,1}})==4);
    assert(sol.longestIncreasingPath({{1}})==1);
    std::cout<<"329 problem complete\n";
}

/*
743 Network delay time
Medium
Companies
You are given a network of n nodes, labeled from 1 to n. You are also given times,
a list of travel times as directed edges times[i] = (ui, vi, wi), where ui is the source node,
vi is the target node, and wi is the time it takes for a signal to travel from source to target.
We will send a signal from a given node k.
Return the minimum time it takes for all the n nodes to receive the signal.
If it is impossible for all the n nodes to receive the signal, return -1.

Example 1:
Input: times = [[2,1,1],[2,3,1],[3,4,1]], n = 4, k = 2
Output: 2

Example 2:
Input: times = [[1,2,1]], n = 2, k = 1
Output: 1

Example 3:
Input: times = [[1,2,1]], n = 2, k = 2
Output: -1

Constraints:
1 <= k <= n <= 100
1 <= times.length <= 6000
times[i].length == 3
1 <= ui, vi <= n
ui != vi
0 <= wi <= 100
All the pairs (ui, vi) are unique. (i.e., no multiple edges.)
*/

class Solution_743
{
    using size_t=std::size_t;
    public:
    int networkDelayTime(const std::vector<std::vector<int>>& times, int n, int k)
    {
        using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
        graph_t graph;
        edge_list_to_graph(n+1,times,graph,true);
        tree_search_t<graph_t> tree;
        sparse_dijkstra(graph,k,full_search_t<graph_t>{},tree);
        int time=0;
        for(int node=1;node<=n;++node)
        {
            if(!tree.opt_dist(node)) return -1;
            time=std::max(time,(int)*tree.opt_dist(node));
        }
        return time;
    }
};

void Test_743()
{
    Solution_743 sol;
    assert(sol.networkDelayTime({{2,1,1},{2,3,1},{3,4,1}},4,2)==2);
    assert(sol.networkDelayTime({{1,2,1}},2,1)==1);
    assert(sol.networkDelayTime({{1,2,1}},2,2)==-1);
    std::cout<<"743 problem complete\n";
}


/*
882 Reacheable node in subdivided graph
You are given an undirected graph (the "original graph") with n nodes labeled from 0 to n-1.
You decide to subdivide each edge in the graph into a chain of nodes,
with the number of new nodes varying between each edge.
The graph is given as a 2D array of edges where edges[i] = [ui, vi, cnti]
indicates that there is an edge between nodes ui and vi in the original graph,
and cnti is the total number of new nodes that you will subdivide the edge into.
Note that cnti == 0 means you will not subdivide the edge.
To subdivide the edge [ui, vi], replace it with (cnti+1) new edges and cnti new nodes.
The new nodes are x1, x2,...,xcnti,and the new edges are
[ui, x1], [x1, x2], [x2, x3], ..., [xcnti-1, xcnti], [xcnti, vi].
In this new graph, you want to know how many nodes are reachable from the node 0,
where a node is reachable if the distance is maxMoves or less.
Given the original graph and maxMoves, return the number of nodes that are
reachable from node 0 in the new graph.

Example 1:
Input: edges = [[0,1,10],[0,2,1],[1,2,2]], maxMoves = 6, n = 3
Output: 13
Explanation: The edge subdivisions are shown in the image above.
The nodes that are reachable are highlighted in yellow.

Example 2:
Input: edges = [[0,1,4],[1,2,6],[0,2,8],[1,3,1]], maxMoves = 10, n = 4
Output: 23

Example 3:
Input: edges = [[1,2,4],[1,4,5],[1,3,1],[2,3,4],[3,4,5]], maxMoves = 17, n = 5
Output: 1
Explanation: Node 0 is disconnected from the rest of the graph, so only node 0 is reachable.

Constraints:
0 <= edges.length <= min(n * (n - 1) / 2, 10^4)
edges[i].length == 3
0 <= ui < vi < n
There are no multiple edges in the graph.
0 <= cnti <= 10^4
0 <= maxMoves <= 10^9
1 <=n <= 3000
*/

class Solution_882
{
    public:
    int reachableNodes(const std::vector<std::vector<int>>& edges, int maxMoves, int n)
    {
        using size_t=std::size_t;
        using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
        using gts=graph_traits<graph_t>;
        using node_t=gts::node_handler_t;
        using edge_t=gts::edge_handler_t;
        graph_t graph(n);
        for(auto&edge:edges)
        {
            size_t n1=edge[0];
            size_t n2=edge[1];
            size_t subdiv=edge[2];
            graph[n1].push_back({n2,subdiv+1});
            graph[n2].push_back({n1,subdiv+1});
        }
        tree_search_t<graph_t> tree;
        size_t reachable=0;
        auto searcher=full_search_t<graph_t>{}.set_edge_filter([&](node_t n,edge_t e)
        {
            return tree.dist(n)+gts::weight(graph,n,e)<=(size_t)maxMoves;
        })
        .set_node_preprocess([&](node_t )
        {
            ++reachable;
            return true;
        });
        dense_dijkstra(graph,0,searcher,tree);
        for(auto&edge:edges)
        {
            size_t n1=edge[0];
            size_t n2=edge[1];
            size_t subdiv=edge[2];
            size_t added=0;
            if(tree.in_tree(n1))
            {
                assert(tree.opt_dist(n1));
                added+=std::min(subdiv,maxMoves-*tree.opt_dist(n1));
            }
            if(tree.in_tree(n2))
            {
                assert(tree.opt_dist(n2));
                added+=std::min(subdiv,maxMoves-*tree.opt_dist(n2));
            }
            reachable+=std::min(added,subdiv);
        }
        return reachable;
    }
};


void Test_882()
{
    Solution_882 sol;
    assert(sol.reachableNodes({{0,1,10},{0,2,1},{1,2,2}},6,3)==13);
    assert(sol.reachableNodes({{0,1,4},{1,2,6},{0,2,8},{1,3,1}},10,4)==23);
    assert(sol.reachableNodes({{1,2,4},{1,4,5},{1,3,1},{2,3,4},{3,4,5}},17,5)==1);
    std::cout<<"882 problem complete\n";
}

/*
1632 Rank transform of matrix
Hard
Given an m x n matrix, return a new matrix answer where answer[row][col]
is the rank of matrix[row][col].
The rank is an integer that represents how large an element is compared to other elements.
It is calculated using the following rules:
The rank is an integer starting from 1.
If two elements p and q are in the same row or column, then:
If p < q then rank(p) < rank(q)
If p == q then rank(p) == rank(q)
If p > q then rank(p) > rank(q)
The rank should be as small as possible.
The test cases are generated so that answer is unique under the given rules.

Example 1:
Input: matrix = [[1,2],[3,4]]
Output: [[1,2],[2,3]]
Explanation:
The rank of matrix[0][0] is 1 because it is the smallest integer in its row and column.
The rank of matrix[0][1] is 2 because matrix[0][1] > matrix[0][0] and matrix[0][0] is rank 1.
The rank of matrix[1][0] is 2 because matrix[1][0] > matrix[0][0] and matrix[0][0] is rank 1.
The rank of matrix[1][1] is 3 because matrix[1][1] > matrix[0][1], matrix[1][1] > matrix[1][0], and both matrix[0][1] and matrix[1][0] are rank 2.

Example 2:
Input: matrix = [[7,7],[7,7]]
Output: [[1,1],[1,1]]

Example 3:
Input: matrix = [[20,-21,14],[-19,4,19],[22,-47,24],[-19,4,19]]
Output: [[4,2,3],[1,3,4],[5,1,6],[1,3,4]]

Constraints:
m == matrix.length
n == matrix[i].length
1 <= m, n <= 500
-10^9 <= matrix[row][col] <= 10^9
*/

class Solution_1632
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    public:
    std::vector<std::vector<int>> matrixRankTransform(const std::vector<std::vector<int>>& matrix)
    {
        const size_t rows=matrix.size();
        const size_t cols=matrix[0].size();
        auto cell_to_indx=[&](size_t r,size_t c){return r*cols+c+1;};
        auto indx_to_cell=[&](size_t indx)
        {
            --indx;
            return std::pair{indx/cols,indx%cols};
        };
        graph_t graph(rows*cols+1);
        for(size_t i=0;i<rows;++i)
        {
            for(size_t j=0;j<cols;++j)
            {
                size_t from=cell_to_indx(i,j);
                graph[0].push_back({from,1});
                for(size_t k=0;k<cols;++k)
                {
                    if(k==j) continue;
                    size_t to=cell_to_indx(i,k);
                    if(matrix[i][j]<matrix[i][k])
                    {
                        graph[from].push_back({to,1});
                    }
                    else if(matrix[i][j]==matrix[i][k])
                    {
                        graph[from].push_back({to,0});
                        graph[to].push_back({from,0});
                    }
                    else
                    {
                        graph[to].push_back({from,1});
                    }
                }
                for(size_t k=0;k<rows;++k)
                {
                    if(k==i) continue;
                    size_t to=cell_to_indx(k,j);
                    if(matrix[i][j]<matrix[k][j])
                    {
                        graph[from].push_back({to,1});
                    }
                    else if(matrix[i][j]==matrix[k][j])
                    {
                        graph[from].push_back({to,0});
                        graph[to].push_back({from,0});
                    }
                    else
                    {
                        graph[to].push_back({from,1});
                    }
                }
            }
        }
        tree_search_t<graph_t> tree;
        auto updater=minimize_distance_t<graph_t,std::greater<>>{};
        bool res=bellman_ford_shortest_path(graph,0,
                                            tree,
                                            always_bool_t<true>{},
                                            updater);
        assert(res);
        std::vector<std::vector<int>> rank(rows,std::vector<int>(cols));
        for(size_t i=1;i<graph.size();++i)
        {
            auto [r,c]=indx_to_cell(i);
            assert(tree.opt_dist(i));
            rank[r][c]=*tree.opt_dist(i);
        }
        return rank;
    }
};

void Test_1632()
{
    using matrix_t=std::vector<std::vector<int>>;
    Solution_1632 sol;
    bool ans;
    ans=sol.matrixRankTransform({{7,7},{7,7}})==matrix_t{{1,1},{1,1}};
    assert(ans);
    ans=sol.matrixRankTransform({{1,2},{3,4}})==matrix_t{{1,2},{2,3}};
    assert(ans);
    ans=sol.matrixRankTransform({{20,-21,14},{-19,4,19},{22,-47,24},{-19,4,19}})==
                                matrix_t{{4,2,3},{1,3,4},{5,1,6},{1,3,4}};
    assert(ans);
    std::cout<<"1632 problem complete\n";
}


/*
1334 Find city with the smallest number of reachable neighbors
Medium
There are n cities numbered from 0 to n-1.
Given the array edges where edges[i] = [fromi, toi, weighti] represents a
bidirectional and weighted edge between cities fromi and toi,
and given the integer distanceThreshold.
Return the city with the smallest number of cities that are reachable
through some path and whose distance is at most distanceThreshold,
If there are multiple such cities, return the city with the greatest number.
Notice that the distance of a path connecting cities i and j is equal
to the sum of the edges weights along that path.

Example 1:
Input: n = 4, edges = [[0,1,3],[1,2,1],[1,3,4],[2,3,1]], distanceThreshold = 4
Output: 3
Explanation: The figure above describes the graph.
The neighboring cities at a distanceThreshold = 4 for each city are:
City 0 -> [City 1, City 2]
City 1 -> [City 0, City 2, City 3]
City 2 -> [City 0, City 1, City 3]
City 3 -> [City 1, City 2]
Cities 0 and 3 have 2 neighboring cities at a distanceThreshold = 4,
but we have to return city 3 since it has the greatest number.

Example 2:
Input: n = 5, edges = [[0,1,2],[0,4,8],[1,2,3],[1,4,2],[2,3,1],[3,4,1]], distanceThreshold = 2
Output: 0
Explanation: The figure above describes the graph.
The neighboring cities at a distanceThreshold = 2 for each city are:
City 0 -> [City 1]
City 1 -> [City 0, City 4]
City 2 -> [City 3, City 4]
City 3 -> [City 2, City 4]
City 4 -> [City 1, City 2, City 3]
The city 0 has 1 neighboring city at a distanceThreshold = 2.

Constraints:
2 <= n <= 100
1 <= edges.length <= n * (n - 1) / 2
edges[i].length == 3
0 <= fromi < toi < n
1 <= weighti, distanceThreshold <= 10^4
All pairs (fromi, toi) are distinct.
*/

class Solution_1334
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    public:
    int findTheCity(int n,const std::vector<std::vector<int>>& edges, int distanceThreshold)
    {
        graph_t graph;
        tree_search_t<graph_t> tree{};
        edge_list_to_graph(n,edges,graph,false);
        std::vector<size_t> reachable(n,0);
        size_t source;
        size_t city=0;
        auto search=full_search_t<graph_t>{}.set_edge_filter([&](size_t node,edge_t edge)
        {
            return tree.dist(node)+traits::weight(graph,node,edge)<=(size_t)distanceThreshold;
        })
        .set_node_preprocess([&](size_t )
        {
            reachable[source]++;
            return true;
        });
        for(source=0;source<(size_t)n;++source)
        {
            dense_dijkstra(graph,source,search,tree);
            if(reachable[source]<=reachable[city])
            {
                city=source;
            }
        }
        return city;
    }
};

void Test_1334()
{
    Solution_1334 sol;
    bool ans;
    ans=sol.findTheCity(4,{{0,1,3},{1,2,1},{1,3,4},{2,3,1}},4)==3;
    assert(ans);
    ans=sol.findTheCity(5,{{0,1,2},{0,4,8},{1,2,3},{1,4,2},{2,3,1},{3,4,1}},2)==0;
    assert(ans);
    std::cout<<"1334 problem complete\n";
}

/*
1368 Min cost
Hard
Given an m x n grid. Each cell of the grid has a sign pointing to the next cell you should
visit if you are currently in this cell. The sign of grid[i][j] can be:
1 which means go to the cell to the right. (i.e go from grid[i][j] to grid[i][j + 1])
2 which means go to the cell to the left. (i.e go from grid[i][j] to grid[i][j - 1])
3 which means go to the lower cell. (i.e go from grid[i][j] to grid[i + 1][j])
4 which means go to the upper cell. (i.e go from grid[i][j] to grid[i - 1][j])
Notice that there could be some signs on the cells of the grid that point outside the grid.
You will initially start at the upper left cell (0, 0). A valid path in the grid is a
path that starts from the upper left cell (0, 0) and ends at the bottom-right
cell (m - 1, n - 1) following the signs on the grid.
The valid path does not have to be the shortest.
You can modify the sign on a cell with cost = 1.
You can modify the sign on a cell one time only.
Return the minimum cost to make the grid have at least one valid path.


Example 1:
Input: grid = [[1,1,1,1],[2,2,2,2],[1,1,1,1],[2,2,2,2]]
Output: 3
Explanation: You will start at point (0, 0).
The path to (3, 3) is as follows. (0, 0) --> (0, 1) --> (0, 2) --> (0, 3)
change the arrow to down with cost = 1 --> (1, 3) --> (1, 2) --> (1, 1) --> (1, 0)
change the arrow to down with cost = 1 --> (2, 0) --> (2, 1) --> (2, 2) --> (2, 3)
change the arrow to down with cost = 1 --> (3, 3)
The total cost = 3.

Example 2:
Input: grid = [[1,1,3],[3,2,2],[1,1,4]]
Output: 0
Explanation: You can follow the path from (0, 0) to (2, 2).

Example 3:
Input: grid = [[1,2],[4,3]]
Output: 1

Constraints:
m == grid.length
n == grid[i].length
1 <= m, n <= 100
1 <= grid[i][j] <= 4
*/

class Solution_1368
{
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    const int right=1;
    const int left=2;
    const int lower=3;
    const int upper=4;
    public:
    int minCost(const std::vector<std::vector<int>>& grid)
    {
        const size_t rows=grid.size();
        const size_t cols=grid[0].size();
        auto cell_to_indx=[&](size_t r,size_t c){return r*cols+c;};
        graph_t graph(rows*cols);
        for(size_t i=0;i<rows;++i)
        {
            for(size_t j=0;j<cols;++j)
            {
                size_t from=cell_to_indx(i,j);
                if(i!=0)
                {
                    graph[from].push_back({cell_to_indx(i-1,j),(grid[i-1][j]==upper? 0:1)});
                }
                if(i!=rows-1)
                {
                    graph[from].push_back({cell_to_indx(i+1,j),(grid[i][j]==lower? 0:1)});
                }
                if(j!=0)
                {
                    graph[from].push_back({cell_to_indx(i,j-1),(grid[i][j]==left? 0:1)});
                }
                if(j!=cols-1)
                {
                    graph[from].push_back({cell_to_indx(i,j+1),(grid[i][j]==right? 0:1)});
                }

            }
        }
        size_t dest=cell_to_indx(rows-1,cols-1);
        tree_search_t<graph_t> tree;
        sparse_dijkstra(graph,0,full_search_t<graph_t>{},tree);
        assert(tree.opt_dist(dest));
        return *tree.opt_dist(dest);
    }
};

void Test_1368()
{
    Solution_1368 sol;
    bool ans;
    ans=sol.minCost({{1,1,1,1},{2,2,2,2},{1,1,1,1},{2,2,2,2}})==3;
    assert(ans);
    ans=sol.minCost({{1,1,3},{3,2,2},{1,1,4}})==0;
    assert(ans);
    ans=sol.minCost({{1,2},{4,3}})==1;
    assert(ans);
    std::cout<<"1368 problem complete\n";
}

/*

1557 Minimal set
Medium
Given a directed acyclic graph, with n vertices numbered from 0 to n-1,
and an array edges where edges[i] = [fromi, toi] represents a directed edge from node
fromi to node toi.
Find the smallest set of vertices from which all nodes in the graph are reachable.
It's guaranteed that a unique solution exists.
Notice that you can return the vertices in any order.

Example 1:
Input: n = 6, edges = [[0,1],[0,2],[2,5],[3,4],[4,2]]
Output: [0,3]
Explanation: It's not possible to reach all the nodes from a single vertex.
From 0 we can reach [0,1,2,5]. From 3 we can reach [3,4,2,5]. So we output [0,3].

Example 2:
Input: n = 5, edges = [[0,1],[2,1],[3,1],[1,4],[2,4]]
Output: [0,2,3]
Explanation: Notice that vertices 0, 3 and 2 are not reachable from any other node,
so we must include them. Also any of these vertices can reach nodes 1 and 4.

Constraints:

2 <= n <= 10^5
1 <= edges.length <= min(10^5, n * (n - 1) / 2)
edges[i].length == 2
0 <= fromi, toi < n
All pairs (fromi, toi) are distinct.
*/

class Solution_1557
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<std::pair<size_t,size_t>>>;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    public:
    std::vector<int> findSmallestSetOfVertices(int n,const std::vector<std::vector<int>>& edges)
    {
        graph_t graph;
        edge_list_to_graph(n,edges,graph,true);
        std::vector<char> processed(n,0);
        auto search=full_search_t<graph_t>{}.set_node_preprocess([&](size_t node)
        {
            processed[node]=1;
            return true;
        })
        .set_edge_filter([&](size_t node,edge_t edge)
        {
            return !processed[traits::target(graph,node,edge)];
        });
        std::vector<size_t> top_sorted;
        std::vector<size_t> nodes(n);
        std::iota(nodes.begin(),nodes.end(),0);
        bool dag=total_topological_sort(graph,nodes,[&](size_t node ){top_sorted.push_back(node);});
        assert(dag);
        std::reverse(top_sorted.begin(),top_sorted.end());
        std::vector<int> res;
        for(node_t source:top_sorted)
        {
            if(!processed[source])
            {
                res.push_back(source);
                breadth_search(true,graph,source,search);
            }
        }
        std::sort(res.begin(),res.end());
        return res;
    }
};

void Test_1557()
{
    using vec_t=std::vector<int>;
    Solution_1557 sol;
    bool ans;
    ans=sol.findSmallestSetOfVertices(6,{{0,1},{0,2},{2,5},{3,4},{4,2}})==vec_t{0,3};
    assert(ans);
    ans=sol.findSmallestSetOfVertices(5,{{0,1},{2,1},{3,1},{1,4},{2,4}})==vec_t{0,2,3};
    assert(ans);
    std::cout<<"1557 problem complete\n";
}

/*
1584 min cost connect points
Medium
You are given an array points representing integer coordinates of some points on a 2D-plane,
where points[i] = [xi, yi].
The cost of connecting two points [xi, yi] and [xj, yj] is the manhattan distance between them:
|xi - xj| + |yi - yj|, where |val| denotes the absolute value of val.
Return the minimum cost to make all points connected.
All points are connected if there is exactly one simple path between any two points.

Example 1:
Input: points = [[0,0],[2,2],[3,10],[5,2],[7,0]]
Output: 20
Explanation:
We can connect the points as shown above to get the minimum cost of 20.
Notice that there is a unique path between every pair of points.

Example 2:
Input: points = [[3,12],[-2,5],[-4,1]]
Output: 18

Constraints:
1 <= points.length <= 1000
-10^6 <= xi, yi <= 10^6
All pairs (xi, yi) are distinct.
*/
class Solution_1584
{
    using size_t=std::size_t;
    using graph_t=std::vector<std::vector<size_t>>;
    using traits=graph_traits<graph_t>;
    using node_t=traits::node_handler_t;
    using edge_t=traits::edge_handler_t;
    public:
    int minCostConnectPoints(const std::vector<std::vector<int>>& points)
    {
        std::vector<size_t> adj_list(points.size());
        std::iota(adj_list.begin(),adj_list.end(),0);
        graph_t graph(points.size(),adj_list);

        auto wg=make_weighted_graph(graph,[&](node_t n,edge_t e)
        {
            node_t target=traits::target(graph,n,e);
            return std::abs(points[n][0]-points[target][0])+
                   std::abs(points[n][1]-points[target][1]);
        });
        tree_search_t<decltype(wg)> tree;
        return dense_prim(wg,0,tree).first;
    }
};

void Test_1584()
{
    Solution_1584 sol;
    bool ans;
    ans=sol.minCostConnectPoints({{0,0},{2,2},{3,10},{5,2},{7,0}})==20;
    assert(ans);
    ans=sol.minCostConnectPoints({{3,12},{-2,5},{-4,1}})==18;
    assert(ans);
    std::cout<<"1584 problem complete\n";
}

}


void test_leetcode()
{
    Test_207();
    Test_210();
    Test_329();
    Test_743();
    Test_882();
    Test_1632();
    Test_1334();
    Test_1368();
    Test_1557();
    Test_1584();
    std::cout<<"TestLeetcode complete\n";
}



























