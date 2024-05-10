#ifndef _liga_default_graph_
#define _liga_default_graph_

#include <utility>
#include <vector>
#include <algorithm>
#include <limits>

#include "graph_chars.h"
#include "adjacency_list_view.h"


namespace liga{


struct graph_definitions
{
    using size_t=std::size_t;
    static const size_t max_integer=std::numeric_limits<size_t>::max();
    class base_node_t
    {
        size_t m_index=max_integer;
        public:
        size_t index()const{return m_index;}
        bool in_graph()const{return m_index!=max_integer;}
        virtual~base_node_t(){}
        template<class n,class e,char c>
        friend class graph_t;
    };
    template<class node_t>
    class base_edge_t
    {
        node_t*     m_source=nullptr;
        node_t*     m_target=nullptr;
        bool        m_direct;
        size_t      m_index=max_integer;
        size_t      m_pos_in_source_list=max_integer;
        size_t      m_pos_in_target_list=max_integer;
        public:
        node_t* source()const{return m_source;}
        node_t* target()const{return m_target;}
        node_t* adjacent(const node_t* end)const
        {
            return end==m_source? m_target:(end==m_target? m_source:nullptr);
        }
        bool directed()const{return m_direct;}
        size_t index()const{return m_index;}
        bool in_graph()const{return m_index!=max_integer;}
        virtual~base_edge_t(){}
        template<class n,class e,char c>
        friend class graph_t;
    };
    struct void_function_t
    {
        template<class...types_t>
        void operator()(types_t...)const
        {
        }
    };
};

template<class node_t_,class edge_t_,char type_>
class graph_t
{
    using size_t=std::size_t;
    static const size_t max_integer=std::numeric_limits<size_t>::max();
    struct target_t
    {
        node_t_* operator()(typename std::vector<std::pair<node_t_*,edge_t_*>>::const_iterator iter)const
        {
            return iter->first;
        }
    };
    public:
    using node_t=     node_t_;
    using edge_t=     edge_t_;
    using base_node_t=graph_definitions::base_node_t;
    using base_edge_t=graph_definitions::base_edge_t<node_t>;
    using adjacency_list_t=std::vector<std::vector<std::pair<node_t*,edge_t*>>>;
    using adjacency_list_view_t=liga::adjacency_list_view_t<adjacency_list_t,target_t,node_t*>;
    static const char type=type_;
    static adjacency_list_t join(const adjacency_list_t&fst,const adjacency_list_t&snd)
    {
        assert(fst.size()==snd.size());
        adjacency_list_t res=fst;
        for(size_t i=0;i<fst.size();++i)
        {
            for(size_t j=0;j<snd[i].size();++j)
            {
                res[i].push_back(snd[i][j]);
            }
        }
        return res;
    }
    private:
    std::vector<node_t*> m_nodes;
    std::vector<edge_t*> m_edges;
    adjacency_list_t     m_undirected_edges;
    adjacency_list_t     m_out_edges;
    adjacency_list_t     m_in_edges;

    void m_add_edge_imp(edge_t*added,node_t* source,node_t* target,bool is_direct)
    {
        assert(source!=target||(type&graph_chars_t::has_loops));
        added->m_source=source;
        added->m_target=target;
        added->m_direct=is_direct;
        added->m_index=m_edges.size();
        m_edges.push_back(added);
        if(!is_direct)
        {
            auto&adj=m_undirected_edges;
            added->m_pos_in_source_list=adj[source->index()].size();
            added->m_pos_in_target_list=adj[target->index()].size();
            adj[source->index()].push_back({target,added});
            if(target!=source)
                adj[target->index()].push_back({source,added});
        }
        else
        {
            auto&out_list=m_out_edges;
            auto&in_list= m_in_edges;
            added->m_pos_in_source_list=out_list[source->index()].size();
            added->m_pos_in_target_list=in_list[target->index()].size();
            out_list[source->index()].push_back({target,added});
            in_list[target->index()].push_back({source,added});
        }
    }

