#include <iostream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include "../graph_utility.h"


#include "matching.h"

namespace liga{

namespace matching{

namespace {// utility

template<class vec_t,class int_t>
void swap_remove(vec_t&vec,int_t i)
{
    std::swap(vec[i],vec.back());
    vec.pop_back();
}

}// utility


int matching_weight(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                    const std::vector<size_t>&match)
{
    using size_t=std::size_t;
    int weight=0;
    for(size_t from=0;from<graph.size();++from)
    {
        size_t to=match[from];
        if(to<=from) continue;
        weight+=std::find_if(graph[from].begin(),graph[from].end(),[&](auto&e){return e.first==to;})->second;
    }
    return weight;
}

int covering_weight(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                    const std::vector<std::vector<std::size_t>>&cov)
{
    using size_t=std::size_t;
    int weight=0;
    for(size_t from=0;from<graph.size();++from)
    {
        for(size_t to:cov[from])
        {
            if(to>from) continue;
            weight+=(*find_edge(graph,from,to))->second;
        }
    }
    return weight;
}

bool is_valid_matching(const std::vector<std::vector<std::size_t>>&graph,
                       const std::vector<std::size_t>&match)
{
    using size_t=std::size_t;
    for(size_t from=0;from<graph.size();++from)
    {
        size_t to=match[from];
        if(to==from) continue;
        if(to>=graph.size()||match[to]!=from||!find_edge(graph,to,from)) return false;
    }
    return true;
}

bool is_valid_matching(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                       const std::vector<std::size_t>&match)
{
    using size_t=std::size_t;
    for(size_t from=0;from<graph.size();++from)
    {
        size_t to=match[from];
        if(to==from) continue;
        if(to>=graph.size()||match[to]!=from||!find_edge(graph,to,from)) return false;
    }
    return true;
}

bool is_valid_covering(const std::vector<std::vector<std::size_t>>&graph,
                       const std::vector<std::vector<std::size_t>>&cov)
{
    using size_t=std::size_t;
    for(size_t from=0;from<graph.size();++from)
    {
        if(cov[from].empty()) return false;
        for(size_t to:cov[from])
        {
            if(to>=graph.size()||from==to||!find_edge(graph,from,to)||!find_edge(cov,to,from))return false;
        }
    }
    return true;
}

bool is_valid_covering(const std::vector<std::vector<std::pair<std::size_t,int>>>&graph,
                       const std::vector<std::vector<std::size_t>>&cov)
{
    using size_t=std::size_t;
    for(size_t from=0;from<graph.size();++from)
    {
        if(cov[from].empty()) return false;
        for(size_t to:cov[from])
        {
            if(to>=graph.size()||from==to||!find_edge(graph,from,to)||!find_edge(cov,to,from))return false;
        }
    }
    return true;
}

/******************************************************************************
*******************************************************************************
                            bipartite_solver_t
*******************************************************************************
******************************************************************************/

void bipartite_solver_t::m_augment(size_t root,size_t tail,std::vector<size_t>&prev,std::vector<size_t>&match)
{
    while(true)
    {
        size_t p=prev[tail];
        match[p]=tail;
        match[tail]=p;
        if(p==root) break;
        tail=prev[p];
    }
}

size_t bipartite_solver_t::max_cardinality(const graph_t&graph,std::vector<size_t>&match)
{
    //std::cout<<"bipartite_solver_t::max_cardinality begin\n";
    size_t res=0;
    std::vector<size_t> queue;
    std::vector<size_t> prev;

    match.clear();
    for(size_t i=0;i<graph.size();++i)
    {
        match.push_back(i);
        prev.push_back(i);
    }
    for(size_t root=0;root<graph.size();++root)
    {
        if(match[root]!=root) continue;
        queue.push_back(root);
        while(!queue.empty())
        {
            size_t from=queue.back();
            queue.pop_back();
            for(size_t to:graph[from])
            {
                assert(to!=root);
                if(prev[to]==to)
                {
                    prev[to]=from;
                    if(size_t m=match[to];m!=to)
                    {
                        prev[m]=to;
                        queue.push_back(m);
                    }
                    else
                    {
                        m_augment(root,to,prev,match);
                        queue.clear();
                        for(size_t i=0;i<graph.size();++i)
                        {
                            prev[i]=i;
                        }
                        res++;
                        break;
                    }
                }
                else assert(prev[to]!=match[to]);
            }
        }
    }
    assert(is_valid_matching(graph,match));
    return res;
}

size_t bipartite_solver_t::max_cardinality(const graph_t&graph)
{
    std::vector<size_t> match;
    return max_cardinality(graph,match);
}


template<bool is_perfect>
std::optional<int> bipartite_solver_t::m_max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    std::vector<size_t> exts;
    std::vector<size_t> prev;
    std::vector<int>    pot;

