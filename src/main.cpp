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

// ---- small input helpers (return false on end of input) --------------------
static bool ask(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, out));
}

static int toInt(const std::string& s) {
    try {
        std::size_t used = 0;
        int v = std::stoi(s, &used);
        if (used != s.size()) throw InvalidInputException("Please enter a whole number");
        return v;
    } catch (const std::logic_error&) {
        throw InvalidInputException("Please enter a whole number");
    }
}

// ---- sample data -----------------------------------------------------------
static void loadFleet(Fleet& fleet) {
    fleet.add(std::make_unique<Car>("VIN1001", "UK07AB1234", "Maruti Swift", 1500));
    fleet.add(std::make_unique<Car>("VIN1002", "UK07CD5678", "Hyundai Creta", 2200));
    fleet.add(std::make_unique<Car>("VIN1003", "DL01XY0007", "Toyota Innova", 3200, 7));
    fleet.add(std::make_unique<ElectricVehicle>("VIN2001", "UK08EV0001", "Tata Nexon EV", 2500, 40.5, 80));
    fleet.add(std::make_unique<ElectricVehicle>("VIN2002", "DL02EV0042", "MG ZS EV", 2800, 50.3, 25));
    fleet.add(std::make_unique<LuxuryBus>("VIN3001", "UK04BU9999", "Volvo 9400", 9000, 40, 1500));
    fleet.add(std::make_unique<Bike>("VIN4001", "UK07BK1111", "Royal Enfield", 800));
    fleet.add(std::make_unique<Bike>("VIN4002", "UK07BK2222", "Hero Splendor", 500));
}

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
}

static void printMenu() {
    std::cout << "\n=========== VeloRent ===========\n"
                 " 1. Show fleet (sorted by rate)\n"
                 " 2. Find vehicle by VIN / plate\n"
                 " 3. Find vehicles in a price range\n"
                 " 4. Cheapest relocation route + drop-off charge\n"
                 " 5. Branch network (minimum spanning tree)\n"
                 " 6. New booking request\n"
                 " 7. Process next booking\n"
                 " 8. Undo last booking\n"
                 " 9. Show ledger.bin\n"
                 "10. Huffman compression demo\n"
                 "11. Fleet maintenance inspection (polymorphism)\n"
                 " 0. Exit\n"
                 "Choice: ";
}

