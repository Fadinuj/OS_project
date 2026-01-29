#include "graph.h"
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>

/**
 * Check if vertex index v is within [0, g->n).
 */
static int in_bounds(const Graph* g, int v) {
    return (g != NULL && v >= 0 && v < g->n);
}

/**
 * Count how many times v appears in u's adjacency list.
 */
static int count_neighbor(const Graph* g, int u, int v){
    int c = 0;
    for (EdgeNode* e = g->adj[u].head; e; e = e->next)
        if (e->to == v) c++;
    return c;
}

/**
 * Simple-graph duplicate check for undirected edge (u--v).
 */
static int edge_exists_simple(const Graph* g, int u, int v){
    if (u == v) return count_neighbor(g, u, u) >= 2;
    return count_neighbor(g, u, v) >= 1;
}

/**
 * Create a graph with n vertices and no edges.
 */
Graph* graph_create(int n) {
    if (n <= 0) return NULL;

    Graph* g = (Graph*)malloc(sizeof(Graph));
    if (!g) return NULL;

    g->n = n;
    g->adj = (Vertex*)calloc((size_t)n, sizeof(Vertex));
    if (!g->adj) { free(g); return NULL; }

    return g;
}

/**
 * Destroy a graph and free all associated memory.
 */
void graph_destroy(Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->n; ++i) {
        EdgeNode* cur = g->adj[i].head;
        while (cur) {
            EdgeNode* tmp = cur;
            cur = cur->next;
            free(tmp);
        }
    }
    free(g->adj);
    free(g);
}

/**
 * Add an undirected edge u--v with default weight 1.
 * Backward compatible with existing code.
 */
int graph_add_edge(Graph* g, int u, int v) {
    return graph_add_weighted_edge(g, u, v, 1);
}

/**
 * Add an undirected edge u--v with specified weight.
 */
int graph_add_weighted_edge(Graph* g, int u, int v, int weight) {
    if (!in_bounds(g, u) || !in_bounds(g, v)) return -1;

    if (edge_exists_simple(g, u, v)) return -3;

    if (u == v) {
        // Self-loop: add two entries with same weight
        EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
        EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
        if (!e1 || !e2) { free(e1); free(e2); return -2; }
        
        e1->to = u;
        e1->weight = weight;
        e1->next = g->adj[u].head;
        
        e2->to = u;
        e2->weight = weight;
        e2->next = e1;
        
        g->adj[u].head = e2;
        return 0;
    } else {
        // Regular edge: add two directed edges
        EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
        EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
        if (!e1 || !e2) { free(e1); free(e2); return -2; }

        e1->to = v;
        e1->weight = weight;
        e1->next = g->adj[u].head;
        
        e2->to = u;
        e2->weight = weight;
        e2->next = g->adj[v].head;

        g->adj[u].head = e1;
        g->adj[v].head = e2;
        return 0;
    }
}

/**
 * Get weight of edge between u and v.
 */
int graph_get_edge_weight(const Graph* g, int u, int v) {
    if (!in_bounds(g, u) || !in_bounds(g, v)) return 0;
    
    for (EdgeNode* e = g->adj[u].head; e; e = e->next) {
        if (e->to == v) {
            return e->weight;
        }
    }
    return 0; // Edge doesn't exist
}

/**
 * Check if graph has any weighted edges (weight != 1).
 */
static int has_weights(const Graph* g) {
    if (!g) return 0;
    
    for (int i = 0; i < g->n; i++) {
        for (EdgeNode* e = g->adj[i].head; e; e = e->next) {
            if (e->weight != 1) return 1;
        }
    }
    return 0;
}

/**
 * Print adjacency lists to stdout with optional weights.
 */
void graph_print(const Graph* g) {
    if (!g) return;
    
    int show_weights = has_weights(g);
    
    for (int i = 0; i < g->n; ++i) {
        printf("%d:", i);
        for (EdgeNode* cur = g->adj[i].head; cur; cur = cur->next) {
            if (show_weights) {
                printf(" %d(w:%d)", cur->to, cur->weight);
            } else {
                printf(" %d", cur->to);
            }
        }
        printf("\n");
    }
}

/* 
 * Rest of the file remains exactly the same - all the Euler circuit code,
 * Vec implementation, EdgeView, etc. - since they don't need to know about weights
 */

/**
 * Vec - tiny dynamic array implementation
 */
typedef struct { int *a; int n, cap; } Vec;
static int  v_reserve(Vec* v, int cap){ if (cap<=v->cap) return 0; int c=v->cap? v->cap*2:16; if(c<cap) c=cap; int*na=(int*)realloc(v->a, sizeof(int)*c); if(!na) return -1; v->a=na; v->cap=c; return 0; }
static int  v_push(Vec* v, int x){ if(v_reserve(v,v->n+1)) return -1; v->a[v->n++]=x; return 0; }
static int  v_pop (Vec* v){ return v->a[--v->n]; }
static int  v_back(const Vec* v){ return v->a[v->n-1]; }
static void v_free(Vec* v){ free(v->a); v->a=NULL; v->n=v->cap=0; }
static void v_reverse(Vec* v){ for(int i=0,j=v->n-1;i<j;i++,j--){ int t=v->a[i]; v->a[i]=v->a[j]; v->a[j]=t; } }

/**
 * EdgeView for Hierholzer's algorithm
 */
typedef struct { int u, v; } UEEdge;
typedef struct {
    UEEdge* edges; int m;  
    Vec* incid;   int n;   
} EdgeView;