    match.clear();
    for(size_t i=0;i<graph.size();++i)
    {
        match.push_back(i);
        prev.push_back(i);
        pot.push_back(0);
        for(auto [to,weight]:graph[i])
        {
            pot.back()=std::max(pot.back(),weight);
        }
    }
    for(size_t root=0;root<graph.size();++root)
    {
        if(match[root]!=root) continue;
        exts.push_back(root);
        //std::cout<<"root "<<root<<'\n';
        while(true)
        {
            std::optional<int> min_delta[2];
            std::pair<size_t,size_t> min_edge;
            size_t min_external;
            for(size_t from:exts)
            {
                for(size_t j=0;j<graph[from].size();++j)
                {
                    auto [to,weight]=graph[from][j];
                    assert(to!=root);
                    if(prev[to]!=to)
                    {
                        assert(prev[to]!=match[to]);
                        continue;
                    }
                    int delta=pot[from]+pot[to]-weight;
                    //std::cout<<"from,to,delta:"<<from<<','<<to<<','<<delta<<'\n';
                    if(!min_delta[0]||delta<*min_delta[0])
                    {
                        min_delta[0]=delta;
                        min_edge={from,j};
                    }
                }
            }
            if constexpr(!is_perfect)
            {
                min_external=exts[0];
                min_delta[1]=pot[min_external];
                for(size_t ext:exts)
                {
                    if(pot[ext]<*min_delta[1])
                    {
                        min_delta[1]=pot[ext];
                        min_external=ext;
                    }
                }
            }

            int total_min_delta;char case_=-1;
            if(min_delta[0]&&(case_==-1||*min_delta[0]<total_min_delta))
            {
                total_min_delta=*min_delta[0];
                case_=0;
            }
            if constexpr(!is_perfect)
            {
                if(min_delta[1]&&(case_==-1||*min_delta[1]<total_min_delta))
                {
                    total_min_delta=*min_delta[1];
                    case_=1;
                }
            }

            assert(case_!=-1||is_perfect);
            if(case_==-1) return {};
            //std::cout<<"case_:"<<(int)case_<<'\n';
            //std::cout<<"min_delta:"<<min_delta<<'\n';
            //std::cout<<"min_delta_2:"<<min_delta_2<<'\n';
            //std::cout<<"min_delta_1:"<<(min_delta_1? *min_delta_1:99999)<<'\n';

            for(int ext:exts)
            {
                pot[ext]-=total_min_delta;
                assert(is_perfect||pot[ext]>=0);
                if(int m=match[ext];m!=ext) pot[m]+=total_min_delta;
            }

            if(case_==0)
            {
                auto [to,weight]=graph[min_edge.first][min_edge.second];
                prev[to]=min_edge.first;
                if(size_t m=match[to];m!=to)
                {
                    prev[m]=to;
                    exts.push_back(m);
                }
                else
                {
                    m_augment(root,to,prev,match);
                    exts.clear();
                    for(size_t i=0;i<graph.size();++i) prev[i]=i;
                    break;
                }
            }
            else
            {
                if(min_external!=root)
                {
                    assert(match[min_external]!=min_external);
                    m_augment(root,match[min_external],prev,match);
                    match[min_external]=min_external;
                }
                exts.clear();
                for(size_t i=0;i<graph.size();++i) prev[i]=i;
                break;
            }
        }
    }
    assert(is_valid_matching(graph,match));
    return matching_weight(graph,match);
}

std::optional<int> bipartite_solver_t::perfect_max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    return m_max_weighted<true>(graph,match);
}


std::optional<int> bipartite_solver_t::perfect_max_weighted(const w_graph_t&graph)
{
    std::vector<size_t> match;
    return m_max_weighted<true>(graph,match);
}

int bipartite_solver_t::max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    return *m_max_weighted<false>(graph,match);
}

int bipartite_solver_t::max_weighted(const w_graph_t&graph)
{
    std::vector<size_t> match;
    return *m_max_weighted<false>(graph,match);;
}

int bipartite_solver_t::min_covering(const w_graph_t&graph,graph_t&cov)
{
    using size_t=w_graph_t::size_type;
    std::vector<size_t> exts;
    std::vector<size_t> prev;
    std::vector<size_t> match;
    std::vector<int>    pot(graph.size(),0);

    cov.resize(graph.size());
    cov.assign(graph.size(),{});
    for(size_t i=0;i<graph.size();++i)
    {
        prev.push_back(i);
        match.push_back(i);
    }
    auto drop_tree=[&]()
    {
        exts.clear();
        for(size_t i=0;i<graph.size();++i) prev[i]=i;
    };
    auto save_match=[&](size_t i)
    {
        assert(match[i]!=i);
        assert(!find_edge(cov,i,match[i]));
        cov[i].push_back(match[i]);
        match[i]=i;
    };
    auto count_cover_edge=[&](size_t from){return match[from]==from? 0:1+cov[from].size();};

    auto remove_match=[&](size_t from,size_t to)
    {
        assert(!cov[from].empty());
        if(match[from]==to)
        {
            match[from]=cov[from].back();
            cov[from].pop_back();
        }
        else
        {
            auto e=find_edge(cov,from,to);
            assert(e);
            swap_remove(cov[from],*e-cov[from].begin());
        }
    };

    for(size_t root=0;root<graph.size();++root)
    {
        if(count_cover_edge(root)) continue;
        exts.push_back(root);
        //std::cout<<"root "<<root<<'\n';
        while(true)
        {
            std::optional<int> min_delta_1,min_delta_2;
            std::pair<size_t,size_t> min_edge;
            size_t min_internal;
            for(size_t from:exts)
            {
                //assert(count_cover_edge(from)<=1);
                for(size_t j=0;j<graph[from].size();++j)
                {
                    auto [to,weight]=graph[from][j];
                    assert(to!=root);
                    if(prev[to]!=to)
                    {
                        assert(match[to]!=prev[to]);
                        continue;
                    }
                    int delta=weight-pot[from]-pot[to];
                    //std::cout<<"from,to,delta:"<<from<<','<<to<<','<<delta<<'\n';
                    if(!min_delta_1||delta<*min_delta_1)
                    {
                        min_delta_1=delta;
                        min_edge={from,j};
                    }
                }
            }
            for(size_t ext:exts)
            {
                if(size_t internal=prev[ext];internal!=ext&&(!min_delta_2||pot[internal]<min_delta_2))
                {
                    min_delta_2=pot[internal];
                    min_internal=internal;
                }
            }
            int min_delta;int case_=-1;
            if(min_delta_1)
            {
                min_delta=*min_delta_1;
                case_=1;
            }
            if(min_delta_2&&(case_==-1||*min_delta_2<min_delta))
            {
                min_delta=*min_delta_2;
                case_=2;
            }
            assert(case_!=-1);
            //std::cout<<"case_:"<<(int)case_<<'\n';
            //std::cout<<"min_delta:"<<min_delta<<'\n';
            //std::cout<<"min_delta_2:"<<min_delta_2<<'\n';
            //std::cout<<"min_delta_1:"<<(min_delta_1? *min_delta_1:99999)<<'\n';
            for(size_t ext:exts)
            {
                pot[ext]+=min_delta;
                if(ext==root) continue;
                pot[match[ext]]-=min_delta;
                assert(pot[match[ext]]>=0);
            }
            if(case_==1)
            {
                auto [to,weight]=graph[min_edge.first][min_edge.second];
                prev[to]=min_edge.first;
                /**
                    "to" node is exposed: augment to-from and drop tree
                */
                if(const size_t covered=count_cover_edge(to);covered==0)
                {
                    m_augment(root,to,prev,match);
                    drop_tree();
                    break;
                }
                /**
                    "to" node covered few edges: augment to-from and drop tree
                */
                else if(covered>1)
                {
                    assert(pot[to]==0);
                    save_match(to);
                    m_augment(root,to,prev,match);
                    drop_tree();
                    break;
                }
                /**
                    "to" and match[to] node covered one edge:
                    add match[to] to external
                */
                else if(const size_t matched=match[to];count_cover_edge(matched)==1)
                {
                    assert(match[matched]==to);
                    prev[matched]=to;
                    exts.push_back(matched);
                }
                /**
                    "to" node covered one edge, but node matched with "to"
                    covered few edges: remove edge to-match[to],augment to-root
                    and drop tree
                */
                else
                {
                    assert(pot[matched]==0);
                    assert(count_cover_edge(matched)>1);
                    remove_match(matched,to);
                    m_augment(root,to,prev,match);
                    drop_tree();
                    break;
                }
            }
            /**
                after augmentation "min_internal"-root,
                "min_internal" covered 2 edges
            */
            else
            {
                save_match(min_internal);
                m_augment(root,min_internal,prev,match);
                drop_tree();
                break;
            }
        }
    }
    for(size_t i=0;i<graph.size();++i)
    {
        assert(match[i]!=i);
        save_match(i);
    }
    assert(is_valid_covering(graph,cov));
    return covering_weight(graph,cov);
}

