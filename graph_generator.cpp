#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>

#include "graph_generator.h"

/******************************************************************************
*******************************************************************************
                            graph_generator_t
*******************************************************************************
******************************************************************************/

std::pair<std::size_t,std::size_t> edge_range(std::size_t nodes,graph_class_t class_)
{
    assert(nodes>=2);

    std::pair<std::size_t,std::size_t> res;
    switch(class_)
    {
        case graph_class_t::undirected_graph_id:
        res= {nodes-1,nodes*(nodes-1)/2};
        break;

        case graph_class_t::directed_graph_id:
        res=  {nodes-1,nodes*(nodes-1)};
        break;

        case graph_class_t::undirected_tree_id:
        res=  {nodes-1,nodes-1};
        break;

        case graph_class_t::directed_tree_id:
        res=  {nodes-1,2*(nodes-1)};
        break;

        case graph_class_t::bipartite_id:
        {
            std::size_t part1=nodes/2;
            res=  {nodes-1,part1*(nodes-part1)};
        }
        break;

        case graph_class_t::dag_id:
        res=  {nodes-1,nodes*(nodes-1)/2};
        break;

        case graph_class_t::strongly_connected_id:
        res=  {nodes,nodes*(nodes-1)};
        break;

        default:assert(false);
    }
    return res;
}

void graph_generator_t::m_make_undirected_tree(graph_t&tree,size_t nodes)
{
    auto rand=rand_generator(size_t{0},nodes,m_init_random_engine++);
    for(size_t from=1;from<nodes;++from)
    {
        size_t to=rand()%from;
        tree[from].push_back(to);
        tree[to].push_back(from);
    }
}

void graph_generator_t::m_make_undirected_tree(graph_t&tree,size_t nodes,adj_matrix_t&mtx)
{
    auto rand=rand_generator(size_t{0},nodes,m_init_random_engine++);
    for(size_t from=1;from<nodes;++from)
    {
        size_t to=rand()%from;
        assert(!mtx[from][to]);
        tree[from].push_back(to);
        tree[to].push_back(from);
        mtx[from][to]=mtx[to][from]=1;
    }
}

void graph_generator_t::m_make_directed_tree(graph_t&tree,size_t nodes)
{
    auto rand_gen=rand_generator(size_t{0},2*nodes,m_init_random_engine++);
    for(size_t from=1;from<nodes;++from)
    {
        size_t rand=rand_gen();
        bool from_to=rand&1;
        size_t to=(rand/2)%from;
        if(from_to)
        {
            tree[from].push_back(to);
        }
        else
        {
            tree[to].push_back(from);
        }
    }
}

void graph_generator_t::m_make_directed_tree(graph_t&tree,size_t nodes,adj_matrix_t&mtx)
{
    auto rand_gen=rand_generator(size_t{0},2*nodes,m_init_random_engine++);
    for(size_t from=1;from<nodes;++from)
    {
        size_t rand=rand_gen();
        bool from_to=rand&1;
        size_t to=(rand/2)%from;
        if(from_to)
        {
            assert(!mtx[from][to]);
            tree[from].push_back(to);
            mtx[from][to]=1;
        }
        else
        {
            assert(!mtx[to][from]);
            tree[to].push_back(from);
            mtx[to][from]=1;
        }
    }
}

void graph_generator_t::m_make_undirected_graph(graph_t&graph,size_t nodes,size_t edges)
{
    adj_matrix_t in_graph(nodes,std::vector<char>(nodes,0));
    m_make_undirected_tree(graph,nodes,in_graph);
    size_t remain_edges=edges-nodes+1;

    std::vector<std::pair<size_t,size_t>> free_edges;
    for(size_t i=0;i<nodes;++i)
    {
        for(size_t j=0;j<i;++j)
        {
            if(!in_graph[i][j]) free_edges.push_back({i,j});
        }
    }
    assert(free_edges.size()>=remain_edges);
    random_queue_t<std::pair<size_t,size_t>> queue(free_edges,m_init_random_engine++);
    for(;remain_edges;remain_edges--)
    {
        auto [from,to]=queue.pop();
        graph[from].push_back(to);
        graph[to].push_back(from);
    }
}

void graph_generator_t::m_make_directed_graph(graph_t&graph,size_t nodes,size_t edges)
{
    adj_matrix_t in_graph(nodes,std::vector<char>(nodes,0));
    m_make_directed_tree(graph,nodes,in_graph);
    edges-=nodes-1;

    std::vector<std::pair<size_t,size_t>> free_edges;
    for(size_t i=0;i<nodes;++i)
    {
        for(size_t j=0;j<nodes;++j)
        {
            if(i!=j&&!in_graph[i][j]) free_edges.push_back({i,j});
        }
    }
    assert(free_edges.size()>=edges);
    random_queue_t<std::pair<size_t,size_t>> queue(free_edges,m_init_random_engine++);
    for(;edges;edges--)
    {
        auto [from,to]=queue.pop();
        graph[from].push_back(to);
    }
}

