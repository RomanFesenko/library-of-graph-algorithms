
#ifndef  _liga_graph_generator_
#define  _liga_graph_generator_

#include <vector>
#include <array>
#include <algorithm>
#include <optional>
#include <utility>
#include <type_traits>

#include "rand_utility.h"

namespace liga{


class graph_generator_t
{
    using size_t=std::size_t;
    public:
    using graph_t=std::vector<std::vector<size_t>>;// adjacency list
    enum graph_type_t:char
    {
        tree_id=0,
        undirected_graph_id,
        directed_graph_id,
        dag_id,
        bipartite_id,
        strongly_connected_id
    };
    static bool is_undirect(graph_type_t t)
    {
        return t==tree_id||t==undirected_graph_id||t==bipartite_id;
    }
    static std::pair<std::size_t,std::size_t> edge_range(std::size_t nodes,graph_type_t);
    private:
    using adj_matrix_t=std::vector<std::vector<char>>;

    int m_init_random_engine=0;

    void m_make_undirected_tree(graph_t&,size_t nodes);
    void m_make_undirected_tree(graph_t&,size_t nodes,adj_matrix_t&);

    void m_make_directed_tree(graph_t&,size_t nodes);
    void m_make_directed_tree(graph_t&,size_t nodes,adj_matrix_t&);

    void m_make_undirected_graph(graph_t&,size_t nodes,size_t edges);
    void m_make_directed_graph(graph_t&,size_t nodes,size_t edges);

    void m_make_dag(graph_t&,size_t nodes,size_t edges);

    void m_make_bipartite(graph_t&,size_t nodes,size_t edges);


    public:
    template<class weight_t>
    using w_graph_t=std::vector<std::vector<std::pair<size_t,weight_t>>>;// adjacency list

    template<class l_t>
    struct euclidian_graph_t
    {
        using length_t=l_t;
        struct node_t
        {
            std::array<length_t,2>                  point;
            std::vector<std::pair<size_t,length_t>> adjacent_points;
        };
        static length_t distance(const node_t&fst,const node_t&snd)
        {
            length_t delta_x=fst.point[0]-snd.point[0];
            length_t delta_y=fst.point[1]-snd.point[1];
            return std::sqrt(delta_x*delta_x+delta_y*delta_y);
        }
        class view_t
        {
            const std::vector<node_t>*m_nodes;
            public:
            using length_t=l_t;
            using node_handler_t=size_t;
            using edge_handler_t=std::vector<std::pair<size_t,length_t>>::const_iterator;

            view_t(const std::vector<node_t>&nodes):m_nodes(&nodes){}

            inline edge_handler_t edge_begin(node_handler_t n)const
            {
                return (*m_nodes)[n].adjacent_points.begin();
            }
            inline void edge_inc(node_handler_t n,edge_handler_t&edge)const
            {
                ++edge;
            }
            inline bool edge_end(node_handler_t n,edge_handler_t e)const
            {
                return (*m_nodes)[n].adjacent_points.end()==e;
            }
            inline node_handler_t target(node_handler_t n,edge_handler_t e)const
            {
                return e->first;
            }
        };
        std::vector<node_t> nodes;
        view_t view()const
        {
            return view_t(nodes);
        }
        void clear()
        {
            nodes.clear();
        }
        void resize(std::size_t s)
        {
            nodes.resize(s);
        }
    };