int bipartite_solver_t::min_covering(const w_graph_t&graph)
{
    graph_t cov;
    return min_covering(graph,cov);
}


/******************************************************************************
*******************************************************************************
                            nonbipartite_solver_t
*******************************************************************************
******************************************************************************/


namespace blossom_ns{


struct node_t
{
    using size_t=std::size_t;
    using edge_t=std::pair<node_t*,node_t*>;
    size_t index;
    node_t*child;
    node_t*parent;
    node_t*top_parent;
    node_t*adj_nodes[2];
    edge_t adj_edges[2];
    node_t(size_t ind)
    {
        index=ind;
        parent=top_parent=this;
        child=adj_nodes[0]=adj_nodes[1]=nullptr;
    }
    bool is_top()const{return top_parent==this;}
    bool is_blossom()const{return child;}
    node_t*&prev_node(){return adj_nodes[0];}
    node_t*&match_node(){return adj_nodes[1];}
    edge_t&prev_edge(){return adj_edges[0];}
    edge_t&match_edge(){return adj_edges[1];}

    node_t*sub_node(node_t*term)
    {
        assert(!term->child);
        node_t*sub=term;
        while(sub->parent!=this) sub=sub->parent;
        return sub;
    }

    int blossom_match_ind(const std::vector<size_t>&m)const
    {
        if(m[adj_edges[0].first->index]==adj_edges[0].second->index) return 0;
        if(m[adj_edges[1].first->index]==adj_edges[1].second->index) return 1;

        return -1;
    }

    template<class fn_t>
    void traverse_nodes(fn_t fnc)
    {
        fnc(this);
        if(child)
        {
            child->traverse_nodes(fnc);
            for(node_t*cur=child->prev_node();cur!=child;cur=cur->prev_node())
            {
                cur->traverse_nodes(fnc);
            }
        }
    }
    template<class fn_t>
    void traverse_terminal_nodes(fn_t fnc)
    {
        if(!child)
        {
            fnc(this);
        }
        else
        {
            child->traverse_terminal_nodes(fnc);
            for(node_t*cur=child->prev_node();cur!=child;cur=cur->prev_node())
            {
                cur->traverse_terminal_nodes(fnc);
            }
        }
    }

