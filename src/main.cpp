// main.cpp -- Integration layer: connects all four modules in one console app.
// VeloRent - Smart Vehicle Rental Management System  (Team: The Hamsters)
//
//   Module 1 (Paras)   : Fleet, Vehicle hierarchy, hash table, sort, recursion
//   Module 2 (Rakesh)  : HubGraph, Dijkstra, Floyd-Warshall, MST, Strategy
//   Module 3 (Brijesh) : AVL price index, range search, Huffman coding
//   Module 4 (Tianshu) : heap + queue + undo stack, RAII, ledger.bin, exceptions
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

#include "Booking.h"
#include "HubGraph.h"
#include "RateTree.h"
#include "Vehicle.h"
using namespace std;

// ---- small input helpers (return false on end of input) --------------------
static bool ask(const string& prompt, string& out) {
    cout << prompt;
    return static_cast<bool>(getline(cin, out));
}

static int toInt(const string& s) {
    try {
        size_t used = 0;
        int v = stoi(s, &used);
        if (used != s.size()) {
            throw InvalidInputException("Please enter a whole number");
        }
        return v;
    } catch (const logic_error&) {
        throw InvalidInputException("Please enter a whole number");
    }
}

// ---- console look: banner, section titles, vehicle table -------------------
// (plain ASCII only, so it also looks right in the Windows console)
static const char* RULE = "+============================================================+\n";

static void banner() {
    cout << "\n" << RULE
              << "|                  V E L O R E N   R E N T A L               |\n"
              << "|           Smart Vehicle Rental Management System           |\n"
              << "|     Team The Hamsters  |  TCS302 DSA  +  TCS307 OOP        |\n"
              << RULE;
}

static void section(const string& title) {
    cout << "\n--- " << title << " " << string(title.size() < 57 ? 57 - title.size() : 0, '-') << "\n";
}

static const char* TABLE_LINE =
    "+------------+---------+------------+-------------------+----------+-----------+\n";

static void vehicleHeader() {
    cout << TABLE_LINE
              << "| " << left << setw(10) << "Type" << " | " << setw(7) << "VIN"
              << " | " << setw(10) << "Plate" << " | " << setw(17) << "Model"
              << " | " << setw(8) << "Rate/day" << " | " << setw(9) << "Status" << " |\n"
              << TABLE_LINE;
}

static void vehicleRow(const Vehicle& v) {
    cout << "| " << left << setw(10) << v.type() << " | " << setw(7) << v.getVIN()
              << " | " << setw(10) << v.getLicensePlate() << " | " << setw(17) << v.getBrand()
              << " | " << right << setw(8)
              << ("Rs." + to_string(static_cast<long>(v.getBaseRate())))
              << " | " << left << setw(9) << v.getStatus() << " |\n";
}

