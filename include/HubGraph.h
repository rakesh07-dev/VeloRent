
// Topics covered:
//   DSA : graphs (adjacency list), Dijkstra's shortest path (priority queue),
//         Floyd-Warshall (all-pairs shortest path), Kruskal's Minimum Spanning
//         Tree with Union-Find, sorting, arrays/vectors
//   OOP : classes/objects, constructors, inheritance, Strategy design pattern
//         (abstract class + polymorphism), custom exceptions, exception handling
//   STL : vector, unordered_map, priority_queue, sort, reverse, unique_ptr
#ifndef VELORENT_HUBGRAPH_H
#define VELORENT_HUBGRAPH_H

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using namespace std;

// ===========================================================================
// 1. CUSTOM EXCEPTIONS (inheritance): one base class, three specific errors
// ===========================================================================
class VeloException : public runtime_error {
public:
    explicit VeloException(const string& message) : runtime_error(message) {}
};

class InvalidInputException : public VeloException {   // bad value typed by the user
public:
    explicit InvalidInputException(const string& message) : VeloException(message) {}
};

class NotFoundException : public VeloException {       // hub does not exist
public:
    explicit NotFoundException(const string& message) : VeloException(message) {}
};

class GraphException : public VeloException {          // e.g. negative route cost
public:
    explicit GraphException(const string& message) : VeloException(message) {}
};

// ===========================================================================
// 2. SMALL DATA TYPES
// ===========================================================================
// A route leaving one hub: destination id and its cost (distance in km).
struct Edge {
    int to;
    double cost;
};

struct PathResult {
    bool reachable = false;
    double cost = 0.0;
    vector<int> path;          // hub ids from source to destination
};

struct MSTEdge {
    int u, v;
    double cost;
};

struct MSTResult {
    vector<MSTEdge> edges;
    double totalCost = 0.0;
};

// ===========================================================================
// 3. HubGraph: directed weighted graph
//    each city depot (hub) is a vertex, each route is an edge with a cost in km
// ===========================================================================
class HubGraph {
    vector<string> names_;                    // id -> hub name
    unordered_map<string, int> idOf_;         // hub name -> id
    vector<vector<Edge>> adj_;                // adjacency list: adj_[u] = routes leaving u

    // Union-Find (disjoint set): tells Kruskal whether two hubs are already connected.
    struct DSU {
        vector<int> parent, rank;
        explicit DSU(int n) : parent(n), rank(n, 0) {
            for (int i = 0; i < n; ++i) parent[i] = i;
        }
        int find(int x) { return parent[x] == x ? x : parent[x] = find(parent[x]); }  // path compression
        bool unite(int a, int b) {
            a = find(a);
            b = find(b);
            if (a == b) return false;                  // already connected -> would make a cycle
            if (rank[a] < rank[b]) swap(a, b);         // union by rank: attach smaller tree
            parent[b] = a;
            if (rank[a] == rank[b]) ++rank[a];
            return true;
        }
    };

public:
    static const double INF;                  // "infinity" = no path (defined after the class)

    int addHub(const string& name) {
        if (name.empty()) throw InvalidInputException("Hub name cannot be empty");
        auto it = idOf_.find(name);
        if (it != idOf_.end()) return it->second;      // hub already exists
        int id = static_cast<int>(names_.size());
        names_.push_back(name);
        idOf_[name] = id;
        adj_.emplace_back();
        return id;
    }

    int hubId(const string& name) const {
        auto it = idOf_.find(name);
        if (it == idOf_.end()) throw NotFoundException("Unknown hub: " + name);
        return it->second;
    }

    const string& hubName(int id) const { return names_.at(static_cast<size_t>(id)); }
    int hubCount() const { return static_cast<int>(names_.size()); }

    // Adds a one-way route; bidirectional = true also adds the reverse route.
    void addRoute(const string& from, const string& to, double cost, bool bidirectional = false) {
        if (cost < 0) throw GraphException("Route cost cannot be negative");
        int a = addHub(from), b = addHub(to);
        adj_[a].push_back({b, cost});
        if (bidirectional) adj_[b].push_back({a, cost});
    }