    void set_as_blossom()
    {
        parent=top_parent=this;
        if(child)
        {
            child->parent=this;
            for(node_t*cur=child->prev_node();cur!=child;cur=cur->prev_node())
            {
                cur->parent=this;
            }
            traverse_nodes([this](node_t*node){node->top_parent=this;});
        }
    }
};


node_t::edge_t reverse(const node_t::edge_t&e)
{
    return {e.second,e.first};
}


bool is_valid_blossom_graph(const std::vector<node_t*>&nodes,const std::vector<std::size_t>&match)
{
    //std::cout<<"is_valid_blossom_graph begin\n";
    for(node_t*node:nodes)
    {
        //std::cout<<"check node:"<<node->index<<'\n';
        if(node->top_parent==node)
        {
            if(node_t*prev=node->prev_node();prev)
            {
                if(prev->top_parent!=prev) return false;
            }
            if(node_t*m=node->match_node();m)
            {
                if(m->top_parent!=m)    return false;
                if(m->match_node()!=node) return false;
                if(m->match_edge()!=reverse(node->match_edge())) return false;
                if(match[m->match_edge().first->index]!=m->match_edge().second->index||
                   match[m->match_edge().second->index]!=m->match_edge().first->index) return false;
            }
        }
        if(node->is_blossom())
        {
            std::vector<node_t*> forward,backward;
            //std::cout<<"is_valid_blossom_graph forward\n";
            for(node_t*cur=node->child->prev_node();cur!=node->child;cur=cur->prev_node())
            {
                //std::cout<<"cur:"<<cur->index<<'\n';
                if(cur->parent!=node) return false;
                if(cur->top_parent!=node->top_parent) return false;
                forward.push_back(cur);
            }
            //std::cout<<"is_valid_blossom_graph backward\n";
            for(node_t*cur=node->child->match_node();cur!=node->child;cur=cur->match_node())
            {
                //std::cout<<"cur:"<<cur->index<<'\n';
                if(cur->parent!=node) return false;
                if(cur->top_parent!=node->top_parent) return false;
                backward.push_back(cur);
            }
            std::reverse(backward.begin(),backward.end());
            if(forward!=backward)  return false;
            if(forward.size()%2!=0)return false;
        }
    }
    //std::cout<<"is_valid_blossom_graph end\n";
    return true;
}



void expose_node(node_t* top_node,node_t*low_node,std::vector<std::size_t>&match);

void set_match(node_t*fst,node_t*snd,const node_t::edge_t&e,std::vector<std::size_t>&match)
{
    //std::cout<<"set_match:"<<fst->index<<':'<<snd->index<<'\n';
    expose_node(fst,e.first,match);
    expose_node(snd,e.second,match);
    match[e.first->index]=e.second->index;
    match[e.second->index]=e.first->index;
}

void set_match_links(node_t*from,node_t*to,const node_t::edge_t&e)
{
    from->match_node()=to;
    from->match_edge()=e;
    to->match_node()=from;
    to->match_edge()=reverse(e);
}

void expose_node(node_t* top_n,node_t*low_n,std::vector<std::size_t>&match)
{
    //std::cout<<"expose_node:"<<top_n->index<<'>'<<low_n->index<<'\n';
    //system("PAUSE");
    assert(!low_n->child);
    assert(low_n->top_parent==top_n->top_parent);
    if(top_n==low_n)
    {
        match[low_n->index]=low_n->index;
        return;
    }
    node_t*sub_n=top_n->sub_node(low_n);
    //std::cout<<"sub_n:"<<sub_n->index<<'\n';

    if(const int ind=sub_n->blossom_match_ind(match);ind!=-1)
    {
        node_t*p=sub_n->adj_nodes[ind];
        node_t*pp=p->adj_nodes[ind];
        while(p!=sub_n)
        {
            set_match(p,pp,p->adj_edges[ind],match);
            p=pp->adj_nodes[ind];
            pp=p->adj_nodes[ind];
        }
    }
    expose_node(sub_n,low_n,match);
}

node_t* make_blossom(node_t*fst,node_t*snd,node_t*root,std::vector<node_t*>&nodes)
{
    using edge_t=node_t::edge_t;
    static std::vector<char> marked;
    marked.resize(nodes.size());
    marked.assign(nodes.size(),0);

    //std::cout<<"make_blossom\n";
    //std::cout<<"fst:"<<fst->index<<'\n';
    //std::cout<<"snd:"<<snd->index<<'\n';
    //std::cout<<"blossom root:"<<root->index<<'\n';

    node_t*top_root=root->top_parent;
    marked[top_root->index]=1;
    for(node_t*cur=fst->top_parent;cur!=top_root;cur=cur->prev_node())
    {
        marked[cur->index]=1;
    }
    node_t*blossom_base=snd->top_parent;

    node_t*next_node=fst->top_parent;
    edge_t next_edge={snd,fst};
    while(true)
    {
        blossom_base->adj_nodes[1]=next_node;
        blossom_base->adj_edges[1]=next_edge;
        if(!marked[blossom_base->index])
        {
            next_node=blossom_base;
            next_edge=reverse(blossom_base->prev_edge());
            blossom_base=blossom_base->prev_node();
        }
        else break;
    }

    node_t*new_blossom=new node_t(nodes.size());
    nodes.push_back(new_blossom);
    new_blossom->child=blossom_base;
    new_blossom->prev_node()=new_blossom->match_node()=blossom_base->prev_node();
    new_blossom->prev_edge()=new_blossom->match_edge()=blossom_base->prev_edge();

    if(node_t*m=new_blossom->match_node();m) m->match_node()=new_blossom;

    node_t*prev_node=snd->top_parent;
    edge_t prev_edge={fst,snd};
    for(node_t*cur=fst->top_parent;cur!=blossom_base;cur=cur->adj_nodes[1])
    {
        cur->adj_nodes[1]=cur->adj_nodes[0];
        cur->adj_edges[1]=cur->adj_edges[0];
        cur->adj_nodes[0]=prev_node;
        cur->adj_edges[0]=prev_edge;
        prev_node=cur;
        prev_edge=reverse(cur->adj_edges[1]);
    }
    blossom_base->prev_node()=prev_node;
    blossom_base->prev_edge()=prev_edge;

    new_blossom->set_as_blossom();

    for(node_t*n:nodes)
    {
        if(n->top_parent==n&&n->prev_node()&&n->prev_node()->parent==new_blossom)
        {
            n->prev_node()=new_blossom;
        }
    }
    return new_blossom;
}

void delete_blossom(node_t*blossom,std::vector<node_t*>&nodes,std::vector<std::size_t>&match,std::vector<node_t*>&exts)
{
    using edge_t=node_t::edge_t;
    assert(blossom->top_parent==blossom);
    assert(blossom->prev_node());
    assert(blossom->match_node());
    assert(blossom->match_node()!=blossom->prev_node());

    node_t*sub_1=blossom->sub_node(blossom->prev_edge().first);
    node_t*sub_2=blossom->sub_node(blossom->match_edge().first);
    assert(sub_1->parent==blossom&&sub_2->parent==blossom);
    int ind=sub_1->blossom_match_ind(match)==0? 1:0;

    //std::cout<<"decompose blossom 1\n";

    for(node_t*cur=blossom->child;;)
    {
        cur->set_as_blossom();
        cur=cur->prev_node();
        if(cur==blossom->child) break;
    }

    //std::cout<<"decompose blossom 2\n";
    for(node_t*cur=sub_1->adj_nodes[ind];cur!=sub_2;)
    {
        node_t*pp=cur->adj_nodes[ind]->adj_nodes[ind];
        set_match_links(cur,cur->adj_nodes[ind],cur->adj_edges[ind]);
        cur->prev_node()=cur->match_node()->prev_node()=nullptr;
        cur=pp;
    }

    //std::cout<<"decompose blossom 3\n";

    node_t*match_node=blossom->match_node();
    edge_t match_edge=match_node->match_edge();
    for(node_t*cur=sub_2;;)
    {
        match_node->prev_node()=cur;
        match_node->prev_edge()=match_edge;
        cur->prev_node()=cur->adj_nodes[ind];
        cur->prev_edge()=cur->adj_edges[ind];
        set_match_links(match_node,cur,match_edge);
        if(cur==sub_1) break;

        match_node=cur->prev_node();
        match_edge=match_node->adj_edges[ind];
        cur=match_node->adj_nodes[ind];
        exts.push_back(match_node);
    }
    sub_1->prev_node()=blossom->prev_node();
    sub_1->prev_edge()=blossom->prev_edge();

    nodes.back()->index=blossom->index;
    swap_remove(nodes,blossom->index);
    delete blossom;
}

void augment(node_t*root,node_t* tail,std::vector<std::size_t>&match)
{
    //std::cout<<"augment:"<<root->index<<'-'<<tail->index<<'\n';
    while(true)
    {
        node_t* p=tail->prev_node();
        set_match(tail,p,tail->prev_edge(),match);
        set_match_links(tail,p,tail->prev_edge());
        if(p==root) return;
        tail=p->prev_node();
    }
}


struct new_edge_t{} ;
struct blossom_edge_t{} ;
struct ext_node_t{} ;
struct int_node_t{} ;
struct ext_quasi_node_t{} ;
struct int_quasi_node_t{} ;

class search_data_t
{
    using w_graph_t=std::vector<std::vector<std::pair<std::size_t,int>>>;
    public:
    enum case_t
    {
        find_vertex_id=0,
        find_blossom_id,
        delete_blossom_id,
        expose_node_id,
        added_match_id,
        undefine_id
    };
    class edge_t
    {
        using size_t=std::size_t;
        std::pair<size_t,size_t> m_iter;
        public:
        edge_t()=default;
        edge_t(size_t f,size_t s ):m_iter{f,s}{}
        size_t fst()const{return m_iter.first;}
        size_t snd(const w_graph_t&graph)const
        {
            return graph[m_iter.first][m_iter.second].first;
        }
        int wht(const w_graph_t&graph)const
        {
            return graph[m_iter.first][m_iter.second].second;
        }
    };
    private:
    const w_graph_t&      m_graph;
    std::vector<node_t*>& m_nodes;
    std::vector<node_t*>& m_ext_nodes;
    node_t*               m_root;
    std::optional<double> m_min_delta[5];
    edge_t                m_min_edge[5];
    node_t*               m_min_node[5];
    double                m_total_min;
    case_t                m_opt_case_id=undefine_id;
    public:
    search_data_t(const w_graph_t&wg,std::vector<node_t*>&nds,std::vector<node_t*>&exts,node_t*r):
    m_graph(wg),m_nodes(nds),m_ext_nodes(exts),m_root(r){}

