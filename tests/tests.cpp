// tests.cpp -- unit tests for all four VeloRent modules
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>

#include "Booking.h"
#include "HubGraph.h"
#include "RateTree.h"
#include "Vehicle.h"
using namespace std;

static int passed = 0;
static int failed = 0;

#define CHECK(name, cond)                              \
    do {                                               \
        if (cond) {                                    \
            ++passed;                                  \
            cout << "[PASS] " << name << "\n";         \
        } else {                                       \
            ++failed;                                  \
            cout << "[FAIL] " << name << "\n";         \
        }                                              \
    } while (0)

template <typename Ex, typename F>
static bool throwsType(F fn) {
    try {
        fn();
    } catch (const Ex&) {
        return true;
    } catch (...) {
        return false;
    }
    return false;
}

static bool nearEqual(double a, double b) {
    return fabs(a - b) < 1e-9;
}

// ------------------------------ Module 1 ------------------------------------
static void testModule1() {
    cout << "\n--- Module 1: Fleet Core (Paras) ---\n";
    Car c("V1", "P1", "Maruti", 1000);
    ElectricVehicle e("V2", "P2", "Tata", 2000, 40, 80);
    LuxuryBus b("V3", "P3", "Volvo", 9000, 40, 1000);
    Bike k("V4", "P4", "Hero", 500);

    CHECK("Car tariff = rate x days", nearEqual(c.calculateTariff(3), 3000));
    CHECK("EV tariff adds Rs.120 charging fee", nearEqual(e.calculateTariff(2), 4120));
    CHECK("Bus tariff includes driver per day", nearEqual(b.calculateTariff(2), 20000));
    CHECK("Bike gets 10% discount at 7 days", nearEqual(k.calculateTariff(7), 3150));
    CHECK("Bike has no discount below 7 days", nearEqual(k.calculateTariff(6), 3000));
    CHECK("Polymorphism through Vehicle*", [] {
        unique_ptr<Vehicle> v = make_unique<Car>("X", "Y", "Z", 100);
        return nearEqual(v->calculateTariff(2), 200);
    }());
    CHECK("operator< compares daily rate", c < e);
    CHECK("Zero days rejected", throwsType<InvalidInputException>([&] { c.calculateTariff(0); }));
    CHECK("Negative rate rejected", throwsType<InvalidInputException>([] { Car("A", "B", "C", -5); }));

    VehicleHashTable table(4);
    bool insertedAll = true;
    vector<unique_ptr<Car>> cars;
    for (int i = 0; i < 200; ++i) {
        cars.push_back(make_unique<Car>("V" + to_string(i), "P" + to_string(i), "B", 100 + i));
        insertedAll = insertedAll && table.insert("V" + to_string(i), cars.back().get());
    }
    CHECK("Hash table inserts 200 keys", insertedAll && table.size() == 200);
    CHECK("Hash table rehashed (load factor <= 0.75)", table.loadFactor() <= 0.75 && table.bucketCount() > 4);

    bool foundAll = true;
    for (int i = 0; i < 200; ++i) {
        foundAll = foundAll && (table.find("V" + to_string(i)) == cars[i].get());
    }
    CHECK("Hash table finds every key after rehash", foundAll);
    CHECK("Hash table: missing key -> nullptr", table.find("nope") == nullptr);
    CHECK("Hash table rejects duplicate key", !table.insert("V5", cars[5].get()));
    CHECK("Hash table remove works", table.remove("V5") && table.find("V5") == nullptr && table.size() == 199);
    CHECK("Hash table remove of missing key is false", !table.remove("V5"));

    Fleet fleet;
    fleet.add(make_unique<Car>("A", "PA", "x", 3000));
    fleet.add(make_unique<Car>("B", "PB", "x", 1000));
    fleet.add(make_unique<Car>("C", "PC", "x", 2000));
    fleet.add(make_unique<Car>("D", "PD", "x", 500));

    CHECK("Fleet: duplicate VIN rejected", throwsType<DuplicateKeyException>([&] { fleet.add(make_unique<Car>("A", "PZ", "x", 1)); }));
    CHECK("Fleet: duplicate plate rejected", throwsType<DuplicateKeyException>([&] { fleet.add(make_unique<Car>("Z", "PA", "x", 1)); }));
    CHECK("Fleet: find by plate", fleet.findByPlate("PC") && fleet.findByPlate("PC")->getVIN() == "C");
    CHECK("Fleet: unknown VIN throws NotFound", throwsType<NotFoundException>([&] { fleet.getByVin("??"); }));

    vector<Vehicle*> sorted = fleet.sortedByRate();
    CHECK("Recursive merge sort orders by rate",
          sorted[0]->getVIN() == "D" && sorted[1]->getVIN() == "B" && sorted[2]->getVIN() == "C" && sorted[3]->getVIN() == "A");
    CHECK("Recursive binary search finds rate 2000", Fleet::binarySearchRate(sorted, 2000) == 2);
    CHECK("Recursive binary search miss returns -1", Fleet::binarySearchRate(sorted, 1234) == -1);

    vector<Vehicle*> emptyList;
    Fleet::mergeSort(emptyList);
    CHECK("Merge sort handles empty input", emptyList.empty());
}