void graph_generator_t::m_make_dag(graph_t&graph,size_t numnodes,size_t edges)
{
    std::vector<size_t> nodes(numnodes);
    std::iota(nodes.begin(),nodes.end(),0);
    std::random_shuffle(nodes.begin(),nodes.end());// in topological sort order

    auto rand_gen=rand_generator(size_t{0},numnodes,m_init_random_engine++);
    adj_matrix_t in_graph(numnodes,std::vector<char>(numnodes,0));
    for(size_t to_indx=1;to_indx<numnodes;++to_indx)
    {
        size_t from_indx=rand_gen()%to_indx;
        size_t from=nodes[from_indx];
        size_t to=nodes[to_indx];
        graph[from].push_back(to);
        in_graph[from][to]=1;
    }
    edges-=numnodes-1;
    std::vector<std::pair<size_t,size_t>> free_edges;
    for(size_t i=0;i<numnodes;++i)
    {
        for(size_t j=i+1;j<numnodes;++j)
        {
            if(!in_graph[nodes[i]][nodes[j]])
            {
                free_edges.push_back({nodes[i],nodes[j]});// nodes[i]->nodes[j]
            }
        }
    }
    assert(free_edges.size()>=edges);
    random_queue_t<std::pair<size_t,size_t>> queue(free_edges,m_init_random_engine++);
    for(;edges;edges--)
    {
        auto [from,to]=queue.pop();
        graph[from].push_back(to);
    }
}

void graph_generator_t::m_make_bipartite(graph_t&graph,size_t numnodes,size_t edges)
{
    adj_matrix_t in_graph(numnodes,std::vector<char>(numnodes,0));
    std::vector<char> node_type(numnodes,0);
    auto make_edge=[&](size_t fst,size_t snd)
    {
        assert(fst!=snd);
        assert(!in_graph[fst][snd]);
        graph[fst].push_back(snd);
        graph[snd].push_back(fst);
        in_graph[fst][snd]=in_graph[snd][fst]=1;
    };
    auto rand_gen=rand_generator(size_t{0},numnodes,m_init_random_engine++);
    size_t num_node_1=rand_gen();
    size_t num_node_2=numnodes-num_node_1;
    if(num_node_1>num_node_2) std::swap(num_node_1,num_node_2);
    while(num_node_1*num_node_2<edges)
    {
        assert(num_node_1<num_node_2);
        ++num_node_1;
        --num_node_2;
    }
    std::vector<size_t> nodes_1(numnodes),nodes_2;
    std::iota(nodes_1.begin(),nodes_1.end(),0);
    random_queue_t<size_t> queue(nodes_1,m_init_random_engine++);
    while(nodes_2.size()!=num_node_2)
    {
        size_t val=queue.pop();
        nodes_2.push_back(val);
        node_type[val]=1;
    }
    make_edge(nodes_1[0],nodes_2[0]);
    size_t ind_1=1;
    size_t ind_2=1;
    while(ind_1!=nodes_1.size()||ind_2!=nodes_2.size())
    {
        if(ind_1!=nodes_1.size())
        {
            make_edge(nodes_1[ind_1],nodes_2[rand_gen()%ind_2]);
            ++ind_1;
        }
        if(ind_2!=nodes_2.size())
        {
            make_edge(nodes_2[ind_2],nodes_1[rand_gen()%ind_1]);
            ++ind_2;
        }
    }
    edges-=numnodes-1;
    std::vector<std::pair<size_t,size_t>> free_edges;
    for(size_t i=0;i<graph.size();++i)
    {
        for(size_t j=0;j<i;++j)
        {
            if(!in_graph[i][j]&&node_type[i]!=node_type[j]) free_edges.push_back({i,j});
        }
    }
    assert(free_edges.size()>=edges);
    random_queue_t<std::pair<size_t,size_t>> equeue(free_edges,m_init_random_engine++);
    for(;edges;edges--)
    {
        auto [from,to]=equeue.pop();
        graph[from].push_back(to);
        graph[to].push_back(from);
    }
}


bool graph_generator_t::graph_chars_t::is_valid()const
{
    if(num_nodes<2) return false;
    auto r=edge_range(num_nodes,g_class);
    return num_edges>=r.first&&num_edges<=r.second;
}


bool graph_generator_t::make_graph(graph_t&graph,const graph_chars_t&chars_)
{
    if(!chars_.is_valid()) return false;
    graph.clear();
    graph.resize(chars_.num_nodes,{});
    switch(chars_.g_class)
    {
        case graph_class_t::undirected_graph_id:
        m_make_undirected_graph(graph,chars_.num_nodes,chars_.num_edges);
        break;

        case graph_class_t::directed_graph_id:
        m_make_directed_graph(graph,chars_.num_nodes,chars_.num_edges);
        break;

        case graph_class_t::undirected_tree_id:
        m_make_undirected_tree(graph,chars_.num_nodes);
        break;

        case graph_class_t::directed_tree_id:
        m_make_directed_tree(graph,chars_.num_nodes);
        break;

        case graph_class_t::bipartite_id:
        m_make_bipartite(graph,chars_.num_nodes,chars_.num_edges);
        break;

        case graph_class_t::dag_id:
        m_make_dag(graph,chars_.num_nodes,chars_.num_edges);
        break;

        default:assert(false);
    }
    return true;
}


