    // ---- Dijkstra: cheapest path from src to dst, O((V+E) log V) ------------
    // Idea: always settle the closest unsettled hub next, then "relax" its routes.
    PathResult dijkstra(int src, int dst) const {
        int n = hubCount();
        if (src < 0 || src >= n || dst < 0 || dst >= n)
            throw NotFoundException("Hub id out of range");

        vector<double> dist(n, INF);          // best known cost from src
        vector<int> prev(n, -1);              // previous hub on the best path
        typedef pair<double, int> QItem;      // (distance, hub)
        priority_queue<QItem, vector<QItem>, greater<QItem> > pq;   // min-heap: smallest distance on top

        dist[src] = 0.0;
        pq.push(QItem(0.0, src));
        while (!pq.empty()) {
            QItem item = pq.top();
            pq.pop();
            double d = item.first;
            int u = item.second;
            if (d > dist[u]) continue;                 // stale queue entry, skip
            if (u == dst) break;                       // reached the target
            for (size_t i = 0; i < adj_[u].size(); ++i) {
                const Edge& e = adj_[u][i];
                if (dist[u] + e.cost < dist[e.to]) {   // relax: found a cheaper way to e.to
                    dist[e.to] = dist[u] + e.cost;
                    prev[e.to] = u;
                    pq.push(QItem(dist[e.to], e.to));
                }
            }
        }

        PathResult res;
        if (dist[dst] == INF) return res;              // unreachable
        res.reachable = true;
        res.cost = dist[dst];
        for (int at = dst; at != -1; at = prev[at]) res.path.push_back(at);   // walk back to src
        reverse(res.path.begin(), res.path.end());
        return res;
    }

    PathResult dijkstra(const string& from, const string& to) const {
        return dijkstra(hubId(from), hubId(to));
    }

    // ---- Floyd-Warshall: shortest distance between EVERY pair, O(V^3) -------
    // Idea: for each hub k, ask "is going via k cheaper for any pair (i, j)?"
    vector<vector<double> > floydWarshall() const {
        int n = hubCount();
        vector<vector<double> > d(n, vector<double>(n, INF));
        for (int i = 0; i < n; ++i) {
            d[i][i] = 0.0;
            for (size_t k = 0; k < adj_[i].size(); ++k)
                d[i][adj_[i][k].to] = min(d[i][adj_[i][k].to], adj_[i][k].cost);
        }
        for (int k = 0; k < n; ++k)
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j)
                    if (d[i][k] + d[k][j] < d[i][j]) d[i][j] = d[i][k] + d[k][j];
        return d;
    }

    // ---- Kruskal: cheapest set of routes that links all hubs ----------------
    // Idea: sort routes by cost, keep a route only if it joins two separate groups.
    // Routes are treated as undirected for this "branch network" plan.
    static bool cheaper(const MSTEdge& a, const MSTEdge& b) { return a.cost < b.cost; }

    MSTResult kruskalMST() const {
        vector<MSTEdge> all;
        for (int u = 0; u < hubCount(); ++u)
            for (size_t i = 0; i < adj_[u].size(); ++i) {
                MSTEdge e = {u, adj_[u][i].to, adj_[u][i].cost};
                all.push_back(e);
            }
        sort(all.begin(), all.end(), cheaper);

        DSU dsu(hubCount());
        MSTResult res;
        for (size_t i = 0; i < all.size(); ++i) {
            if (dsu.unite(all[i].u, all[i].v)) {       // keep the route only if it makes no cycle
                res.edges.push_back(all[i]);
                res.totalCost += all[i].cost;
            }
        }
        return res;
    }

    void print(ostream& os = cout) const {
        for (int u = 0; u < hubCount(); ++u) {
            os << "  " << left << setw(11) << names_[u] << "->";
            for (size_t i = 0; i < adj_[u].size(); ++i)
                os << " " << names_[adj_[u][i].to] << "(" << adj_[u][i].cost << "km)";
            os << "\n";
        }
    }
};

// Static member must be defined once, outside the class.
const double HubGraph::INF = numeric_limits<double>::infinity();

// ===========================================================================
// 4. STRATEGY PATTERN: the drop-off charge rule can be swapped at run time.
//    DropOffCalculator holds *a* strategy and does not care which one it is.
// ===========================================================================
class DropOffStrategy {
public:
    virtual ~DropOffStrategy() {}
    virtual double charge(double routeKm) const = 0;      // every strategy must implement these
    virtual string name() const = 0;
};

class SameHubDrop : public DropOffStrategy {              // returned to the pickup hub: free
public:
    double charge(double) const { return 0.0; }
    string name() const { return "Same-hub return"; }
};

class OneWayDrop : public DropOffStrategy {               // Rs.200 base + Rs.12 per km
public:
    double charge(double routeKm) const { return 200.0 + 12.0 * routeKm; }
    string name() const { return "One-way drop-off"; }
};

class PeakSeasonDrop : public DropOffStrategy {           // one-way price + 25% surcharge
public:
    double charge(double routeKm) const { return (200.0 + 12.0 * routeKm) * 1.25; }
    string name() const { return "Peak-season one-way"; }
};

class DropOffCalculator {
    unique_ptr<DropOffStrategy> strategy_;
public:
    explicit DropOffCalculator(unique_ptr<DropOffStrategy> s) { setStrategy(move(s)); }
    void setStrategy(unique_ptr<DropOffStrategy> s) {
        if (!s) throw InvalidInputException("Strategy cannot be null");
        strategy_ = move(s);
    }
    double calculate(double routeKm) const { return strategy_->charge(routeKm); }
    string strategyName() const { return strategy_->name(); }
};