    double total_min()const{return m_total_min;}
    case_t opt_case()const{return m_opt_case_id;}
    void exam_node(node_t* node,double min_value,case_t case_id)
    {
        if(!m_min_delta[case_id]||min_value<*m_min_delta[case_id])
        {
            m_min_delta[case_id]=min_value;
            m_min_node[case_id]=node;
        }
    }
    void exam_edge(const edge_t&edge,double min_value,case_t case_id)
    {
        if(!m_min_delta[case_id]||min_value<*m_min_delta[case_id])
        {
            m_min_delta[case_id]=min_value;
            m_min_edge[case_id]=edge;
        }
    }
    bool define_opt_case()
    {
        m_opt_case_id=undefine_id;
        for(int i=0;i<5;++i)
        {
            if(m_min_delta[i]&&(m_opt_case_id==undefine_id||*m_min_delta[i]<m_total_min))
            {
                m_total_min=*m_min_delta[i];
                m_opt_case_id=(case_t)i;
            }
        }
        return m_opt_case_id!=undefine_id;
    }
    edge_t&opt_edge(){assert(m_opt_case_id!=undefine_id);return m_min_edge[m_opt_case_id];}
    node_t*opt_node(){assert(m_opt_case_id!=undefine_id);return m_min_node[m_opt_case_id];}

    void out()const
    {
        std::cout<<"optimal case:"<<m_opt_case_id<<'\n';
        std::cout<<"total min:"<<(m_opt_case_id!=undefine_id? m_total_min:999999)<<'\n';
        for(int i=0;i<5;++i)
        {
            std::cout<<"case:"<<i;
            std::cout<<"min:"<<(m_min_delta[i]? *m_min_delta[i]:999999)<<'\n';
        }
    }

    template<class visitor_t>
    void visit_tree_edge(visitor_t vis)
    {
        //std::cout<<"visit_tree_edge"<<'\n';
        for(node_t* top_from:m_ext_nodes)
        {
            //std::cout<<"top_from:"<<top_from->index<<'\n';
            assert(top_from->is_top());
            top_from->traverse_terminal_nodes([&](node_t*from)
            {
                assert(from->index<m_graph.size());
                for(size_t j=0;j<m_graph[from->index].size();++j)
                {
                    auto [to_ind,weight]=m_graph[from->index][j];
                    node_t*top_to=m_nodes[to_ind]->top_parent;
                    if(top_to==top_from) continue;
                    if(node_t*prev=top_to->prev_node();!prev&&top_to!=m_root->top_parent)
                    {
                        //std::cout<<"find_vertex_id:"<<from->index<<','<<j<<'\n';
                        exam_edge({from->index,j},vis(new_edge_t{},{from->index,j}),find_vertex_id);
                    }
                    else if(prev==top_to->match_node())
                    {
                        //std::cout<<"find_blossom_id:"<<from->index<<','<<j<<'\n';
                        exam_edge({from->index,j},vis(blossom_edge_t{},{from->index,j}),find_blossom_id);
                    }
                }
            });
        }
    }
};

template<class...ts_t>
struct tree_visitor_t:ts_t...
{
    using ts_t::operator()...;
};
template<class...ts_t>
tree_visitor_t(ts_t...)->tree_visitor_t<ts_t...>;

template<class visitor_t>
void visit_tree_node(std::vector<node_t*>&exts_top,visitor_t vis)
{
    for(node_t* top_from:exts_top)
    {
        top_from->traverse_terminal_nodes([&](node_t*node){vis(ext_node_t{},node);});
        if(top_from->is_blossom()) vis(ext_quasi_node_t{},top_from);

        if(node_t*m=top_from->match_node();m)
        {
            m->traverse_terminal_nodes([&](node_t*node){vis(int_node_t{},node);});
            if(m->is_blossom()) vis(int_quasi_node_t{},m);
        }
    }
}

}//blossom_ns