    public:
    static constexpr bool strongly_directed()
    {
        return graph_chars_t::is_strongly_directed<type>();
    }
    static constexpr bool strongly_undirected()
    {
        return graph_chars_t::is_strongly_undirected<type>();
    }
    static constexpr bool mixed()
    {
        return graph_chars_t::is_mixed<type>();
    }
    static constexpr bool has_multiedges()
    {
        return (type&graph_chars_t::has_multiedges);
    }

    graph_t()
    {
        static_assert(std::is_base_of_v<base_node_t,node_t>);
        static_assert(std::is_base_of_v<base_edge_t,edge_t>);
    }

    const std::vector<node_t*>&nodes()const{return m_nodes;}
    const std::vector<edge_t*>&edges()const{return m_edges;}

    const adjacency_list_t& undirected_adj_list()const
    requires ((type&graph_chars_t::undirected)==graph_chars_t::undirected)
    {
        return m_undirected_edges;
    }
    const std::vector<std::pair<node_t*,edge_t*>>& undirected_edges(const node_t*source)const
    requires ((type&graph_chars_t::undirected)==graph_chars_t::undirected)
    {
        return m_undirected_edges[source->index()];
    }

    const adjacency_list_t& out_adj_list()const
    requires ((type&graph_chars_t::directed)==graph_chars_t::directed)
    {
        return m_out_edges;
    }
    const std::vector<std::pair<node_t*,edge_t*>>& out_edges(const node_t*source)const
    requires ((type&graph_chars_t::directed)==graph_chars_t::directed)
    {
        return m_out_edges[source->index()];
    }

    const adjacency_list_t& in_adj_list()const
    requires ((type&graph_chars_t::directed)==graph_chars_t::directed)
    {
        return m_in_edges;
    }
    const std::vector<std::pair<node_t*,edge_t*>>& in_edges(const node_t*source)const
    requires ((type&graph_chars_t::directed)==graph_chars_t::directed)
    {
        return m_in_edges[source->index()];
    }

    edge_t* find_edge(const node_t* fst,const node_t* snd)const
    requires ((type&graph_chars_t::has_multiedges)==0)
    {
        auto find_in_list=[&](const adjacency_list_t& list)->edge_t*
        {
            for(auto [node,edge]:list[fst->index()])
            {
                if(node==snd) return edge;
            }
            return nullptr;
        };
        edge_t*edge;
        if constexpr(type&graph_chars_t::directed)
        {
            if(edge=find_in_list(m_out_edges);edge) return edge;
            if(edge=find_in_list(m_in_edges);edge)  return edge;
        }
        if constexpr(type&graph_chars_t::undirected)
        {
            if(edge=find_in_list(m_undirected_edges);edge) return edge;
        }
        return nullptr;
    }

    template<class func_t>
    std::size_t traverse_edges(node_t* fst,node_t* snd,func_t func)const
    requires (has_multiedges())
    {
        size_t total_traversed=0;
        std::vector<char> traversed(edges().size(),0);
        auto travers_list=[&](const adjacency_list_t& list)
        {
            for(auto [node,edge]:list[fst->index()])
            {
                if(node==snd&&!traversed[edge->index()])
                {
                    func(edge);
                    traversed[edge->index()]=1;
                    ++total_traversed;
                }
            }
        };
        if constexpr(type&graph_chars_t::directed)
        {
            travers_list(m_out_edges);
        }
        if constexpr(type&graph_chars_t::undirected)
        {
            travers_list(m_undirected_edges);
        }
        return total_traversed;
    }