// ---- sample data -----------------------------------------------------------
// 30 vehicles: 12 cars, 7 electric, 7 bikes, 4 luxury buses.
static void loadFleet(Fleet& fleet) {
    // Cars
    fleet.add(make_unique<Car>("VIN1001", "UK07AB1234", "Maruti Swift", 1500));
    fleet.add(make_unique<Car>("VIN1002", "UK07CD5678", "Hyundai Creta", 2200));
    fleet.add(make_unique<Car>("VIN1003", "DL01XY0007", "Toyota Innova", 3200, 7));
    fleet.add(make_unique<Car>("VIN1004", "UK07EF9012", "Honda City", 2400));
    fleet.add(make_unique<Car>("VIN1005", "UK08GH3456", "Kia Seltos", 2600));
    fleet.add(make_unique<Car>("VIN1006", "DL03MN2211", "Mahindra Thar", 3000, 4));
    fleet.add(make_unique<Car>("VIN1007", "HR26PQ4455", "Maruti Ertiga", 2300, 7));
    fleet.add(make_unique<Car>("VIN1008", "UP14RS7788", "Tata Harrier", 3400));
    fleet.add(make_unique<Car>("VIN1009", "UK07TU6633", "Maruti Alto", 1200, 4));
    fleet.add(make_unique<Car>("VIN1010", "DL04VW1020", "Hyundai Verna", 2000));
    fleet.add(make_unique<Car>("VIN1011", "RJ14XY5090", "Toyota Fortuner", 4500, 7));
    fleet.add(make_unique<Car>("VIN1012", "UK04ZA8844", "Maruti Dzire", 1500));
    // Electric vehicles (name, rate, battery kWh, charge %)
    fleet.add(make_unique<ElectricVehicle>("VIN2001", "UK08EV0001", "Tata Nexon EV", 2500, 40.5, 80));
    fleet.add(make_unique<ElectricVehicle>("VIN2002", "DL02EV0042", "MG ZS EV", 2800, 50.3, 25));
    fleet.add(make_unique<ElectricVehicle>("VIN2003", "UK07EV0077", "Tata Tiago EV", 1900, 19.2, 65));
    fleet.add(make_unique<ElectricVehicle>("VIN2004", "DL05EV0310", "Mahindra XUV400", 2700, 39.4, 90));
    fleet.add(make_unique<ElectricVehicle>("VIN2005", "HR26EV0512", "BYD Atto 3", 3600, 60.5, 55));
    fleet.add(make_unique<ElectricVehicle>("VIN2006", "UK08EV0099", "Tata Punch EV", 2100, 35.0, 20));
    fleet.add(make_unique<ElectricVehicle>("VIN2007", "DL06EV0777", "Hyundai Kona EV", 3300, 39.2, 72));
    // Luxury buses (name, rate, seats, driver per day)
    fleet.add(make_unique<LuxuryBus>("VIN3001", "UK04BU9999", "Volvo 9400", 9000, 40, 1500));
    fleet.add(make_unique<LuxuryBus>("VIN3002", "DL01BU1212", "Mercedes Coach", 12000, 45, 1800));
    fleet.add(make_unique<LuxuryBus>("VIN3003", "UK07BU3434", "Force Traveller", 6000, 17, 1200));
    fleet.add(make_unique<LuxuryBus>("VIN3004", "UP14BU5656", "Tata Starbus", 7500, 32, 1500));
    // Bikes
    fleet.add(make_unique<Bike>("VIN4001", "UK07BK1111", "Royal Enfield", 800));
    fleet.add(make_unique<Bike>("VIN4002", "UK07BK2222", "Hero Splendor", 500));
    fleet.add(make_unique<Bike>("VIN4003", "UK07BK3333", "Bajaj Pulsar", 600));
    fleet.add(make_unique<Bike>("VIN4004", "UK07BK4444", "Honda Activa", 450));
    fleet.add(make_unique<Bike>("VIN4005", "DL07BK5555", "KTM Duke 390", 1200));
    fleet.add(make_unique<Bike>("VIN4006", "UK08BK6666", "TVS Apache", 650));
    fleet.add(make_unique<Bike>("VIN4007", "HR26BK7777", "RE Himalayan", 1100));
}

