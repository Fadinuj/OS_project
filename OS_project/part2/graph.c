#include "graph.h"
#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>


/**
 * Check if vertex index v is within [0, g->n).
 * @param g Graph pointer (may be NULL).
 * @param v Vertex index to check.
 * @return Non-zero if valid; 0 otherwise.
 */
static int in_bounds(const Graph* g, int v) {
    return (g != NULL && v >= 0 && v < g->n);
}

/**
 * Count how many times v appears in u's adjacency list.
 * @param g Graph pointer (non-NULL).
 * @param u Source vertex.
 * @param v Neighbor to count in u's list.
 * @return Number of occurrences of v in u's adjacency list.
 */

static int count_neighbor(const Graph* g, int u, int v){
    int c = 0;
    for (EdgeNode* e = g->adj[u].head; e; e = e->next)
        if (e->to == v) c++;
    return c;
}


/**
 * Simple-graph duplicate check for undirected edge (u--v).
 *  - For u!=v: if u->v appears at least once, the undirected edge exists.
 *  - For u==v: one self-loop is represented by two u->u entries; if already 2, reject.
 * @param g Graph pointer (non-NULL).
 * @param u First vertex.
 * @param v Second vertex.
 * @return Non-zero if the (undirected) edge already exists under the policy; 0 otherwise.
 */

static int edge_exists_simple(const Graph* g, int u, int v){
    if (u == v) return count_neighbor(g, u, u) >= 2;
    return count_neighbor(g, u, v) >= 1;
}

/**
 * Create a graph with n vertices and no edges.
 * @param n Number of vertices (must be > 0).
 * @return Pointer to a new Graph, or NULL if n<=0 or allocation fails.
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
 * Destroy a graph and free all associated memory (safe for NULL).
 * @param g Graph pointer (may be NULL).
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
 * Add an undirected edge u--v under the simple-graph policy.
 * Allocates two adjacency nodes (u->v) and (v->u) on success.         
 * Rejects duplicates and a second self-loop.
 * @param g Graph pointer (non-NULL).
 * @param u Vertex index in [0,n-1].
 * @param v Vertex index in [0,n-1].
 * @return 0 on success; -1 out of bounds; -2 OOM; -3 duplicate/second self-loop.
 */
int graph_add_edge(Graph* g, int u, int v) {
    if (!in_bounds(g, u) || !in_bounds(g, v)) return -1;

    if (edge_exists_simple(g, u, v)) return -3;

    if (u == v) {
        EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
        EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
        if (!e1 || !e2) { free(e1); free(e2); return -2; }
        e1->to = u;
        e1->next = g->adj[u].head;  
        e2->to = u;
        e2->next = e1;            
        g->adj[u].head = e2;       
        return 0;
    } else {
        EdgeNode* e1 = (EdgeNode*)malloc(sizeof(EdgeNode));
        EdgeNode* e2 = (EdgeNode*)malloc(sizeof(EdgeNode));
        if (!e1 || !e2) { free(e1); free(e2); return -2; }

        e1->to = v; e1->next = g->adj[u].head;
        e2->to = u; e2->next = g->adj[v].head;

        g->adj[u].head = e1;
        g->adj[v].head = e2;
        return 0;
    }
}


/**
 *  Print adjacency lists to stdout. One line per vertex.
 * @param g Graph pointer (NULL is ignored).
 */
void graph_print(const Graph* g) {
    if (!g) return;
    for (int i = 0; i < g->n; ++i) {
        printf("%d:", i);
        for (EdgeNode* cur = g->adj[i].head; cur; cur = cur->next) {
            printf(" %d", cur->to);
        }
        printf("\n");
    }
}

/**
 * 
 * Vec
 *   A tiny dynamic array of int.
 *   Used as:
    - a stack during the Hierholzer walk,
     - the output path buffer,
     - per-vertex incidence lists (incid[v]) in EdgeView.
 *   Fields:
 *     - a   : pointer to the storage (int*), or NULL when empty
 *     - n   : current number of elements
 *     - cap : current capacity (>= n), 0 when empty
 1.  v_reserve(Vec* v, int cap)
 *   Ensures the vector has at least 'cap' capacity.
 *   Grows geometrically (×2, min 16). On failure, 'v' is unchanged.
 *   Returns: 0 on success; -1 on allocation failure.
 2.  v_push(Vec* v, int x)
 *   Appends 'x' to the end of the vector, growing if needed.
 *   Returns: 0 on success; -1 on allocation failure.
 3. v_pop(Vec* v)
 *   Removes and returns the last element.
 4. v_back(const Vec* v)
 *   Returns (without removing) the last element.
 5. v_free(Vec* v)
 *   Frees the storage and resets the vector to an empty state:
 *   a = NULL, n = 0, cap = 0.
 6. v_reverse(Vec* v)
 *   Reverses the elements in place.
 */
