// tests.cpp -- simple self-checking tests for every module.
// Build & run with:  make test
#include <cmath>
#include <cstdio>
#include <iostream>
#include <memory>

#include "Booking.h"
#include "HubGraph.h"
#include "RateTree.h"
#include "Vehicle.h"

static int passed = 0, failed = 0;

#define CHECK(name, cond)                                              \
    do {                                                               \
        if (cond) { ++passed; std::cout << "[PASS] " << name << "\n"; } \
        else { ++failed; std::cout << "[FAIL] " << name << "\n"; }      \
    } while (0)

template <typename Ex, typename F>
static bool throwsType(F f) {
    try { f(); } catch (const Ex&) { return true; } catch (...) { return false; }
    return false;
}

static bool near(double a, double b) { return std::fabs(a - b) < 1e-9; }

// ------------------------------ Module 1 ------------------------------------
static void testModule1() {
    std::cout << "\n--- Module 1: Fleet Core (Paras) ---\n";
    Car c("V1", "P1", "Maruti", 1000);
    ElectricVehicle e("V2", "P2", "Tata", 2000, 40, 80);
    LuxuryBus b("V3", "P3", "Volvo", 9000, 40, 1000);
    Bike k("V4", "P4", "Hero", 500);
    CHECK("Car tariff = rate x days", near(c.calculateTariff(3), 3000));
    CHECK("EV tariff adds Rs.120 charging fee", near(e.calculateTariff(2), 4120));
    CHECK("Bus tariff includes driver per day", near(b.calculateTariff(2), 20000));
    CHECK("Bike gets 10% discount at 7 days", near(k.calculateTariff(7), 3150));
    CHECK("Bike has no discount below 7 days", near(k.calculateTariff(6), 3000));
    CHECK("Polymorphism through Vehicle*", [] {
        std::unique_ptr<Vehicle> v = std::make_unique<Car>("X", "Y", "Z", 100);
        return near(v->calculateTariff(2), 200);
    }());
    CHECK("operator< compares daily rate", c < e);
    CHECK("Zero days rejected", throwsType<InvalidInputException>([&] { c.calculateTariff(0); }));
    CHECK("Negative rate rejected", throwsType<InvalidInputException>([] { Car("A", "B", "C", -5); }));

    VehicleHashTable t(4);
    bool ok = true;
    std::vector<std::unique_ptr<Car>> cars;
    for (int i = 0; i < 200; ++i) {
        cars.push_back(std::make_unique<Car>("V" + std::to_string(i), "P" + std::to_string(i), "B", 100 + i));
        ok = ok && t.insert("V" + std::to_string(i), cars.back().get());
    }
    CHECK("Hash table inserts 200 keys", ok && t.size() == 200);
    CHECK("Hash table rehashed (load factor <= 0.75)", t.loadFactor() <= 0.75 && t.bucketCount() > 4);
    bool all = true;
    for (int i = 0; i < 200; ++i) all = all && t.find("V" + std::to_string(i)) == cars[i].get();
    CHECK("Hash table finds every key after rehash", all);
    CHECK("Hash table: missing key -> nullptr", t.find("nope") == nullptr);
    CHECK("Hash table rejects duplicate key", !t.insert("V5", cars[5].get()));
    CHECK("Hash table remove works", t.remove("V5") && t.find("V5") == nullptr && t.size() == 199);
    CHECK("Hash table remove of missing key is false", !t.remove("V5"));

    Fleet f;
    f.add(std::make_unique<Car>("A", "PA", "x", 3000));
    f.add(std::make_unique<Car>("B", "PB", "x", 1000));
    f.add(std::make_unique<Car>("C", "PC", "x", 2000));
    f.add(std::make_unique<Car>("D", "PD", "x", 500));
    CHECK("Fleet: duplicate VIN rejected", throwsType<DuplicateKeyException>([&] { f.add(std::make_unique<Car>("A", "PZ", "x", 1)); }));
    CHECK("Fleet: duplicate plate rejected", throwsType<DuplicateKeyException>([&] { f.add(std::make_unique<Car>("Z", "PA", "x", 1)); }));
    CHECK("Fleet: find by plate", f.findByPlate("PC") && f.findByPlate("PC")->getVIN() == "C");
    CHECK("Fleet: unknown VIN throws NotFound", throwsType<NotFoundException>([&] { f.getByVin("??"); }));
    auto sorted = f.sortedByRate();
    CHECK("Recursive merge sort orders by rate",
          sorted[0]->getVIN() == "D" && sorted[1]->getVIN() == "B" && sorted[2]->getVIN() == "C" && sorted[3]->getVIN() == "A");
    CHECK("Recursive binary search finds rate 2000", Fleet::binarySearchRate(sorted, 2000) == 2);
    CHECK("Recursive binary search miss returns -1", Fleet::binarySearchRate(sorted, 1234) == -1);
    std::vector<Vehicle*> empty;
    Fleet::mergeSort(empty);
    CHECK("Merge sort handles empty input", empty.empty());
}