// 14 hubs. The first 7 hubs and 9 routes are the original Phase 3 data;
// the rest were added to make the network bigger (distances are approximate).
static void loadRoutes(HubGraph& g) {
    g.addRoute("Dehradun", "Rishikesh", 42, true);
    g.addRoute("Rishikesh", "Haridwar", 24, true);
    g.addRoute("Dehradun", "Haridwar", 54, true);
    g.addRoute("Dehradun", "Mussoorie", 35, true);
    g.addRoute("Haridwar", "Roorkee", 32, true);
    g.addRoute("Roorkee", "Delhi", 175, true);
    g.addRoute("Haridwar", "Delhi", 215, true);
    g.addRoute("Dehradun", "Chandigarh", 170, true);
    g.addRoute("Chandigarh", "Delhi", 250, false);      // one-way, shows a DIRECTED graph
    g.addRoute("Delhi", "Chandigarh", 245, false);
    // --- added hubs ---
    g.addRoute("Roorkee", "Saharanpur", 30, true);
    g.addRoute("Saharanpur", "Meerut", 105, true);
    g.addRoute("Meerut", "Delhi", 70, true);
    g.addRoute("Saharanpur", "Ambala", 75, true);
    g.addRoute("Ambala", "Chandigarh", 45, true);
    g.addRoute("Ambala", "Delhi", 210, true);
    g.addRoute("Delhi", "Agra", 230, true);
    g.addRoute("Delhi", "Jaipur", 280, true);
    g.addRoute("Agra", "Jaipur", 240, true);
    g.addRoute("Haridwar", "Haldwani", 180, true);
    g.addRoute("Haldwani", "Nainital", 40, true);
    g.addRoute("Delhi", "Haldwani", 270, true);
}

static void printMenu() {
    cout << "\n" << RULE
              << "|                        MAIN MENU                           |\n"
              << RULE
              << "  FLEET\n"
                 "    1. Show fleet (sorted by rate)\n"
                 "    2. Find vehicle by VIN / plate\n"
                 "    3. Find vehicles in a price range\n"
                 "   11. Fleet maintenance inspection (polymorphism)\n"
                 "  ROUTES AND PRICING\n"
                 "    4. Cheapest relocation route + drop-off charge\n"
                 "    5. Branch network (minimum spanning tree)\n"
                 "   12. Hub reachability (BFS and DFS)\n"
                 "   13. Prim vs Kruskal network check\n"
                 "   14. Bellman-Ford distances from a hub\n"
                 "  BOOKINGS\n"
                 "    6. New booking request\n"
                 "    7. Process next booking\n"
                 "    8. Undo last booking\n"
                 "    9. Show ledger.bin\n"
                 "  EXTRAS\n"
                 "   10. Huffman compression demo\n"
                 "    0. Exit\n"
                 "------------------------------------------------------------\n"
                 "Choice: ";
}