    node_t* add_node(node_t*node)
    {
        assert(!node->in_graph());
        node->m_index=m_nodes.size();
        m_nodes.push_back(node);
        if constexpr(type&graph_chars_t::undirected)
        {
            m_undirected_edges.push_back({});
        }
        if constexpr(type&graph_chars_t::directed)
        {
            m_out_edges.push_back({});
            m_in_edges.push_back({});
        }
        return node;
    }
    template<class edge_postprocess_t=graph_definitions::void_function_t>
    node_t* remove_node(node_t*node,edge_postprocess_t edge_postprocess=graph_definitions::void_function_t())
    {
        assert(node->in_graph());
        if constexpr(type&graph_chars_t::undirected)
        {
            assert(m_undirected_edges.size()==m_nodes.size());
            auto&adj=m_undirected_edges[node->index()];
            while(!adj.empty())
            {
                edge_t*for_remove=adj[0].second;
                remove_edge(for_remove);
                edge_postprocess(for_remove);
            }
            std::swap(m_undirected_edges[node->index()],m_undirected_edges.back());
            m_undirected_edges.pop_back();
        }
        if constexpr(type&graph_chars_t::directed)
        {
            assert(m_out_edges.size()==m_nodes.size());
            auto& out=m_out_edges[node->index()];
            while(!out.empty())
            {
                edge_t*for_remove=out[0].second;
                remove_edge(for_remove);
                edge_postprocess(for_remove);
            }
            std::swap(m_out_edges[node->index()],m_out_edges.back());
            m_out_edges.pop_back();

            assert(m_in_edges.size()==m_nodes.size());
            auto&in=m_in_edges[node->index()];
            while(!in.empty())
            {
                edge_t*for_remove=in[0].second;
                remove_edge(for_remove);
                edge_postprocess(for_remove);
            }
            std::swap(m_in_edges[node->index()],m_in_edges.back());
            m_in_edges.pop_back();
        }
        std::swap(m_nodes[node->index()],m_nodes.back());
        m_nodes[node->index()]->m_index=node->index();
        m_nodes.pop_back();
        node->m_index=max_integer;
        return node;
    }
    bool add_edge(edge_t*edge,node_t* source,node_t* target) requires(!mixed())
    {
        assert(!edge->in_graph());
        if constexpr (!has_multiedges())
        {
            edge_t* prev=find_edge(source,target);
            if(prev) return false;
        }
        m_add_edge_imp(edge,source,target,strongly_directed());
        return true;
    }
    bool add_edge(edge_t*edge,node_t* source,node_t* target,bool is_direct) requires(mixed())
    {
        assert(!edge->in_graph());
        if constexpr (!has_multiedges())
        {
            edge_t* prev=find_edge(source,target);
            if(prev) return false;
        }
        m_add_edge_imp(edge,source,target,is_direct);
        return true;
    }
    edge_t* replace_or_add_edge(edge_t*edge,node_t* source,node_t* target)
    requires (!has_multiedges()&&!mixed())
    {
        assert(!edge->in_graph());
        edge_t* prev=find_edge(source,target);
        if(prev)
        {
            remove_edge(prev);
        }
        m_add_edge_imp(edge,source,target,strongly_directed());
        return prev;
    }
    edge_t* replace_or_add_edge(edge_t*edge,node_t* source,node_t* target,bool is_direct)
    requires (!has_multiedges()&&mixed())
    {
        assert(!edge->in_graph());
        edge_t* prev=find_edge(source,target);
        if(prev)
        {
            remove_edge(prev);
        }
        m_add_edge_imp(edge,source,target,is_direct);
        return prev;
    }
    edge_t* remove_edge(edge_t*edge)
    {
        assert(edge->in_graph());
        node_t*source=edge->source();
        node_t*target=edge->target();
        assert(source!=target||(type&graph_chars_t::has_loops));
        size_t pos_source=edge->m_pos_in_source_list;
        size_t pos_target=edge->m_pos_in_target_list;
        if(edge->directed())
        {
            auto&adj_out=m_out_edges[source->index()];
            assert(adj_out[pos_source].second==edge);
            std::swap(adj_out[pos_source],adj_out.back());
            adj_out.pop_back();
            adj_out[pos_source].second->m_pos_in_source_list=pos_source;

            auto&adj_in=m_in_edges[target->index()];
            assert(adj_in[pos_target].second==edge);
            std::swap(adj_in[pos_target],adj_in.back());
            adj_in.pop_back();
            adj_in[pos_target].second->m_pos_in_target_list=pos_target;
        }
        else
        {
            auto&adj_out=m_undirected_edges[source->index()];
            assert(adj_out[pos_source].second==edge);
            std::swap(adj_out[pos_source],adj_out.back());
            adj_out.pop_back();
            edge_t*replaced=adj_out[pos_source].second;
            if(replaced->source()==source)
            {
                replaced->m_pos_in_source_list=pos_source;
            }
            else
            {
                assert(replaced->target()==source);
                replaced->m_pos_in_target_list=pos_source;
            }
            if(source!=target)
            {
                auto&adj_in=m_undirected_edges[target->index()];
                assert(adj_in[pos_target].second==edge);
                std::swap(adj_in[pos_target],adj_in.back());
                adj_in.pop_back();
                edge_t*replaced2=adj_in[pos_target].second;
                if(replaced2->target()==target)
                {
                    replaced2->m_pos_in_target_list=pos_target;
                }
                else
                {
                    assert(replaced2->source()==target);
                    replaced2->m_pos_in_source_list=pos_target;
                }
            }
        }
        std::swap(m_edges[edge->index()],m_edges.back());
        m_edges[edge->index()]->m_index=edge->index();
        m_edges.pop_back();
        edge->m_index=edge->m_pos_in_source_list=edge->m_pos_in_target_list=max_integer;
        edge->m_source=edge->m_target=nullptr;
        return edge;
    }
    template<class edge_postprocess_t=graph_definitions::void_function_t,
             class node_postprocess_t=graph_definitions::void_function_t>
    void clear(edge_postprocess_t edge_postprocess=graph_definitions::void_function_t(),
               node_postprocess_t node_postprocess=graph_definitions::void_function_t())
    {
        while(!nodes().empty())
        {
            node_t* for_remove=nodes().back();
            remove_node(for_remove,edge_postprocess);
            node_postprocess(for_remove);
        }
    }