// ------------------------------ Module 2 ------------------------------------
static void testModule2() {
    std::cout << "\n--- Module 2: Graph & Routing (Rakesh) ---\n";
    HubGraph g;
    g.addRoute("A", "B", 4, true);
    g.addRoute("A", "C", 1, true);
    g.addRoute("C", "B", 2, true);
    g.addRoute("B", "D", 5, true);
    g.addRoute("D", "E", 3, false);           // one-way
    g.addHub("Island");                       // unreachable hub

    PathResult r = g.dijkstra("A", "B");
    CHECK("Dijkstra picks cheaper 2-hop route (cost 3)", r.reachable && near(r.cost, 3) && r.path.size() == 3);
    CHECK("Dijkstra path is A->C->B", g.hubName(r.path[0]) == "A" && g.hubName(r.path[1]) == "C" && g.hubName(r.path[2]) == "B");
    CHECK("Dijkstra to itself costs 0", near(g.dijkstra("A", "A").cost, 0));
    CHECK("Dijkstra respects direction (E cannot reach D)", !g.dijkstra("E", "D").reachable);
    CHECK("Dijkstra: unreachable hub", !g.dijkstra("A", "Island").reachable);
    CHECK("Dijkstra: A->E = 1+2+5+3 = 11", near(g.dijkstra("A", "E").cost, 11));
    CHECK("Unknown hub throws NotFound", throwsType<NotFoundException>([&] { g.dijkstra("A", "Mars"); }));
    CHECK("Negative route cost rejected", throwsType<GraphException>([&] { g.addRoute("A", "B", -1); }));

    auto fw = g.floydWarshall();
    int a = g.hubId("A"), b = g.hubId("B"), e = g.hubId("E");
    CHECK("Floyd-Warshall A->B = 3", near(fw[a][b], 3));
    CHECK("Floyd-Warshall agrees with Dijkstra (A->E)", near(fw[a][e], g.dijkstra(a, e).cost));
    CHECK("Floyd-Warshall unreachable = INF", fw[e][a] == HubGraph::INF);

    MSTResult m = g.kruskalMST();
    CHECK("Kruskal MST total = 1+2+3+5 = 11", near(m.totalCost, 11));
    CHECK("Kruskal MST has V-2 edges (Island is isolated)", m.edges.size() == 4);

    DropOffCalculator calc(std::make_unique<OneWayDrop>());
    CHECK("One-way strategy: 200 + 12*100 = 1400", near(calc.calculate(100), 1400));
    calc.setStrategy(std::make_unique<SameHubDrop>());
    CHECK("Same-hub strategy is free", near(calc.calculate(100), 0));
    calc.setStrategy(std::make_unique<PeakSeasonDrop>());
    CHECK("Peak strategy = one-way x 1.25 = 1750", near(calc.calculate(100), 1750));
    CHECK("Strategy swapped at run time (name changes)", calc.strategyName() == "Peak-season one-way");
}

// ------------------------------ Module 3 ------------------------------------
static void testModule3() {
    std::cout << "\n--- Module 3: Search & Trees (Brijesh) ---\n";
    AVLTree<RateEntry> t;
    for (int i = 1; i <= 1000; ++i) t.insert(RateEntry{static_cast<double>(i), "V" + std::to_string(i)});
    CHECK("AVL holds 1000 sorted inserts", t.size() == 1000);
    CHECK("AVL stays balanced (height <= 14, a plain BST would be 1000)", t.height() <= 14);
    CHECK("AVL contains existing key", t.contains(RateEntry{500, "V500"}));
    CHECK("AVL does not contain missing key", !t.contains(RateEntry{500.5, "V?"}));
    CHECK("AVL ignores duplicates", !t.insert(RateEntry{10, "V10"}) && t.size() == 1000);
    CHECK("AVL min / max", near(t.findMin().rate, 1) && near(t.findMax().rate, 1000));
    auto in = t.inorder();
    bool sorted = true;
    for (std::size_t i = 1; i < in.size(); ++i) sorted = sorted && in[i - 1] < in[i];
    CHECK("In-order traversal is sorted", sorted);
    auto r = rangeByRate(t, 100, 105);
    CHECK("Range query 100..105 returns 6 items", r.size() == 6 && near(r.front().rate, 100) && near(r.back().rate, 105));
    CHECK("Range query with no match is empty", rangeByRate(t, 2000, 3000).empty());
    CHECK("Bad range throws", throwsType<InvalidInputException>([&] { rangeByRate(t, 10, 5); }));

    AVLTree<RateEntry> small;
    small.insert({3, "c"}); small.insert({1, "a"}); small.insert({2, "b"});    // needs Left-Right rotation
    CHECK("AVL rotation: 3 keys give height 2", small.height() == 2);
    AVLTree<RateEntry> empty;
    CHECK("Empty tree findMin throws", throwsType<NotFoundException>([&] { empty.findMin(); }));
    AVLTree<int> ints;                                  // template works for other types too
    ints.insert(5); ints.insert(2); ints.insert(8);
    CHECK("AVLTree<int> works (templates)", ints.contains(8) && !ints.contains(9));

    HuffmanCoder h;
    std::string text = "aaaaabbbcc d";
    h.build(text);
    std::string bits = h.encode(text);
    CHECK("Huffman round trip (decode(encode(x)) == x)", h.decode(bits) == text);
    CHECK("Huffman compresses repetitive text", bits.size() < text.size() * 8);
    CHECK("Frequent char gets a code no longer than a rare char", h.codes().at('a').size() <= h.codes().at('d').size());
    HuffmanCoder one;
    one.build("zzzz");
    CHECK("Huffman single-character text works", one.decode(one.encode("zzzz")) == "zzzz");
    CHECK("Huffman empty text rejected", throwsType<InvalidInputException>([&] { one.build(""); }));
}