static void ev_free(EdgeView* ev){
    if(!ev) return;
    for(int i=0;i<ev->n;i++) v_free(&ev->incid[i]);
    free(ev->incid);
    free(ev->edges);
}

static int degree_vertex_adj(const Graph* g, int v){
    int d=0; for(EdgeNode* e=g->adj[v].head; e; e=e->next) d++;
    return d; 
}

static int is_connected_ignore_isolated(const Graph* g){
    int start = -1;
    for(int i=0;i<g->n;i++)
        if (degree_vertex_adj(g,i)>0) { start=i; break; }
    if (start==-1) return 1;

    char* vis = (char*)calloc((size_t)g->n, 1);
    if(!vis) return 0;
    Vec st={0};
    (void)v_push(&st, start); vis[start]=1;

    while(st.n){
        int u = v_pop(&st);
        for(EdgeNode* e=g->adj[u].head; e; e=e->next){
            int v = e->to;
            if(!vis[v]){ vis[v]=1; (void)v_push(&st, v); }
        }
    }
    int ok=1;
    for(int i=0;i<g->n;i++)
        if (degree_vertex_adj(g,i)>0 && !vis[i]) { ok=0; break; }

    v_free(&st); free(vis);
    return ok;
}

static int build_edge_view(const Graph* g, EdgeView* ev){
    ev->n = g->n;
    ev->edges = NULL; ev->m = 0;
    ev->incid = (Vec*)calloc((size_t)ev->n, sizeof(Vec));
    if(!ev->incid) return -1;

    long long sumdeg = 0;
    for(int u=0; u<g->n; ++u)
        for(EdgeNode* e=g->adj[u].head; e; e=e->next) sumdeg++;
    int m_est = (int)(sumdeg/2 + 1);

    ev->edges = (UEEdge*)malloc(sizeof(UEEdge) * (size_t)m_est);
    if(!ev->edges){ ev_free(ev); return -1; }

    int* loop_half = (int*)calloc((size_t)ev->n, sizeof(int));
    if(!loop_half){ ev_free(ev); return -1; }

    for(int u=0; u<g->n; ++u){
        for(EdgeNode* e=g->adj[u].head; e; e=e->next){
            int v = e->to;

            if (u == v) {
                if ((++loop_half[u] & 1) == 0) {
                    if(ev->m == m_est){
                        m_est = m_est ? m_est*2 : 16;
                        UEEdge* ne = (UEEdge*)realloc(ev->edges, sizeof(UEEdge)*(size_t)m_est);
                        if(!ne){ 
                            free(loop_half);
                             ev_free(ev); 
                             return -1; 
                        }
                        ev->edges = ne;
                    }
                    ev->edges[ev->m] = (UEEdge){u,u};
                    if (v_push(&ev->incid[u], ev->m) || v_push(&ev->incid[u], ev->m)) {
                        free(loop_half); ev_free(ev); return -1;
                    }
                    ev->m++;
                }
            } else if (u < v) {
                if(ev->m == m_est){
                    m_est = m_est ? m_est*2 : 16;
                    UEEdge* ne = (UEEdge*)realloc(ev->edges, sizeof(UEEdge)*(size_t)m_est);
                    if(!ne){ free(loop_half); ev_free(ev); return -1; }
                    ev->edges = ne;
                }
                ev->edges[ev->m] = (UEEdge){u,v};
                if (v_push(&ev->incid[u], ev->m) || v_push(&ev->incid[v], ev->m)) {
                    free(loop_half); ev_free(ev); return -1;
                }
                ev->m++;
            }
        }
    }
    free(loop_half);
    return 0;
}

int graph_has_euler_circuit(const Graph* g){
    if (!g) return 0;

    if (!is_connected_ignore_isolated(g)) return 0;

    long long sumdeg = 0;
    for (int i = 0; i < g->n; ++i){
        int d = degree_vertex_adj(g, i);
        if (d % 2 != 0) return 0;
        sumdeg += d;
    }
    if (sumdeg == 0) return 0; 

    return 1;
}

int graph_find_euler_circuit(const Graph* g, int** out_cycle, int* out_len){
    if (!g || !out_cycle || !out_len) return 0;
    *out_cycle = NULL; *out_len = 0;

    if (!graph_has_euler_circuit(g)) return 0;

    EdgeView ev; if (build_edge_view(g, &ev)) return 0;

    int start = -1;
    for (int i = 0; i < ev.n; ++i){
        if (ev.incid[i].n > 0) { start = i; break; }
    }
    if (start == -1) { ev_free(&ev); return 0; }

    int* used = (int*)calloc((size_t)ev.m, sizeof(int));
    int* it   = (int*)calloc((size_t)ev.n, sizeof(int));
    if(!used || !it){ free(used); free(it); ev_free(&ev); return 0; }

    Vec stack={0}, path={0};
    (void)v_push(&stack, start);

    while (stack.n){
        int u = v_back(&stack);

        while (it[u] < ev.incid[u].n && used[ ev.incid[u].a[it[u]] ]) it[u]++;

        if (it[u] == ev.incid[u].n){
            (void)v_push(&path, u);
            (void)v_pop(&stack);
        } else {
            int ei = ev.incid[u].a[it[u]++];
            if (!used[ei]) {
                used[ei] = 1;
                int a = ev.edges[ei].u, b = ev.edges[ei].v;
                int v = (u==a) ? b : a;
                (void)v_push(&stack, v);
            }
        }
    }

    v_reverse(&path);

    free(used); free(it);
    ev_free(&ev);
    v_free(&stack);

    if (path.n < 1) { v_free(&path); return 0; }

    *out_cycle = path.a; 
    *out_len   = path.n;

    return 1;
}