    void check_consistency()const
    {
        if(strongly_directed())
        {
            assert(m_undirected_edges.empty());
            assert(m_in_edges.size()==m_nodes.size());
            assert(m_out_edges.size()==m_nodes.size());
        }
        else if(strongly_undirected())
        {
            assert(m_in_edges.empty());
            assert(m_out_edges.empty());
            assert(m_undirected_edges.size()==m_nodes.size());
        }
        else
        {
            assert(m_in_edges.size()==m_nodes.size());
            assert(m_out_edges.size()==m_nodes.size());
            assert(m_undirected_edges.size()==m_nodes.size());
        }
        for(size_t ind=0;ind<m_nodes.size();++ind)
        {
            assert(m_nodes[ind]->index()==ind);
        }
        for(size_t ind=0;ind<m_edges.size();++ind)
        {
            assert(m_edges[ind]->index()==ind);
        }

        for(size_t s_ind=0;s_ind<m_undirected_edges.size();++s_ind)
        {
            const auto&adj=m_undirected_edges[s_ind];
            node_t* source=nodes()[s_ind];
            for(size_t t_ind=0;t_ind<adj.size();++t_ind)
            {
                auto [target,edge]=adj[t_ind];
                if(edge->source()==source)
                {
                    assert(edge->target()==target);
                    assert(edge->m_pos_in_source_list==t_ind);
                }
                else
                {
                    assert(edge->source()==target);
                    assert(edge->target()==source);
                    assert(edge->m_pos_in_target_list==t_ind);
                }
            }
        }

        for(size_t s_ind=0;s_ind<m_out_edges.size();++s_ind)
        {
            const auto&adj=m_out_edges[s_ind];
            node_t* source=nodes()[s_ind];
            for(size_t t_ind=0;t_ind<adj.size();++t_ind)
            {
                auto [target,edge]=adj[t_ind];
                assert(edge->source()==source);
                assert(edge->target()==target);
                assert(edge->m_pos_in_source_list==t_ind);
            }
        }

        for(size_t t_ind=0;t_ind<m_in_edges.size();++t_ind)
        {
            const auto&adj=m_in_edges[t_ind];
            node_t* target=nodes()[t_ind];
            for(size_t s_ind=0;s_ind<adj.size();++s_ind)
            {
                auto [source,edge]=adj[s_ind];
                assert(edge->source()==source);
                assert(edge->target()==target);
                assert(edge->m_pos_in_target_list==s_ind);
            }
        }
    }
};

}// liga

#endif


