// ------------------------------ Module 2 ------------------------------------
static void testModule2() {
    cout << "\n--- Module 2: Graph & Routing (Rakesh) ---\n";
    HubGraph g;
    g.addRoute("A", "B", 4, true);
    g.addRoute("A", "C", 1, true);
    g.addRoute("C", "B", 2, true);
    g.addRoute("B", "D", 5, true);
    g.addRoute("D", "E", 3, false);
    g.addHub("Island");

    PathResult r = g.dijkstra("A", "B");
    CHECK("Dijkstra picks cheaper 2-hop route (cost 3)", r.reachable && nearEqual(r.cost, 3) && r.path.size() == 3);
    CHECK("Dijkstra path is A->C->B", g.hubName(r.path[0]) == "A" && g.hubName(r.path[1]) == "C" && g.hubName(r.path[2]) == "B");
    CHECK("Dijkstra to itself costs 0", nearEqual(g.dijkstra("A", "A").cost, 0));
    CHECK("Dijkstra respects direction (E cannot reach D)", !g.dijkstra("E", "D").reachable);
    CHECK("Dijkstra: unreachable hub", !g.dijkstra("A", "Island").reachable);
    CHECK("Dijkstra: A->E = 1+2+5+3 = 11", nearEqual(g.dijkstra("A", "E").cost, 11));
    CHECK("Unknown hub throws NotFound", throwsType<NotFoundException>([&] { g.dijkstra("A", "Mars"); }));
    CHECK("Negative route cost rejected", throwsType<GraphException>([&] { g.addRoute("A", "B", -1); }));

    vector<vector<double>> fw = g.floydWarshall();
    int a = g.hubId("A");
    int b = g.hubId("B");
    int e = g.hubId("E");
    CHECK("Floyd-Warshall A->B = 3", nearEqual(fw[a][b], 3));
    CHECK("Floyd-Warshall agrees with Dijkstra (A->E)", nearEqual(fw[a][e], g.dijkstra(a, e).cost));
    CHECK("Floyd-Warshall unreachable = INF", fw[e][a] == HubGraph::INF);

    MSTResult m = g.kruskalMST();
    CHECK("Kruskal MST total = 1+2+3+5 = 11", nearEqual(m.totalCost, 11));
    CHECK("Kruskal MST has V-2 edges (Island is isolated)", m.edges.size() == 4);

    DropOffCalculator calc(make_unique<OneWayDrop>());
    CHECK("One-way strategy: 200 + 12*100 = 1400", nearEqual(calc.calculate(100), 1400));
    calc.setStrategy(make_unique<SameHubDrop>());
    CHECK("Same-hub strategy is free", nearEqual(calc.calculate(100), 0));
    calc.setStrategy(make_unique<PeakSeasonDrop>());
    CHECK("Peak strategy = one-way x 1.25 = 1750", nearEqual(calc.calculate(100), 1750));
    CHECK("Strategy swapped at run time (name changes)", calc.strategyName() == "Peak-season one-way");

    vector<int> bfsOrder = g.bfs(g.hubId("A"));
    CHECK("BFS from A visits A,B,C first (1 hop), then D, then E",
          bfsOrder.size() == 5 && g.hubName(bfsOrder[0]) == "A" && g.hubName(bfsOrder[3]) == "D" && g.hubName(bfsOrder[4]) == "E");
    CHECK("BFS does not reach the isolated hub", find(bfsOrder.begin(), bfsOrder.end(), g.hubId("Island")) == bfsOrder.end());

    vector<int> dfsOrder = g.dfs(g.hubId("A"));
    CHECK("DFS from A reaches the same 5 hubs", dfsOrder.size() == 5 && g.hubName(dfsOrder[0]) == "A");
    CHECK("DFS bad id throws NotFound", throwsType<NotFoundException>([&] { g.dfs(99); }));

    MSTResult prim = g.primMST();
    CHECK("Prim total equals Kruskal total (11)", nearEqual(prim.totalCost, m.totalCost) && nearEqual(prim.totalCost, 11));
    CHECK("Prim spans the 5 connected hubs (4 edges)", prim.edges.size() == 4);

    HubGraph::BFResult bf = g.bellmanFord(g.hubId("A"));
    CHECK("Bellman-Ford agrees with Dijkstra (A->E = 11)", nearEqual(bf.dist[g.hubId("E")], 11) && !bf.negativeCycle);
    CHECK("Bellman-Ford: unreachable hub stays INF", bf.dist[g.hubId("Island")] == HubGraph::INF);

    vector<MSTEdge> negEdges = {{0, 1, 4}, {0, 2, 5}, {1, 2, -3}};
    HubGraph::BFResult negRes = HubGraph::bellmanFord(3, negEdges, 0);
    CHECK("Bellman-Ford handles a negative edge (0->2 = 1)", nearEqual(negRes.dist[2], 1) && !negRes.negativeCycle);

    vector<MSTEdge> cycEdges = {{0, 1, 1}, {1, 2, -3}, {2, 1, 1}};
    CHECK("Bellman-Ford detects a negative cycle", HubGraph::bellmanFord(3, cycEdges, 0).negativeCycle);
}