typedef struct { int *a; int n, cap; } Vec;
static int  v_reserve(Vec* v, int cap){ if (cap<=v->cap) return 0; int c=v->cap? v->cap*2:16; if(c<cap) c=cap; int*na=(int*)realloc(v->a, sizeof(int)*c); if(!na) return -1; v->a=na; v->cap=c; return 0; }
static int  v_push(Vec* v, int x){ if(v_reserve(v,v->n+1)) return -1; v->a[v->n++]=x; return 0; }
static int  v_pop (Vec* v){ return v->a[--v->n]; }
static int  v_back(const Vec* v){ return v->a[v->n-1]; }
static void v_free(Vec* v){ free(v->a); v->a=NULL; v->n=v->cap=0; }
static void v_reverse(Vec* v){ for(int i=0,j=v->n-1;i<j;i++,j--){ int t=v->a[i]; v->a[i]=v->a[j]; v->a[j]=t; } }
/**
 * EdgeView
 * A temporary, deduplicated “edge view” used by Hierholzer’s algorithm so  each undirected edge has a single ID and can be marked as used once.
 *Fields:
 *-edges : array of UEEdge (length m), each edge appears exactly once
 *- m     : number of undirected edges
 *- incid : array (length n) of Vec; incid[x] holds indices of edges that touch vertex x
 *- n     : number of vertices
 * For a normal edge u--v: insert exactly once (typically when u < v),     and append its index to incid[u] and incid[v].
 * For a self-loop u==v: pair the two adjacency entries (u->u, u->u)
 into one UEEdge {u,u}, and append that edge index twice to incid[u]
 (so the loop contributes degree +2 and is discoverable from u).
 */
typedef struct { int u, v; } UEEdge;
typedef struct {
    UEEdge* edges; int m;  
    Vec* incid;   int n;   
} EdgeView;

/**
 * Free all allocations owned by an EdgeView.
 * @param ev EdgeView pointer.
 */
static void ev_free(EdgeView* ev){
    if(!ev) return;
    for(int i=0;i<ev->n;i++) v_free(&ev->incid[i]);
    free(ev->incid);
    free(ev->edges);
}

/**
 * Compute degree of vertex v from adjacency lists.
 * A self-loop contributes +2 (since two u->u entries are stored).
 * @param g Graph pointer.
 * @param v Vertex index.
 * @return Degree of v.
 */
static int degree_vertex_adj(const Graph* g, int v){
    int d=0; for(EdgeNode* e=g->adj[v].head; e; e=e->next) d++;
    return d; 
}

/**
 * Check connectivity ignoring isolated vertices.
 * Returns 1 if all vertices with degree>0 belong to a single connected         component (or if the graph has no edges); 0 otherwise.
 * @param g Graph pointer.
 * @return 1 if connected (ignoring isolated) or no edges; 0 otherwise.
 */
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

/**
 *  Build an EdgeView from adjacency lists.
 *  - Pairs every two u->u directed entries into a single undirected loop (u,u)
 *    and pushes that edge index twice into incid[u].
 *  - For u!=v, inserts edge (min,max) exactly once and adds its index to
 *    both incid[u] and incid[v].
 * @param g Graph pointer.
 * @param ev OUT: EdgeView to fill.
 * @return 0 on success; -1 on allocation failure (ev is cleaned up).
 */
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

/**
 * @brief Check if the graph has an Euler circuit (undirected).
 * @details Returns 1 if
 *  - the subgraph induced by vertices of degree>0 is connected, and
 *  - all vertex degrees are even, and
 *  - there is at least one edge.
 * @param g Graph pointer.
 * @return 1 if Euler circuit exists, else 0.
 */
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



/**
 *  Find an Euler circuit using Hierholzer's algorithm.
 *  - Builds an EdgeView and maintains:
 *      used[m] to mark used edges,
 *      it[n]  as per-vertex iterator over incid lists,
 *      stack  for the forward walk,
 *      path   to collect the final circuit in reverse.
 *  - When a vertex has no more unused incident edges, it is popped to path.
 *  - Finally, path is reversed to obtain the circuit order.
 * @param g Graph pointer.
 * @param out_cycle OUT: allocated array of vertices in order (caller frees).
 * @param out_len   OUT: number of vertices in out_cycle (should be m+1).
 * @return 1 on success; 0 if no Euler circuit or on failure.
 */

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