size_t nonbipartite_solver_t::max_cardinality(const graph_t&graph,std::vector<size_t>&match)
{
    using namespace blossom_ns;
    //std::cout<<"nonbipartite_solver_t::max_cardinality begin\n";

    size_t res=0;

    std::vector<node_t*> nodes;
    std::vector<node_t*> queue;
    std::vector<char>    in_queue(graph.size(),0);

    match.clear();
    for(size_t i=0;i<graph.size();++i)
    {
        nodes.push_back(new node_t(i));
        match.push_back(i);
    }

    auto drop_tree=[&]()
    {
        in_queue.assign(in_queue.size(),0);
        queue.clear();
        for(size_t i=graph.size();i<nodes.size();++i) delete nodes[i];
        nodes.resize(graph.size());
        for(node_t*n:nodes)
        {
            n->prev_node()=nullptr;
            n->parent=n->top_parent=n;
            if(size_t m=match[n->index];m!=n->index)
            {
                assert(match[m]==n->index);
                n->match_node()=nodes[m];
                n->match_edge()={n,nodes[m]};
            }
            else n->match_node()=nullptr;
        }
    };

    for(size_t root=0;root<graph.size();++root)
    {
        if(match[root]!=root) continue;
        queue.push_back(nodes[root]);
        in_queue[root]=1;
        //std::cout<<"root:"<<root<<'\n';
        while(!queue.empty())
        {
            node_t*from=queue.back();
            queue.pop_back();
            in_queue[from->index]=0;
            for(size_t to_ind:graph[from->index])
            {
                node_t*to=nodes[to_ind];
                node_t*top_to=to->top_parent;
                if(!to->prev_node())
                {
                    to->prev_node()=from->top_parent;
                    to->prev_edge()={to,from};
                    //std::cout<<"link:"<<to->index<<"->"<<from->index<<'\n';
                    if(node_t*m=to->match_node();m)
                    {
                        m->prev_node()=to;
                        m->prev_edge()={m,to};
                        //std::cout<<"match link:"<<m->index<<"->"<<to->index<<'\n';
                        assert(is_valid_blossom_graph(nodes,match));
                        queue.push_back(m);
                        in_queue[m->index]=1;
                    }
                    else
                    {
                        augment(nodes[root]->top_parent,to,match);
                        drop_tree();
                        assert(is_valid_blossom_graph(nodes,match));
                        res++;
                        break;
                    }
                }
                else if(top_to!=to&&top_to!=from->top_parent||
                       top_to==to&&to->prev_node()==to->match_node())
                {
                    node_t*blossom=make_blossom(from,to,nodes[root],nodes);
                    blossom->traverse_terminal_nodes([&](node_t*n)
                    {
                       if(!in_queue[n->index])
                       {
                           queue.push_back(n);
                           in_queue[n->index]=1;
                       }
                    });
                    assert(is_valid_blossom_graph(nodes,match));
                }
            }
        }
    }
    assert(is_valid_matching(graph,match));
    return res;
}

size_t nonbipartite_solver_t::max_cardinality(const graph_t&graph)
{
    std::vector<size_t> match;
    return max_cardinality(graph,match);
}