int main() {
    Fleet fleet;
    HubGraph hubs;
    AVLTree<RateEntry> rateIndex;
    BookingManager bookings;
    Ledger ledger("ledger.bin");

    try {
        loadFleet(fleet);
        loadRoutes(hubs);
        for (Vehicle* v : fleet.all()) rateIndex.insert(RateEntry{v->getBaseRate(), v->getVIN()});

        // Crash recovery: reload previously saved bookings and mark those cars as booked.
        std::vector<Booking> saved = ledger.loadAll();
        bookings.restoreConfirmed(saved);
        for (const Booking& b : saved)
            if (Vehicle* v = fleet.findByVin(b.vin)) v->setStatus("Booked");
        std::cout << "VeloRent started. Fleet: " << fleet.size() << " vehicles, hubs: "
                  << hubs.hubCount() << ", bookings recovered from ledger.bin: " << saved.size()
                  << "\n";
    } catch (const VeloException& e) {
        std::cerr << "Start-up error: " << e.what() << "\n";
        return 1;
    }

    std::string line;
    for (;;) {
        printMenu();
        if (!std::getline(std::cin, line)) break;            // end of input
        try {
            int choice = toInt(line);
            if (choice == 0) break;

            if (choice == 1) {
                for (Vehicle* v : fleet.sortedByRate()) std::cout << *v << "\n";

            } else if (choice == 2) {
                std::string key;
                ask("Enter VIN or licence plate: ", key);
                Vehicle* v = fleet.findByVin(key);
                if (!v) v = fleet.findByPlate(key);
                if (!v) throw NotFoundException("No vehicle found for '" + key + "'");
                std::cout << *v << "\n";

            } else if (choice == 3) {
                std::string a, b;
                ask("Minimum rate per day: ", a);
                ask("Maximum rate per day: ", b);
                auto found = rangeByRate(rateIndex, toInt(a), toInt(b));
                if (found.empty()) std::cout << "No vehicles in that range.\n";
                for (const RateEntry& e : found) std::cout << "  " << e << "  " << *fleet.findByVin(e.vin) << "\n";
                std::cout << "(AVL tree height = " << rateIndex.height() << " for " << rateIndex.size() << " vehicles)\n";

            } else if (choice == 4) {
                std::string from, to, mode;
                std::cout << "Hubs:\n";
                hubs.print();
                ask("From hub: ", from);
                ask("To hub: ", to);
                ask("Pricing  1=Same hub  2=One-way  3=Peak season: ", mode);
                PathResult r = hubs.dijkstra(from, to);
                if (!r.reachable) throw NotFoundException("No route from " + from + " to " + to);
                std::cout << "Cheapest route: ";
                for (std::size_t i = 0; i < r.path.size(); ++i)
                    std::cout << (i ? " -> " : "") << hubs.hubName(r.path[i]);
                std::cout << "  (" << r.cost << " km)\n";
                std::unique_ptr<DropOffStrategy> s;
                int m = toInt(mode);
                if (m == 1) s = std::make_unique<SameHubDrop>();
                else if (m == 3) s = std::make_unique<PeakSeasonDrop>();
                else s = std::make_unique<OneWayDrop>();
                DropOffCalculator calc(std::move(s));
                std::cout << calc.strategyName() << " charge: Rs." << calc.calculate(r.cost) << "\n";

            } else if (choice == 5) {
                MSTResult mst = hubs.kruskalMST();
                std::cout << "Cheapest set of routes linking all hubs (Kruskal):\n";
                for (const MSTEdge& e : mst.edges)
                    std::cout << "  " << hubs.hubName(e.u) << " - " << hubs.hubName(e.v) << "  " << e.cost << " km\n";
                std::cout << "Total network length: " << mst.totalCost << " km\n";

            } else if (choice == 6) {
                std::string cust, vin, days, tier;
                ask("Customer name: ", cust);
                ask("Vehicle VIN: ", vin);
                ask("Days: ", days);
                ask("Tier  1=VIP  2=Regular: ", tier);
                Vehicle& v = fleet.getByVin(vin);
                if (!v.isAvailable()) throw InvalidInputException(vin + " is not available");
                int d = toInt(days);
                int id = bookings.submit(cust, vin, d, toInt(tier) == 1 ? VIP : REGULAR, v.calculateTariff(d));
                std::cout << "Booking request #" << id << " accepted (pending).\n";

            } else if (choice == 7) {
                Booking b = bookings.processNext();
                fleet.getByVin(b.vin).setStatus("Booked");
                ledger.append(b);                               // saved to ledger.bin immediately
                std::cout << "Confirmed: " << b << "\n";

            } else if (choice == 8) {
                Booking b = bookings.undoLast();
                fleet.getByVin(b.vin).setStatus("Available");
                ledger.saveAll(bookings.confirmed());
                std::cout << "Rolled back: " << b << "\n";

            } else if (choice == 9) {
                auto all = ledger.loadAll();
                if (all.empty()) std::cout << "Ledger is empty.\n";
                for (const Booking& b : all) std::cout << "  " << b << "\n";

            } else if (choice == 10) {
                std::string text;
                std::ostringstream receipt;
                receipt << "RECEIPT VeloRent\n";
                for (const Booking& b : bookings.confirmed())
                    receipt << b.customer << " rented " << b.vin << " for " << b.days << " days\n";
                if (bookings.confirmed().empty())
                    receipt << "Ravi rented VIN1001 for 3 days\nAsha rented VIN4002 for 2 days\n";
                text = receipt.str();
                HuffmanCoder huff;
                huff.build(text);
                std::string bits = huff.encode(text);
                std::cout << text << "Huffman codes:\n";
                huff.printCodes();
                std::cout << "Original: " << text.size() * 8 << " bits, compressed: " << bits.size()
                          << " bits (" << std::fixed << std::setprecision(1) << huff.compressionRatio(text) * 100
                          << "% of original)\n";
                std::cout << "Decoded text matches original: " << (huff.decode(bits) == text ? "YES" : "NO") << "\n";
                std::cout.unsetf(std::ios::fixed);

            } else if (choice == 11) {
                for (Vehicle* v : fleet.all()) v->inspectMaintenance();   // one call, many behaviours

            } else {
                std::cout << "Please choose 0-11.\n";
            }
        } catch (const VeloException& e) {
            std::cout << "Error: " << e.what() << "\n";            // clean message, program keeps running
        }
    }
    std::cout << "Goodbye.\n";
    return 0;
}