// ------------------------------ Module 3 ------------------------------------
static void testModule3() {
    cout << "\n--- Module 3: Search & Trees (Brijesh) ---\n";
    AVLTree<RateEntry> tree;
    for (int i = 1; i <= 1000; ++i) {
        tree.insert(RateEntry{static_cast<double>(i), "V" + to_string(i)});
    }
    CHECK("AVL holds 1000 sorted inserts", tree.size() == 1000);
    CHECK("AVL stays balanced (height <= 14, a plain BST would be 1000)", tree.height() <= 14);
    CHECK("AVL contains existing key", tree.contains(RateEntry{500, "V500"}));
    CHECK("AVL does not contain missing key", !tree.contains(RateEntry{500.5, "V?"}));
    CHECK("AVL ignores duplicates", !tree.insert(RateEntry{10, "V10"}) && tree.size() == 1000);
    CHECK("AVL min / max", nearEqual(tree.findMin().rate, 1) && nearEqual(tree.findMax().rate, 1000));

    vector<RateEntry> inList = tree.inorder();
    bool isSorted = true;
    for (size_t i = 1; i < inList.size(); ++i) {
        isSorted = isSorted && (inList[i - 1] < inList[i]);
    }
    CHECK("In-order traversal is sorted", isSorted);

    vector<RateEntry> sub = rangeByRate(tree, 100, 105);
    CHECK("Range query 100..105 returns 6 items", sub.size() == 6 && nearEqual(sub.front().rate, 100) && nearEqual(sub.back().rate, 105));
    CHECK("Range query with no match is empty", rangeByRate(tree, 2000, 3000).empty());
    CHECK("Bad range throws", throwsType<InvalidInputException>([&] { rangeByRate(tree, 10, 5); }));

    AVLTree<RateEntry> smallTree;
    smallTree.insert({3, "c"});
    smallTree.insert({1, "a"});
    smallTree.insert({2, "b"});
    CHECK("AVL rotation: 3 keys give height 2", smallTree.height() == 2);

    AVLTree<RateEntry> emptyTree;
    CHECK("Empty tree findMin throws", throwsType<NotFoundException>([&] { emptyTree.findMin(); }));

    AVLTree<int> intTree;
    intTree.insert(5);
    intTree.insert(2);
    intTree.insert(8);
    CHECK("AVLTree<int> works (templates)", intTree.contains(8) && !intTree.contains(9));

    HuffmanCoder huff;
    string text = "aaaaabbbcc d";
    huff.build(text);
    string bits = huff.encode(text);
    CHECK("Huffman round trip (decode(encode(x)) == x)", huff.decode(bits) == text);
    CHECK("Huffman compresses repetitive text", bits.size() < text.size() * 8);
    CHECK("Frequent char gets a code no longer than a rare char", huff.codes().at('a').size() <= huff.codes().at('d').size());

    HuffmanCoder singleChar;
    singleChar.build("zzzz");
    CHECK("Huffman single-character text works", singleChar.decode(singleChar.encode("zzzz")) == "zzzz");
    CHECK("Huffman empty text rejected", throwsType<InvalidInputException>([&] { singleChar.build(""); }));
}