int main() {
    Fleet fleet;
    HubGraph hubs;
    AVLTree<RateEntry> rateIndex;
    BookingManager bookings;
    Ledger ledger("ledger.bin");

    banner();
    try {
        loadFleet(fleet);
        loadRoutes(hubs);
        for (Vehicle* v : fleet.all()) {
            rateIndex.insert(RateEntry{v->getBaseRate(), v->getVIN()});
        }

        // Crash recovery: reload previously saved bookings and mark those cars as booked.
        vector<Booking> saved = ledger.loadAll();
        bookings.restoreConfirmed(saved);
        for (const Booking& b : saved)
            if (Vehicle* v = fleet.findByVin(b.vin)) {
                v->setStatus("Booked");
            }
        cout << "VeloRent started. Fleet: " << fleet.size() << " vehicles, hubs: "
                  << hubs.hubCount() << ", bookings recovered from ledger.bin: " << saved.size()
                  << "\n";
    } catch (const VeloException& e) {
        cerr << "Start-up error: " << e.what() << "\n";
        return 1;
    }

    string line;
    for (;;) {
        printMenu();
        if (!getline(cin, line)) {
            break;
            // end of input
        }
        try {
            int choice = toInt(line);
            if (choice == 0) {
                break;
            }

            if (choice == 1) {
                section("Fleet sorted by daily rate (merge sort)");
                vehicleHeader();
                int free = 0;
                vector<Vehicle*> sorted = fleet.sortedByRate();
                for (Vehicle* v : sorted) {
                    vehicleRow(*v);
                    if (v->isAvailable()) {
                        ++free;
                    }
                }
                cout << TABLE_LINE << sorted.size() << " vehicles, " << free << " available, "
                          << (sorted.size() - free) << " booked.\n";

            } else if (choice == 2) {
                section("Find a vehicle (hash table lookup)");
                string key;
                ask("Enter VIN or licence plate: ", key);
                Vehicle* v = fleet.findByVin(key);
                if (!v) {
                    v = fleet.findByPlate(key);
                }
                if (!v) {
                    throw NotFoundException("No vehicle found for '" + key + "'");
                }
                vehicleHeader();
                vehicleRow(*v);
                cout << TABLE_LINE;

            } else if (choice == 3) {
                section("Vehicles in a price range (AVL tree)");
                string a, b;
                ask("Minimum rate per day: ", a);
                ask("Maximum rate per day: ", b);
                auto found = rangeByRate(rateIndex, toInt(a), toInt(b));
                if (found.empty()) {
                    cout << "No vehicles in that range.\n";
                } else {
                    vehicleHeader();
                    for (const RateEntry& e : found) {
                        vehicleRow(*fleet.findByVin(e.vin));
                    }
                    cout << TABLE_LINE << found.size() << " vehicle(s) found.\n";
                }
                cout << "(AVL tree height = " << rateIndex.height() << " for " << rateIndex.size() << " vehicles)\n";

            } else if (choice == 4) {
                section("Cheapest relocation route (Dijkstra)");
                string from, to, mode;
                cout << "Hubs and routes:\n";
                hubs.print();
                ask("From hub: ", from);
                ask("To hub: ", to);
                ask("Pricing  1=Same hub  2=One-way  3=Peak season: ", mode);
                PathResult r = hubs.dijkstra(from, to);
                if (!r.reachable) {
                    throw NotFoundException("No route from " + from + " to " + to);
                }
                cout << "\n  Route    : ";
                for (size_t i = 0; i < r.path.size(); ++i)
                    cout << (i ? " -> " : "") << hubs.hubName(r.path[i]);
                cout << "\n  Distance : " << r.cost << " km\n";
                unique_ptr<DropOffStrategy> s;
                int m = toInt(mode);
                if (m == 1) {
                    s = make_unique<SameHubDrop>();
                }
                else if (m == 3) {
                    s = make_unique<PeakSeasonDrop>();
                }
                else s = make_unique<OneWayDrop>();
                DropOffCalculator calc(move(s));
                cout << "  Pricing  : " << calc.strategyName() << "\n"
                          << "  Charge   : Rs." << calc.calculate(r.cost) << "\n";

            } else if (choice == 5) {
                section("Branch network (Kruskal + Union-Find)");
                MSTResult mst = hubs.kruskalMST();
                cout << "Cheapest set of routes linking all hubs:\n";
                for (const MSTEdge& e : mst.edges)
                    cout << "  " << left << setw(11) << hubs.hubName(e.u) << " - "
                              << setw(11) << hubs.hubName(e.v) << right << setw(5) << e.cost << " km\n";
                cout << left << "Total network length: " << mst.totalCost << " km\n";

            } else if (choice == 6) {
                section("New booking request");
                string cust, vin, days, tier;
                ask("Customer name: ", cust);
                ask("Vehicle VIN: ", vin);
                ask("Days: ", days);
                ask("Tier  1=VIP  2=Regular: ", tier);
                Vehicle& v = fleet.getByVin(vin);
                if (!v.isAvailable()) {
                    throw InvalidInputException(vin + " is not available");
                }
                int d = toInt(days);
                int id = bookings.submit(cust, vin, d, toInt(tier) == 1 ? VIP : REGULAR, v.calculateTariff(d));
                cout << "Booking request #" << id << " accepted (pending). Quoted amount: Rs."
                          << v.calculateTariff(d) << "\n";

            } else if (choice == 7) {
                section("Process next booking (VIP first)");
                Booking b = bookings.processNext();
                fleet.getByVin(b.vin).setStatus("Booked");
                ledger.append(b);                               // saved to ledger.bin immediately
                cout << "Confirmed: " << b << "\n";

            } else if (choice == 8) {
                section("Undo last booking");
                Booking b = bookings.undoLast();
                fleet.getByVin(b.vin).setStatus("Available");
                ledger.saveAll(bookings.confirmed());
                cout << "Rolled back: " << b << "\n";

            } else if (choice == 9) {
                section("Saved bookings (ledger.bin)");
                auto all = ledger.loadAll();
                if (all.empty()) {
                    cout << "Ledger is empty.\n";
                }
                for (const Booking& b : all) {
                    cout << "  " << b << "\n";
                }
                if (!all.empty()) {
                    cout << all.size() << " booking(s) saved.\n";
                }

            } else if (choice == 10) {
                section("Huffman compression of a receipt");
                string text;
                ostringstream receipt;
                receipt << "RECEIPT VeloRent\n";
                for (const Booking& b : bookings.confirmed())
                    receipt << b.customer << " rented " << b.vin << " for " << b.days << " days\n";
                if (bookings.confirmed().empty())
                    receipt << "Ravi rented VIN1001 for 3 days\nAsha rented VIN4002 for 2 days\n";
                text = receipt.str();
                HuffmanCoder huff;
                huff.build(text);
                string bits = huff.encode(text);
                cout << text << "Huffman codes:\n";
                huff.printCodes();
                cout << "Original: " << text.size() * 8 << " bits, compressed: " << bits.size()
                          << " bits (" << fixed << setprecision(1) << huff.compressionRatio(text) * 100
                          << "% of original)\n";
                cout << "Decoded text matches original: " << (huff.decode(bits) == text ? "YES" : "NO") << "\n";
                cout.unsetf(ios::fixed);

            } else if (choice == 11) {
                section("Fleet maintenance inspection (polymorphism)");
                for (Vehicle* v : fleet.all()) {
                    v->inspectMaintenance();
                    // one call, many behaviours
                }

            } else if (choice == 12) {
                section("Hub reachability (BFS and DFS)");
                string from;
                ask("Start hub: ", from);
                int s = hubs.hubId(from);
                cout << "BFS (fewest hops first): ";
                for (int id : hubs.bfs(s)) {
                    cout << hubs.hubName(id) << " ";
                }
                cout << "\nDFS (deepest first):     ";
                for (int id : hubs.dfs(s)) {
                    cout << hubs.hubName(id) << " ";
                }
                cout << "\n";

            } else if (choice == 13) {
                section("Prim vs Kruskal");
                MSTResult p = hubs.primMST(), k = hubs.kruskalMST();
                cout << "Prim's network:\n";
                for (const MSTEdge& e : p.edges)
                    cout << "  " << left << setw(11) << hubs.hubName(e.u) << " - "
                              << setw(11) << hubs.hubName(e.v) << right << setw(5) << e.cost << " km\n";
                cout << left << "Prim total: " << p.totalCost << " km, Kruskal total: " << k.totalCost << " km -> "
                          << (p.totalCost == k.totalCost ? "SAME" : "DIFFERENT") << "\n";

            } else if (choice == 14) {
                section("Bellman-Ford distances");
                string from;
                ask("Start hub: ", from);
                HubGraph::BFResult r = hubs.bellmanFord(hubs.hubId(from));
                for (int i = 0; i < hubs.hubCount(); ++i) {
                    cout << "  " << left << setw(11) << hubs.hubName(i) << ": ";
                    if (r.dist[i] == HubGraph::INF) {
                        cout << "unreachable\n";
                    }
                    else cout << right << setw(5) << r.dist[i] << " km\n";
                }
                cout << left;
                if (r.negativeCycle) {
                    cout << "Negative cycle detected!\n";
                }

            } else {
                cout << "Please choose 0-14.\n";
            }
        } catch (const VeloException& e) {
            cout << "Error: " << e.what() << "\n";            // clean message, program keeps running
        }
    }
    cout << "\nThank you for using VeloRent. Goodbye.\n";
    return 0;
}