    struct request_t
    {
        graph_type_t type;
        size_t       num_nodes;
        size_t       num_edges;
        bool is_valid()const;
    };
    graph_generator_t(){}
    bool make_graph(graph_t&,const request_t&);
    template<class weight_t>
    bool make_wgraph(w_graph_t<weight_t>&w_graph,const request_t&req,const std::pair<weight_t,weight_t>&range)
    {
        assert(range.second>=range.first);
        graph_t graph;
        if(!make_graph(graph,req)) return false;
        auto gen=rand_generator(range.first,range.second,m_init_random_engine++);
        w_graph.clear();
        w_graph.resize(graph.size(),{});
        if(is_undirect(req.type))
        {
            for(size_t from=0;from<graph.size();++from)
            {
                for(size_t i=0;i<graph[from].size();++i)
                {
                    size_t to=graph[from][i];
                    assert(to!=from);
                    if(to>from) continue;
                    weight_t weight=gen();
                    w_graph[from].push_back({to,weight});
                    w_graph[to].push_back({from,weight});
                }
            }
        }
        else
        {
            for(size_t from=0;from<graph.size();++from)
            {
                for(size_t i=0;i<graph[from].size();++i)
                {
                    w_graph[from].push_back({graph[from][i],gen()});
                }
            }
        }
        return true;
    }
    template<class weight_t>
    w_graph_t<weight_t>
    wdigraph_without_neg_cycles(size_t nodes,size_t edges,std::vector<weight_t>&dists)
    {
        static_assert(std::is_signed_v<weight_t>);
        if(!request_t{directed_graph_id,nodes,edges}.is_valid()) return {};

        const weight_t typical_wht=weight_t{10};
        dists.resize(nodes);
        w_graph_t<weight_t> graph(nodes);
        adj_matrix_t in_graph(nodes,std::vector<char>(nodes,0));
        auto rand_int=rand_generator(size_t{0},nodes,m_init_random_engine++);
        auto rand_wght=rand_generator(-typical_wht,typical_wht,m_init_random_engine++);
        dists[0]=weight_t{0};
        for(size_t to=1;to<nodes;++to)
        {
            size_t from=rand_int()%to;
            weight_t weight=rand_wght();
            graph[from].push_back({to,weight});
            in_graph[from][to]=1;
            dists[to]=dists[from]+weight;
        }
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
        auto rand_double_2=rand_generator(weight_t{0},typical_wht,m_init_random_engine++);
        for(;edges;edges--)
        {
            auto [from,to]=queue.pop();
            weight_t weight=dists[to]-dists[from]+rand_double_2();
            graph[from].push_back({to,weight});
        }
        return graph;
    }

    template<class weight_t>
    w_graph_t<weight_t>
    wdigraph_without_neg_cycles(size_t nodes,size_t edges)
    {
        std::vector<weight_t> dists;
        return wdigraph_without_neg_cycles(nodes,edges,dists);
    }
    template<class length_t,class fdist_t=decltype(euclidian_graph_t<length_t>::distance)>
    bool make_euclidian_graph(euclidian_graph_t<length_t>&euc_graph,
                              const request_t&req,
                              const std::pair<std::array<length_t,2>,
                                              std::array<length_t,2>>&range,
                              fdist_t fdist=euclidian_graph_t<length_t>::distance)
    {
        auto [min,max]=range;
        assert(min[0]<max[0]&&min[1]<max[1]);
        auto gen_x=rand_generator(min[0],max[0],m_init_random_engine++);
        auto gen_y=rand_generator(min[1],max[1],m_init_random_engine++);
        graph_t graph;
        if(!make_graph(graph,req)) return false;
        euc_graph.clear();
        for(size_t i=0;i<graph.size();++i)
        {
            std::array<length_t,2> p={gen_x(),gen_y()};
            euc_graph.nodes.push_back({p,{}});
        }
        if(is_undirect(req.type))
        {
            for(size_t from=0;from<graph.size();++from)
            {
                for(size_t i=0;i<graph[from].size();++i)
                {
                    size_t to=graph[from][i];
                    assert(to!=from);
                    if(to>from) continue;
                    length_t dist=fdist(euc_graph.nodes[from],euc_graph.nodes[to]);
                    euc_graph.nodes[from].adjacent_points.push_back({to,dist});
                    euc_graph.nodes[to].adjacent_points.push_back({from,dist});
                }
            }
        }
        else
        {
            for(size_t from=0;from<graph.size();++from)
            {
                for(size_t i=0;i<graph[from].size();++i)
                {
                    size_t to=graph[from][i];
                    length_t dist=fdist(euc_graph.nodes[from],euc_graph.nodes[to]);
                    euc_graph.nodes[from].adjacent_points.push_back({to,dist});
                }
            }
        }
        return true;
    }
};


}// liga

#endif