// ------------------------------ Module 4 ------------------------------------
static void testModule4() {
    cout << "\n--- Module 4: State & Durability (Tianshu) ---\n";
    BookingHeap heap;
    heap.push({3, "r3", "V", 1, REGULAR, 0});
    heap.push({2, "v2", "V", 1, VIP, 0});
    heap.push({1, "r1", "V", 1, REGULAR, 0});
    heap.push({4, "v4", "V", 1, VIP, 0});
    CHECK("Heap pops VIP first, earliest id first", heap.pop().id == 2 && heap.pop().id == 4);
    CHECK("Heap then serves regular by id", heap.pop().id == 1 && heap.pop().id == 3 && heap.empty());
    CHECK("Pop from empty heap throws", throwsType<NotFoundException>([&] { heap.pop(); }));

    BookingQueue q;
    q.enqueue({1, "a", "V", 1, REGULAR, 0});
    q.enqueue({2, "b", "V", 1, REGULAR, 0});
    CHECK("Queue is FIFO", q.front().id == 1 && q.dequeue().id == 1 && q.dequeue().id == 2 && q.empty());
    CHECK("Dequeue from empty queue throws", throwsType<NotFoundException>([&] { q.dequeue(); }));

    UndoStack st;
    st.push({1, "a", "V", 1, REGULAR, 0});
    st.push({2, "b", "V", 1, REGULAR, 0});
    CHECK("Stack is LIFO", st.top().id == 2 && st.pop().id == 2 && st.pop().id == 1 && st.empty());
    CHECK("Pop from empty stack throws", throwsType<NotFoundException>([&] { st.pop(); }));

    BookingManager mgr;
    int r1 = mgr.submit("Reg1", "V1", 2, REGULAR, 100);
    int r2 = mgr.submit("Reg2", "V2", 2, REGULAR, 100);
    int v1 = mgr.submit("VIP1", "V3", 2, VIP, 100);
    CHECK("Manager serves VIP before earlier regular", mgr.processNext().id == v1);
    CHECK("Manager then serves regulars in arrival order", mgr.processNext().id == r1);
    CHECK("Pending count updates", mgr.pendingCount() == 1 && mgr.confirmed().size() == 2);
    CHECK("Undo rolls back the latest confirmed booking", mgr.undoLast().id == r1 && mgr.confirmed().size() == 1);
    CHECK("Undo twice walks back (LIFO)", mgr.undoLast().id == v1 && mgr.confirmed().empty() && !mgr.canUndo());
    CHECK("Undo with nothing confirmed throws", throwsType<NotFoundException>([&] { mgr.undoLast(); }));
    (void)r2;
    CHECK("Empty customer rejected", throwsType<InvalidInputException>([&] { mgr.submit("", "V", 1, VIP, 1); }));
    CHECK("Zero days rejected", throwsType<InvalidInputException>([&] { mgr.submit("x", "V", 0, VIP, 1); }));

    const string path = "test_ledger.bin";
    Ledger led(path);
    led.clear();
    CHECK("Empty ledger loads nothing", led.loadAll().empty());

    led.append({1, "Rakesh Sanwal", "VIN1001", 3, VIP, 4500.5});
    led.append({2, "Asha", "VIN2001", 2, REGULAR, 5120});
    vector<Booking> saved = led.loadAll();
    CHECK("Ledger round trip keeps 2 records", saved.size() == 2);
    CHECK("Ledger keeps every field", saved[0].id == 1 && saved[0].customer == "Rakesh Sanwal" && saved[0].vin == "VIN1001" &&
                                          saved[0].days == 3 && saved[0].tier == VIP && nearEqual(saved[0].amount, 4500.5));
    {
        ofstream f(path, ios::binary | ios::app);
        f.write("garbage", 7);
    }
    CHECK("Half-written last record is ignored after a crash", led.loadAll().size() == 2);

    led.saveAll({saved[1]});
    CHECK("saveAll rewrites the file", led.loadAll().size() == 1 && led.loadAll()[0].id == 2);
    {
        led.clear();
        led.append({9, string(100, 'N'), string(100, 'V'), 1, VIP, 1});
        CHECK("Over-long strings are truncated safely", led.loadAll().size() == 1 && led.loadAll()[0].customer.size() == 31);
    }
    CHECK("Unwritable path throws FileIOException",
          throwsType<FileIOException>([] { Ledger("/nonexistent_dir/x.bin").append({1, "a", "b", 1, VIP, 1}); }));
    remove(path.c_str());
}

int main() {
    testModule1();
    testModule2();
    testModule3();
    testModule4();
    cout << "\n=============================\n"
         << "Passed: " << passed << "   Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
