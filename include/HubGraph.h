// HubGraph.h  -- MODULE 2: Graph & Routing
// Owner: Rakesh Kumar Sanwal
//
// Topics covered:
//   DSA : graphs (adjacency list), Dijkstra's shortest path (priority queue),
//         Floyd-Warshall (all-pairs shortest path), Kruskal's Minimum Spanning
//         Tree with Union-Find, arrays/vectors, sorting
//   OOP : classes/objects, constructors, Strategy design pattern (abstract
//         class + polymorphism), exception handling
//   STL : vector, unordered_map, priority_queue, sort, unique_ptr
#ifndef VELORENT_HUBGRAPH_H
#define VELORENT_HUBGRAPH_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <stack>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Exceptions.h"
using namespace std;

// A route leaving one hub: destination id and its cost (distance in km).
struct Edge {
    int to;
    double cost;
};

struct PathResult {
    bool reachable = false;
    double cost = 0.0;
    vector<int> path;     // hub ids from source to destination
};

struct MSTEdge {
    int u, v;
    double cost;
};

struct MSTResult {
    vector<MSTEdge> edges;
    double totalCost = 0.0;
};

// ---------------------------------------------------------------------------
// Directed weighted graph: each city depot is a vertex, each route an edge.
// ---------------------------------------------------------------------------
class HubGraph {
    vector<string> hubNames;
    unordered_map<string, int> hubIds;
    vector<vector<Edge>> routes;               // adjacency list

    // Union-Find (disjoint set) used by Kruskal to detect cycles.
    struct DSU {
        vector<int> parent, rank;
        explicit DSU(int n) : parent(n), rank(n, 0) {
            for (int i = 0; i < n; ++i) {
                parent[i] = i;
            }
        }
        int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
        bool unite(int a, int b) {
            a = find(a);
            b = find(b);
            if (a == b) {
                return false;
                // already connected -> cycle
            }
            if (rank[a] < rank[b]) {
                swap(a, b);
            }
            parent[b] = a;
            if (rank[a] == rank[b]) {
                ++rank[a];
            }
            return true;
        }
    };

    void checkId(int id) const {
        if (id < 0 || id >= hubCount()) {
            throw NotFoundException("Hub id out of range");
        }
    }
    void dfsVisit(int u, vector<bool>& seen, vector<int>& order) const {
        seen[u] = true;
        order.push_back(u);
        for (const Edge& e : routes[u])
            if (!seen[e.to]) {
                dfsVisit(e.to, seen, order);
            }
    }

public:
    static constexpr double INF = numeric_limits<double>::infinity();

    int addHub(const string& name) {
        if (name.empty()) {
            throw InvalidInputException("Hub name cannot be empty");
        }
        auto it = hubIds.find(name);
        if (it != hubIds.end()) {
            return it->second;
            // already exists
        }
        int id = static_cast<int>(hubNames.size());
        hubNames.push_back(name);
        hubIds[name] = id;
        routes.emplace_back();
        return id;
    }

    int hubId(const string& name) const {
        auto it = hubIds.find(name);
        if (it == hubIds.end()) {
            throw NotFoundException("Unknown hub: " + name);
        }
        return it->second;
    }

    const string& hubName(int id) const { return hubNames.at(static_cast<size_t>(id)); }
    int hubCount() const { 
        return static_cast<int>(hubNames.size());
     }

    // Adds a one-way route; bidirectional=true also adds the reverse route.
    void addRoute(const string& from, const string& to, double cost,
                  bool bidirectional = false) {
        if (cost < 0) {
            throw GraphException("Route cost cannot be negative");
        }
        int a = addHub(from), b = addHub(to);
        routes[a].push_back({b, cost});
        if (bidirectional) {
            routes[b].push_back({a, cost});
        }
    }