// ===========================================================================
// 5. DEMO DRIVER (console menu for the viva)
// ===========================================================================
static double toNumber(const string& s) {
    size_t used = 0;
    double v = 0;
    try {
        v = stod(s, &used);
    } catch (const logic_error&) {
        throw InvalidInputException("Please enter a number");
    }
    if (used != s.size()) throw InvalidInputException("Please enter a number");
    return v;
}

static string ask(const string& prompt) {
    string line;
    cout << prompt;
    getline(cin, line);
    return line;
}

static void loadSampleMap(HubGraph& g) {
    g.addRoute("Dehradun", "Rishikesh", 42, true);
    g.addRoute("Rishikesh", "Haridwar", 24, true);
    g.addRoute("Dehradun", "Haridwar", 54, true);
    g.addRoute("Dehradun", "Mussoorie", 35, true);
    g.addRoute("Haridwar", "Roorkee", 32, true);
    g.addRoute("Roorkee", "Delhi", 175, true);
    g.addRoute("Haridwar", "Delhi", 215, true);
    g.addRoute("Dehradun", "Chandigarh", 170, true);
    g.addRoute("Chandigarh", "Delhi", 250, false);      // one-way: shows the graph is DIRECTED
    g.addRoute("Delhi", "Chandigarh", 245, false);
}

int main() {
    HubGraph hubs;
    loadSampleMap(hubs);

    cout << "VeloRent - Module 2: Graph & Routing  (" << hubs.hubCount() << " hubs loaded)\n";
    for (;;) {
        cout << "\n=========== Module 2 ===========\n"
                " 1. Show all hubs and routes\n"
                " 2. Cheapest route + drop-off charge (Dijkstra + Strategy)\n"
                " 3. Branch network (Kruskal minimum spanning tree)\n"
                " 4. All-pairs distance table (Floyd-Warshall)\n"
                " 5. Add a new route\n"
                " 0. Exit\n"
                "Choice: ";
        string line;
        if (!getline(cin, line)) break;                       // end of input
        try {
            int choice = static_cast<int>(toNumber(line));
            if (choice == 0) break;

            if (choice == 1) {
                hubs.print();

            } else if (choice == 2) {
                hubs.print();
                string from = ask("From hub (type the name exactly): ");
                string to = ask("To hub: ");
                PathResult r = hubs.dijkstra(from, to);
                if (!r.reachable) throw NotFoundException("No route from " + from + " to " + to);
                cout << "Cheapest route: ";
                for (size_t i = 0; i < r.path.size(); ++i)
                    cout << (i ? " -> " : "") << hubs.hubName(r.path[i]);
                cout << "  (" << r.cost << " km)\n";

                int mode = static_cast<int>(toNumber(ask("Pricing  1=Same hub  2=One-way  3=Peak season: ")));
                unique_ptr<DropOffStrategy> s;
                if (mode == 1) s.reset(new SameHubDrop());
                else if (mode == 3) s.reset(new PeakSeasonDrop());
                else s.reset(new OneWayDrop());
                DropOffCalculator calc(move(s));                 // strategy chosen at run time
                cout << calc.strategyName() << " charge: Rs." << calc.calculate(r.cost) << "\n";

            } else if (choice == 3) {
                MSTResult mst = hubs.kruskalMST();
                cout << "Cheapest set of routes linking all hubs (Kruskal):\n";
                for (size_t i = 0; i < mst.edges.size(); ++i)
                    cout << "  " << hubs.hubName(mst.edges[i].u) << " - " << hubs.hubName(mst.edges[i].v)
                         << "  " << mst.edges[i].cost << " km\n";
                cout << "Total network length: " << mst.totalCost << " km\n";

            } else if (choice == 4) {
                vector<vector<double> > d = hubs.floydWarshall();
                int n = hubs.hubCount();
                cout << "Shortest distance in km between every pair of hubs ('-' = no route):\n";
                cout << setw(12) << " ";
                for (int j = 0; j < n; ++j) cout << setw(8) << hubs.hubName(j).substr(0, 7);
                cout << "\n";
                for (int i = 0; i < n; ++i) {
                    cout << setw(12) << left << hubs.hubName(i) << right;
                    for (int j = 0; j < n; ++j) {
                        if (d[i][j] == HubGraph::INF) cout << setw(8) << "-";
                        else cout << setw(8) << d[i][j];
                    }
                    cout << "\n";
                }
                cout << left;

            } else if (choice == 5) {
                string from = ask("From hub: ");
                string to = ask("To hub: ");
                double km = toNumber(ask("Distance in km: "));
                string two = ask("Two-way route? (y/n): ");
                hubs.addRoute(from, to, km, two == "y" || two == "Y");
                cout << "Route added. Hubs now: " << hubs.hubCount() << "\n";

            } else {
                cout << "Please choose 0-5.\n";
            }
        } catch (const exception& e) {
            cout << "Error: " << e.what() << "\n";            // clean message, program keeps running
        }
    }
    cout << "Goodbye.\n";
    return 0;
}

#endif