// ------------------------------ Module 4 ------------------------------------
static void testModule4() {
    std::cout << "\n--- Module 4: State & Durability (Tianshu) ---\n";
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

    UndoStack s;
    s.push({1, "a", "V", 1, REGULAR, 0});
    s.push({2, "b", "V", 1, REGULAR, 0});
    CHECK("Stack is LIFO", s.top().id == 2 && s.pop().id == 2 && s.pop().id == 1 && s.empty());
    CHECK("Pop from empty stack throws", throwsType<NotFoundException>([&] { s.pop(); }));

    BookingManager m;
    int r1 = m.submit("Reg1", "V1", 2, REGULAR, 100);
    int r2 = m.submit("Reg2", "V2", 2, REGULAR, 100);
    int v1 = m.submit("VIP1", "V3", 2, VIP, 100);
    CHECK("Manager serves VIP before earlier regular", m.processNext().id == v1);
    CHECK("Manager then serves regulars in arrival order", m.processNext().id == r1);
    CHECK("Pending count updates", m.pendingCount() == 1 && m.confirmed().size() == 2);
    CHECK("Undo rolls back the latest confirmed booking", m.undoLast().id == r1 && m.confirmed().size() == 1);
    CHECK("Undo twice walks back (LIFO)", m.undoLast().id == v1 && m.confirmed().empty() && !m.canUndo());
    CHECK("Undo with nothing confirmed throws", throwsType<NotFoundException>([&] { m.undoLast(); }));
    (void)r2;
    CHECK("Empty customer rejected", throwsType<InvalidInputException>([&] { m.submit("", "V", 1, VIP, 1); }));
    CHECK("Zero days rejected", throwsType<InvalidInputException>([&] { m.submit("x", "V", 0, VIP, 1); }));

    const std::string path = "test_ledger.bin";
    Ledger led(path);
    led.clear();
    CHECK("Empty ledger loads nothing", led.loadAll().empty());
    led.append({1, "Rakesh Sanwal", "VIN1001", 3, VIP, 4500.5});
    led.append({2, "Asha", "VIN2001", 2, REGULAR, 5120});
    auto back = led.loadAll();
    CHECK("Ledger round trip keeps 2 records", back.size() == 2);
    CHECK("Ledger keeps every field", back[0].id == 1 && back[0].customer == "Rakesh Sanwal" && back[0].vin == "VIN1001" &&
                                          back[0].days == 3 && back[0].tier == VIP && near(back[0].amount, 4500.5));
    {   // simulate a crash that left half a record at the end of the file
        std::ofstream f(path, std::ios::binary | std::ios::app);
        f.write("garbage", 7);
    }
    CHECK("Half-written last record is ignored after a crash", led.loadAll().size() == 2);
    led.saveAll({back[1]});
    CHECK("saveAll rewrites the file", led.loadAll().size() == 1 && led.loadAll()[0].id == 2);
    {   // long names must be cut, not overflow the fixed record
        led.clear();
        led.append({9, std::string(100, 'N'), std::string(100, 'V'), 1, VIP, 1});
        CHECK("Over-long strings are truncated safely", led.loadAll().size() == 1 && led.loadAll()[0].customer.size() == 31);
    }
    CHECK("Unwritable path throws FileIOException",
          throwsType<FileIOException>([] { Ledger("/nonexistent_dir/x.bin").append({1, "a", "b", 1, VIP, 1}); }));
    std::remove(path.c_str());
}

int main() {
    testModule1();
    testModule2();
    testModule3();
    testModule4();
    std::cout << "\n=============================\n"
              << "Passed: " << passed << "   Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