template<bool is_perfect>
std::optional<int> nonbipartite_solver_t::m_max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    using namespace blossom_ns;
    using edge_t=search_data_t::edge_t;
    using sdata_t=search_data_t;
    std::vector<node_t*> exts;
    std::vector<node_t*> nodes;
    std::vector<double>  pot;

    match.clear();
    for(size_t i=0;i<graph.size();++i)
    {
        nodes.push_back(new node_t(i));
        match.push_back(i);
        pot.push_back(0.0);
        for(auto [to,weight]:graph[i])
        {
            pot.back()=std::max(pot.back(),(double)weight);
        }
    }

    auto drop_tree=[&]()
    {
        for(node_t*n:nodes) if(n->is_top()) n->prev_node()=nullptr;
        exts.clear();
    };

    for(size_t root=0;root<graph.size();++root)
    {
        if(match[root]!=root) continue;
        exts.push_back(nodes[root]->top_parent);
        //std::cout<<"root "<<root<<'\n';
        for(bool tree_valid=true;tree_valid;)
        {
            sdata_t sdata(graph,nodes,exts,nodes[root]);
            sdata.visit_tree_edge(tree_visitor_t
            {
                [&](new_edge_t,const edge_t&e){return pot[e.fst()]+pot[e.snd(graph)]-e.wht(graph);},
                [&](blossom_edge_t,const edge_t&e){return (pot[e.fst()]+pot[e.snd(graph)]-e.wht(graph))/2.0;}
            });

            if constexpr(is_perfect)
            {
                visit_tree_node(exts,tree_visitor_t
                {
                    [&](int_quasi_node_t,node_t*node)
                    {
                        sdata.exam_node(node,pot[node->index]/2.0,sdata_t::delete_blossom_id);
                    },
                    [](auto,node_t*node){}
                });
            }
            else
            {
                visit_tree_node(exts,tree_visitor_t
                {
                    [&](ext_node_t,node_t*node)
                    {
                        sdata.exam_node(node,pot[node->index],sdata_t::expose_node_id);
                    },
                    [&](int_quasi_node_t,node_t*node)
                    {
                        sdata.exam_node(node,pot[node->index]/2.0,sdata_t::delete_blossom_id);
                    },
                    [&](auto,node_t*node){}
                });
            }
            bool is_def=sdata.define_opt_case();
            assert(is_def||is_perfect);
            if(!is_def) return {};

            visit_tree_node(exts,tree_visitor_t
            {
                [&](ext_node_t,node_t*node){pot[node->index]-=sdata.total_min();},
                [&](int_node_t,node_t*node){pot[node->index]+=sdata.total_min();},
                [&](ext_quasi_node_t,node_t*node){pot[node->index]+=2*sdata.total_min();},
                [&](int_quasi_node_t,node_t*node){pot[node->index]-=2*sdata.total_min();}
            });
            //sdata.out();
            //system("PAUSE");

            switch(sdata.opt_case())
            {
                case sdata_t::find_vertex_id:
                {
                    int from_ind=sdata.opt_edge().fst();
                    int to_ind=sdata.opt_edge().snd(graph);
                    node_t* top_to=nodes[to_ind]->top_parent;
                    top_to->prev_node()=nodes[from_ind]->top_parent;
                    top_to->prev_edge()={nodes[to_ind],nodes[from_ind]};
                    if(node_t*m=top_to->match_node();m)
                    {
                        assert(m->top_parent==m);
                        m->prev_node()=top_to;
                        m->prev_edge()=m->match_edge();
                        exts.push_back(m);

                    }
                    else
                    {
                        augment(nodes[root]->top_parent,top_to,match);
                        drop_tree();
                        tree_valid=false;
                        assert(is_valid_blossom_graph(nodes,match));
                    }
                }
                break;

                case sdata_t::find_blossom_id:
                {
                    size_t from_ind=sdata.opt_edge().fst();
                    size_t to_ind=sdata.opt_edge().snd(graph);
                    pot.push_back(0.0);
                    exts.push_back(make_blossom(nodes[from_ind],nodes[to_ind],nodes[root],nodes));
                    exts.erase(std::remove_if(exts.begin(),exts.end(),[](node_t*n){return n->top_parent!=n;}),exts.end());
                }
                break;

                case sdata_t::expose_node_id:
                {
                    assert(!is_perfect);
                    node_t*top=sdata.opt_node()->top_parent;
                    if(node_t*m=top->match_node();m)
                    {
                        augment(nodes[root]->top_parent,m,match);
                        top->match_node()=nullptr;
                    }
                    expose_node(top,sdata.opt_node(),match);
                    drop_tree();
                    tree_valid=false;
                    assert(is_valid_blossom_graph(nodes,match));
                }
                break;

                case sdata_t::delete_blossom_id:
                {
                    swap_remove(pot,sdata.opt_node()->index);
                    delete_blossom(sdata.opt_node(),nodes,match,exts);
                    assert(is_valid_blossom_graph(nodes,match));
                }
                break;

                default: assert(false);
            }
        }
    }
    assert(is_valid_matching(graph,match));
    return matching_weight(graph,match);
}

std::optional<int> nonbipartite_solver_t::perfect_max_weighted(const w_graph_t&graph)
{
    std::vector<size_t> match;
    return m_max_weighted<true>(graph,match);
}

std::optional<int> nonbipartite_solver_t::perfect_max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    return m_max_weighted<true>(graph,match);
}

int nonbipartite_solver_t::max_weighted(const w_graph_t&graph)
{
    std::vector<size_t> match;
    return *m_max_weighted<false>(graph,match);
}

int nonbipartite_solver_t::max_weighted(const w_graph_t&graph,std::vector<size_t>&match)
{
    return *m_max_weighted<false>(graph,match);
}