    // ---- Dijkstra: cheapest path from src to dst, O((V+E) log V) ----------
    PathResult dijkstra(int src, int dst) const {
        int n = hubCount();
        if (src < 0 || src >= n || dst < 0 || dst >= n)
            throw NotFoundException("Hub id out of range");

        vector<double> dist(n, INF);
        vector<int> prev(n, -1);
        using QItem = pair<double, int>;          // (distance, vertex)
        priority_queue<QItem, vector<QItem>, greater<QItem>> pq;  // min-heap

        dist[src] = 0.0;
        pq.push({0.0, src});
        while (!pq.empty()) {
            double d = pq.top().first;
            int u = pq.top().second;
            pq.pop();
            if (d > dist[u]) {
                continue;
                // stale queue entry
            }
            if (u == dst) {
                break;
                // reached target
            }
            for (const Edge& e : routes[u]) {
                if (dist[u] + e.cost < dist[e.to]) {   // relax edge
                    dist[e.to] = dist[u] + e.cost;
                    prev[e.to] = u;
                    pq.push({dist[e.to], e.to});
                }
            }
        }

        PathResult res;
        if (dist[dst] == INF) {
            return res;
            // unreachable
        }
        res.reachable = true;
        res.cost = dist[dst];
        for (int at = dst; at != -1; at = prev[at]) {
            res.path.push_back(at);
        }
        reverse(res.path.begin(), res.path.end());
        return res;
    }

    PathResult dijkstra(const string& from, const string& to) const {
        return dijkstra(hubId(from), hubId(to));
    }

    // ---- Floyd-Warshall: shortest distance between EVERY pair, O(V^3) -----
    vector<vector<double>> floydWarshall() const {
        int n = hubCount();
        vector<vector<double>> d(n, vector<double>(n, INF));
        for (int i = 0; i < n; ++i) {
            d[i][i] = 0.0;
            for (const Edge& e : routes[i]) {
                d[i][e.to] = min(d[i][e.to], e.cost);
            }
        }
        for (int k = 0; k < n; ++k)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (d[i][k] + d[k][j] < d[i][j]) {
                        d[i][j] = d[i][k] + d[k][j];
                    }
        return d;
    }

    // ---- Kruskal: cheapest set of routes that links all hubs --------------
    // Routes are treated as undirected for the "branch network" plan.
    MSTResult kruskalMST() const {
        vector<MSTEdge> all;
        for (int u = 0; u < hubCount(); ++u)
            for (const Edge& e : routes[u]) {
                all.push_back({u, e.to, e.cost});
            }
        sort(all.begin(), all.end(),
                  [](const MSTEdge& a, const MSTEdge& b) { return a.cost < b.cost; });

        DSU dsu(hubCount());
        MSTResult res;
        for (const MSTEdge& e : all) {
            if (dsu.unite(e.u, e.v)) {                 // keep edge only if no cycle
                res.edges.push_back(e);
                res.totalCost += e.cost;
            }
        }
        return res;
    }

