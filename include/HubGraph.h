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
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Exceptions.h"

// A route leaving one hub: destination id and its cost (distance in km).
struct Edge {
    int to;
    double cost;
};

struct PathResult {
    bool reachable = false;
    double cost = 0.0;
    std::vector<int> path;     // hub ids from source to destination
};

struct MSTEdge {
    int u, v;
    double cost;
};

struct MSTResult {
    std::vector<MSTEdge> edges;
    double totalCost = 0.0;
};

// ---------------------------------------------------------------------------
// Directed weighted graph: each city depot is a vertex, each route an edge.
// ---------------------------------------------------------------------------
class HubGraph {
    std::vector<std::string> names_;
    std::unordered_map<std::string, int> idOf_;
    std::vector<std::vector<Edge>> adj_;               // adjacency list

    // Union-Find (disjoint set) used by Kruskal to detect cycles.
    struct DSU {
        std::vector<int> parent, rank;
        explicit DSU(int n) : parent(n), rank(n, 0) {
            for (int i = 0; i < n; ++i) parent[i] = i;
        }
        int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }
        bool unite(int a, int b) {
            a = find(a);
            b = find(b);
            if (a == b) return false;                  // already connected -> cycle
            if (rank[a] < rank[b]) std::swap(a, b);
            parent[b] = a;
            if (rank[a] == rank[b]) ++rank[a];
            return true;
        }
    };

public:
    static const double INF;

    int addHub(const std::string& name) {
        if (name.empty()) throw InvalidInputException("Hub name cannot be empty");
        auto it = idOf_.find(name);
        if (it != idOf_.end()) return it->second;      // already exists
        int id = static_cast<int>(names_.size());
        names_.push_back(name);
        idOf_[name] = id;
        adj_.emplace_back();
        return id;
    }

    int hubId(const std::string& name) const {
        auto it = idOf_.find(name);
        if (it == idOf_.end()) throw NotFoundException("Unknown hub: " + name);
        return it->second;
    }

    const std::string& hubName(int id) const { return names_.at(static_cast<std::size_t>(id)); }
    int hubCount() const { return static_cast<int>(names_.size()); }

    // Adds a one-way route; bidirectional=true also adds the reverse route.
    void addRoute(const std::string& from, const std::string& to, double cost,
                  bool bidirectional = false) {
        if (cost < 0) throw GraphException("Route cost cannot be negative");
        int a = addHub(from), b = addHub(to);
        adj_[a].push_back({b, cost});
        if (bidirectional) adj_[b].push_back({a, cost});
    }

    // ---- Dijkstra: cheapest path from src to dst, O((V+E) log V) ----------
    PathResult dijkstra(int src, int dst) const {
        int n = hubCount();
        if (src < 0 || src >= n || dst < 0 || dst >= n)
            throw NotFoundException("Hub id out of range");

        std::vector<double> dist(n, INF);
        std::vector<int> prev(n, -1);
        using QItem = std::pair<double, int>;          // (distance, vertex)
        std::priority_queue<QItem, std::vector<QItem>, std::greater<QItem>> pq;  // min-heap

        dist[src] = 0.0;
        pq.push({0.0, src});
        while (!pq.empty()) {
            auto item = pq.top();
            int d = item.first;
            int u = item.second;
            pq.pop();
            if (d > dist[u]) continue;                 // stale queue entry
            if (u == dst) break;                       // reached target
            for (const Edge& e : adj_[u]) {
                if (dist[u] + e.cost < dist[e.to]) {   // relax edge
                    dist[e.to] = dist[u] + e.cost;
                    prev[e.to] = u;
                    pq.push({dist[e.to], e.to});
                }
            }
        }

        PathResult res;
        if (dist[dst] == INF) return res;              // unreachable
        res.reachable = true;
        res.cost = dist[dst];
        for (int at = dst; at != -1; at = prev[at]) res.path.push_back(at);
        std::reverse(res.path.begin(), res.path.end());
        return res;
    }

    PathResult dijkstra(const std::string& from, const std::string& to) const {
        return dijkstra(hubId(from), hubId(to));
    }

    // ---- Floyd-Warshall: shortest distance between EVERY pair, O(V^3) -----
    std::vector<std::vector<double>> floydWarshall() const {
        int n = hubCount();
        std::vector<std::vector<double>> d(n, std::vector<double>(n, INF));
        for (int i = 0; i < n; ++i) {
            d[i][i] = 0.0;
            for (const Edge& e : adj_[i]) d[i][e.to] = std::min(d[i][e.to], e.cost);
        }
        for (int k = 0; k < n; ++k)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (d[i][k] + d[k][j] < d[i][j]) d[i][j] = d[i][k] + d[k][j];
        return d;
    }

    // ---- Kruskal: cheapest set of routes that links all hubs --------------
    // Routes are treated as undirected for the "branch network" plan.
    MSTResult kruskalMST() const {
        std::vector<MSTEdge> all;
        for (int u = 0; u < hubCount(); ++u)
            for (const Edge& e : adj_[u]) all.push_back({u, e.to, e.cost});
        std::sort(all.begin(), all.end(),
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

    void print(std::ostream& os = std::cout) const {
        for (int u = 0; u < hubCount(); ++u) {
            os << "  " << std::left << std::setw(11) << names_[u] << "->";
            for (const Edge& e : adj_[u]) os << " " << names_[e.to] << "(" << e.cost << "km)";
            os << "\n";
        }
    }
};

// ---------------------------------------------------------------------------
// STRATEGY PATTERN: drop-off charge can be swapped at run time.
// DropOffCalculator holds *a* strategy and does not care which one.
// ---------------------------------------------------------------------------
class DropOffStrategy {
public:
    virtual ~DropOffStrategy() = default;
    virtual double charge(double routeKm) const = 0;
    virtual std::string name() const = 0;
};

class SameHubDrop : public DropOffStrategy {            // returned to the pickup hub
public:
    double charge(double) const override { return 0.0; }
    std::string name() const override { return "Same-hub return"; }
};

class OneWayDrop : public DropOffStrategy {             // Rs.200 base + Rs.12 per km
public:
    double charge(double routeKm) const override { return 200.0 + 12.0 * routeKm; }
    std::string name() const override { return "One-way drop-off"; }
};

class PeakSeasonDrop : public DropOffStrategy {         // one-way + 25% surcharge
public:
    double charge(double routeKm) const override { return (200.0 + 12.0 * routeKm) * 1.25; }
    std::string name() const override { return "Peak-season one-way"; }
};

class DropOffCalculator {
    std::unique_ptr<DropOffStrategy> strategy_;
public:
    explicit DropOffCalculator(std::unique_ptr<DropOffStrategy> s) { setStrategy(std::move(s)); }
    void setStrategy(std::unique_ptr<DropOffStrategy> s) {
        if (!s) throw InvalidInputException("Strategy cannot be null");
        strategy_ = std::move(s);
    }
    double calculate(double routeKm) const { return strategy_->charge(routeKm); }
    std::string strategyName() const { return strategy_->name(); }
};


const double HubGraph::INF = std::numeric_limits<double>::infinity();

#endif