/*
int nonbipartite_solver_t::min_covering(const w_graph_t&graph,graph_t&cov)
{
    //std::cout<<"nonbipartite solver_t::min_covering\n";
    using namespace blossom_ns;
    using edge_t=search_data_t::edge_t;
    using sdata_t=search_data_t;
    std::vector<node_t*> exts;
    std::vector<node_t*> nodes;
    std::vector<size_t>  match;
    std::vector<double>  pot;

    cov.clear();
    for(size_t i=0;i<graph.size();++i)
    {
        nodes.push_back(new node_t(i));
        cov.push_back({});
        match.push_back(i);
        pot.push_back(0.0);
    }

    auto drop_tree=[&]()
    {
        for(node_t*n:nodes) if(n->is_top()) n->prev_node()=nullptr;
        exts.clear();
    };

    auto save_match=[&](node_t* n)
    {
        assert(!n->is_blossom());
        assert(n->match_node());
        assert(!find_edge(cov,n->index,n->match_edge().second->index));
        cov[n->index].push_back(n->match_edge().second->index);
        n->match_node()=nullptr;
        match[n->index]=n->index;
    };
    auto count_cover_edge=[&](node_t* n)->int
    {
        if(!n->match_node())
        {
            assert(cov[n->index].empty());
            return 0;
        }
        else
        {
            assert(cov[n->index].empty()||!n->is_blossom());
            return 1+cov[n->index].size();
        }
    };

    auto remove_match=[&](node_t* from,node_t* to)
    {
        assert(!from->is_blossom());
        assert(!cov[from->index].empty());
        if(from->match_node()==to)
        {
            node_t* low_level=nodes[cov[from->index].back()];
            assert(!low_level->is_blossom());
            from->match_node()=low_level->top_parent;
            from->match_edge()={from,low_level};
            cov[from->index].pop_back();
        }
        else
        {
            auto e=find_edge(cov,from->index,to->match_edge().first->index);
            assert(e);
            swap_remove(cov[from->index],*e-cov[from->index].begin());
        }
    };

    for(size_t root=0;root<graph.size();++root)
    {
        node_t* top_root=nodes[root]->top_parent;
        if(match[root]!=root||(top_root!=nodes[root]&&pot[root]==0.0)) continue;
        exts.push_back(nodes[root]);
        //std::cout<<"root "<<root<<'\n';
        for(bool tree_valid=true;tree_valid;)
        {
            sdata_t sdata(graph,nodes,exts,nodes[root]);
            sdata.visit_tree_edge(tree_visitor_t
            {
                [&](new_edge_t,const edge_t&e){return e.wht(graph)-pot[e.fst()]-pot[e.snd(graph)];},
                [&](blossom_edge_t,const edge_t&e){return (e.wht(graph)-pot[e.fst()]-pot[e.snd(graph)])/2.0;}
            });

            visit_tree_node(exts,tree_visitor_t
            {
                [&](int_quasi_node_t,node_t*node)
                {
                    sdata.exam_node(node,pot[node->index]/2.0,sdata_t::delete_blossom_id);
                },
                [&](int_node_t,node_t*node)
                {
                    if(node->is_top()) sdata.exam_node(node,pot[node->index],sdata_t::added_match_id);

                },
                [&](ext_node_t,node_t*node)
                {
                    if(!node->is_top()) sdata.exam_node(node,pot[node->index],sdata_t::expose_node_id);
                },
                [](auto,node_t*node){}
            });
            bool is_def=sdata.define_opt_case();
            assert(is_def);

            visit_tree_node(exts,tree_visitor_t
            {
                [&](ext_quasi_node_t,node_t*n){pot[n->index]+=2*sdata.total_min();},
                [&](int_quasi_node_t,node_t*n){pot[n->index]-=2*sdata.total_min();},
                [&](ext_node_t,node_t*n)
                {
                    pot[n->index]+=(n->is_top()? sdata.total_min():-sdata.total_min());
                },
                [&](int_node_t,node_t*n)
                {
                    pot[n->index]+=(n->is_top()? -sdata.total_min():sdata.total_min());
                }
            });

            switch(sdata.opt_case())
            {
                case sdata_t::find_vertex_id:
                {
                    //
                    //   new valid edge to out-of-tree node
                    //
                    size_t from_ind=sdata.opt_edge().fst();
                    size_t to_ind=sdata.opt_edge().snd(graph);
                    node_t* top_to=nodes[to_ind]->top_parent;
                    top_to->prev_node()=nodes[from_ind]->top_parent;
                    top_to->prev_edge()={nodes[to_ind],nodes[from_ind]};

                    if(const size_t covered=count_cover_edge(top_to);covered==0)
                    {
                        augment(top_root,top_to,match);
                        drop_tree();
                        tree_valid=false;
                    }
                    //
                    //    "to" node covered few edges: augment to-from and drop tree
                    //
                    else if(covered>1)
                    {
                        assert(!top_to->is_blossom());
                        assert(pot[top_to->index]==0);
                        save_match(top_to);
                        augment(top_root,top_to,match);
                        drop_tree();
                        tree_valid=false;
                    }
                    //
                    //    "to" and match[to] node covered one edge:
                    //    add match[to] to external
                    //
                    else if(node_t* matched=top_to->match_node();count_cover_edge(matched)==1)
                    {
                        assert(matched->match_node()==top_to);
                        matched->prev_node()=top_to;
                        matched->prev_edge()=matched->match_edge();
                        exts.push_back(matched);
                    }
                    //
                    //    "to" node covered one edge, but node matched with "to"
                    //    covered few edges: remove edge to-match[to],augment to-root
                    //    and drop tree
                    //
                    else
                    {
                        assert(!matched->is_blossom());
                        assert(pot[matched->index]==0);
                        assert(count_cover_edge(matched)>1);
                        remove_match(matched,top_to);
                        augment(top_root,top_to,match);
                        drop_tree();
                        tree_valid=false;
                    }
                }
                break;

                case sdata_t::find_blossom_id:
                {
                    int from_ind=sdata.opt_edge().fst();
                    int to_ind=sdata.opt_edge().snd(graph);
                    pot.push_back(0.0);
                    exts.push_back(make_blossom(nodes[from_ind],nodes[to_ind],nodes[root],nodes));
                    exts.erase(std::remove_if(exts.begin(),exts.end(),[](node_t*n){return n->top_parent!=n;}),exts.end());
                }
                break;

                case sdata_t::expose_node_id:
                {
                    //
                    //    external quasi-node removed from tree by reason
                    //    pot[one of owned node]==0
                    //

                    node_t*top=sdata.opt_node()->top_parent;
                    if(node_t*m=top->match_node();m)
                    {
                        augment(top_root,m,match);
                        top->match_node()=nullptr;
                    }
                    expose_node(top,sdata.opt_node(),match);
                    drop_tree();
                    tree_valid=false;
                    assert(is_valid_blossom_graph(nodes,match));
                }
                break;

                case sdata_t::delete_blossom_id:
                {
                    //
                    //    internal quasi-node decompose by reason
                    //    pot[quasi-node]==0
                    //

                    swap_remove(pot,sdata.opt_node()->index);
                    delete_blossom(sdata.opt_node(),nodes,match,exts);
                    assert(is_valid_blossom_graph(nodes,match));
                }
                break;

                case sdata_t::added_match_id:
                {
                    //
                    //    internal node become covered by two edges by reason
                    //    pot[node]==0
                    //
                    assert(pot[sdata.opt_node()->index]==0);
                    save_match(sdata.opt_node());
                    augment(top_root,sdata.opt_node(),match);
                    drop_tree();
                    tree_valid=false;
                }
                break;

                default: assert(false);

            }
        }
    }
    //
    //    some nodes with pot[node]==0 in quasi-nodes is unmatched
    //    it possible cover him two edges
    //

    for(size_t i=graph.size();i<nodes.size();++i)
    {
        if(nodes[i]->top_parent!=nodes[i]) continue;
        if(nodes[i]->match_node()) continue;
        node_t* un_matched=nullptr;
        nodes[i]->traverse_terminal_nodes([&](node_t*term)
        {
            if(match[term->index]==term->index)
            {
                assert(!un_matched);
                assert(pot[un_matched->index]==0);
                un_matched=term;
            }
        });
        if(un_matched)
        {
            cov[un_matched->index].push_back(un_matched->adj_nodes[0]->index);
            expose_node(nodes[i],un_matched->adj_nodes[0],match);
        }
    }

    for(size_t i=0;i<graph.size();++i)
    {
        assert(match[i]!=i);
        save_match(nodes[i]);
    }
    assert(is_valid_covering(graph,cov));
    return covering_weight(graph,cov);
}

int nonbipartite_solver_t::min_covering(const w_graph_t&graph)
{
    graph_t cov;
    return min_covering(graph,cov);
}
*/

}//matching

}// liga