    // ---- BFS: hubs in order of fewest hops from src, O(V+E) ---------------
    // Uses a QUEUE. Follows one-way routes in their allowed direction.
    vector<int> bfs(int src) const {
        checkId(src);
        vector<bool> seen(hubCount(), false);
        vector<int> order;
        queue<int> q;
        seen[src] = true;
        q.push(src);
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (const Edge& e : routes[u])
                if (!seen[e.to]) {
                    seen[e.to] = true;
                    q.push(e.to);
                }
        }
        return order;
    }

    // ---- DFS: goes as deep as possible first, O(V+E) ----------------------
    // Uses recursion (the call stack acts as the stack).
    vector<int> dfs(int src) const {
        checkId(src);
        vector<bool> seen(hubCount(), false);
        vector<int> order;
        dfsVisit(src, seen, order);
        return order;
    }

    // ---- Prim: cheapest network linking all hubs, O(E log V) --------------
    // MST needs an UNDIRECTED graph, so every route is treated as two-way
    // and the cheaper cost is kept when both directions exist.
    MSTResult primMST() const {
        int n = hubCount();
        vector<vector<Edge>> und(n);
        for (int u = 0; u < n; ++u)
            for (const Edge& e : routes[u]) {
                und[u].push_back({e.to, e.cost});
                und[e.to].push_back({u, e.cost});
            }
        MSTResult res;
        if (n == 0) {
            return res;
        }
        vector<bool> inTree(n, false);
        typedef pair<double, pair<int, int> > Item;   // (cost, (from, to))
        priority_queue<Item, vector<Item>, greater<Item> > pq;
        inTree[0] = true;
        for (const Edge& e : und[0]) {
            pq.push(Item(e.cost, make_pair(0, e.to)));
        }
        while (!pq.empty()) {
            Item top = pq.top();
            pq.pop();
            int from = top.second.first, to = top.second.second;
            if (inTree[to]) {
                continue;
                // would make a cycle
            }
            inTree[to] = true;
            MSTEdge me = {from, to, top.first};
            res.edges.push_back(me);
            res.totalCost += top.first;
            for (const Edge& e : und[to])
                if (!inTree[e.to]) {
                    pq.push(Item(e.cost, make_pair(to, e.to)));
                }
        }
        return res;
    }

    // ---- Bellman-Ford: shortest distances from src, O(V*E) ----------------
    // Unlike Dijkstra it also works with NEGATIVE edge costs and detects a
    // negative cycle. 'edges' is a plain list so it can be tested with
    // negative weights (addRoute() itself only allows costs >= 0).
    struct BFResult {
        vector<double> dist;
        bool negativeCycle = false;
    };
    static BFResult bellmanFord(int n, const vector<MSTEdge>& edges, int src) {
        BFResult r;
        r.dist.assign(n, INF);
        r.dist[src] = 0.0;
        for (int i = 0; i < n - 1; ++i) {
            bool changed = false;
            for (const MSTEdge& e : edges)
                if (r.dist[e.u] != INF && r.dist[e.u] + e.cost < r.dist[e.v]) {
                    r.dist[e.v] = r.dist[e.u] + e.cost;
                    changed = true;
                }
            if (!changed) {
                break;
                // early stop
            }
        }
        for (const MSTEdge& e : edges)                 // one more pass: still improving?
            if (r.dist[e.u] != INF && r.dist[e.u] + e.cost < r.dist[e.v]) {
                r.negativeCycle = true;
            }
        return r;
    }
    BFResult bellmanFord(int src) const {
        checkId(src);
        vector<MSTEdge> edges;
        for (int u = 0; u < hubCount(); ++u)
            for (const Edge& e : routes[u]) {
                MSTEdge me = {u, e.to, e.cost};
                edges.push_back(me);
            }
        return bellmanFord(hubCount(), edges, src);
    }

    void print(ostream& os = cout) const {
        for (int u = 0; u < hubCount(); ++u) {
            os << "  " << left << setw(11) << hubNames[u] << "->";
            for (const Edge& e : routes[u]) {
                os << " " << hubNames[e.to] << "(" << e.cost << "km)";
            }
            os << "\n";
        }
    }
};

#if __cplusplus < 201703L
constexpr double HubGraph::INF;   // C++14 needs this out-of-class definition
#endif

// ---------------------------------------------------------------------------
// STRATEGY PATTERN: drop-off charge can be swapped at run time.
// DropOffCalculator holds *a* strategy and does not care which one.
// ---------------------------------------------------------------------------
class DropOffStrategy {
public:
    virtual ~DropOffStrategy() = default;
    virtual double charge(double routeKm) const = 0;
    virtual string name() const = 0;
};

class SameHubDrop : public DropOffStrategy {            // returned to the pickup hub
public:
    double charge(double) const override { return 0.0; }
    string name() const override { return "Same-hub return"; }
};

class OneWayDrop : public DropOffStrategy {             // Rs.200 base + Rs.12 per km
public:
    double charge(double routeKm) const override { return 200.0 + 12.0 * routeKm; }
    string name() const override { return "One-way drop-off"; }
};

class PeakSeasonDrop : public DropOffStrategy {         // one-way + 25% surcharge
public:
    double charge(double routeKm) const override { return (200.0 + 12.0 * routeKm) * 1.25; }
    string name() const override { return "Peak-season one-way"; }
};

class DropOffCalculator {
    unique_ptr<DropOffStrategy> strategy_;
public:
    explicit DropOffCalculator(unique_ptr<DropOffStrategy> s) { setStrategy(move(s)); }
    void setStrategy(unique_ptr<DropOffStrategy> s) {
        if (!s) {
            throw InvalidInputException("Strategy cannot be null");
        }
        strategy_ = move(s);
    }
    double calculate(double routeKm) const { return strategy_->charge(routeKm); }
    string strategyName() const { return strategy_->name(); }
};

#endif
