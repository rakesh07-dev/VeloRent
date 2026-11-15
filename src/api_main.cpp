// api_main.cpp -- JSON bridge over stdin/stdout for VeloRent
// Connects the Express server to our C++14 fleet, graph, tree, and booking modules.

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "Booking.h"
#include "HubGraph.h"
#include "RateTree.h"
#include "Vehicle.h"
using namespace std;

// ---- string and number helpers --------------------------------------------
static string formatDouble(double val) {
    if (isinf(val) || isnan(val)) {
        return "-1";
    }
    ostringstream os;
    if (fabs(val - floor(val)) < 1e-6) {
        os << fixed << setprecision(0) << val;
    } else {
        os << fixed << setprecision(2) << val;
    }
    return os.str();
}

static int parseInt(const string& s, const string& field) {
    char* end = nullptr;
    errno = 0;
    long v = strtol(s.c_str(), &end, 10);
    if (s.empty() || end == s.c_str() || *end != '\0' || errno == ERANGE || v > INT_MAX || v < INT_MIN) {
        throw InvalidInputException("Invalid integer for " + field);
    }
    return static_cast<int>(v);
}

static double parseDouble(const string& s, const string& field) {
    char* end = nullptr;
    errno = 0;
    double v = strtod(s.c_str(), &end);
    if (s.empty() || end == s.c_str() || *end != '\0' || errno == ERANGE || isnan(v) || isinf(v)) {
        throw InvalidInputException("Invalid number for " + field);
    }
    return v;
}

static string escapeJson(const string& s) {
    ostringstream out;
    for (size_t i = 0; i < s.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        switch (c) {
            case '"':  out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b";  break;
            case '\f': out << "\\f";  break;
            case '\n': out << "\\n";  break;
            case '\r': out << "\\r";  break;
            case '\t': out << "\\t";  break;
            default:
                if (c < 0x20) {
                    out << "\\u00" << hex << setw(2) << setfill('0')
                        << static_cast<int>(c) << dec;
                } else {
                    out << static_cast<char>(c);
                }
        }
    }
    return out.str();
}

static string quoteJson(const string& s) {
    return "\"" + escapeJson(s) + "\"";
}

// Splits a command line into tokens, keeping quoted strings intact.
static vector<string> splitTokens(const string& line) {
    vector<string> tokens;
    size_t i = 0;
    size_t n = line.size();
    while (i < n) {
        while (i < n && (line[i] == ' ' || line[i] == '\t' || line[i] == '\r' || line[i] == '\n')) {
            ++i;
        }
        if (i >= n) {
            break;
        }
        if (line[i] == '"') {
            ++i;
            string word;
            bool closed = false;
            while (i < n) {
                if (line[i] == '\\' && i + 1 < n) {
                    ++i;
                    if (line[i] == 'n') {
                        word += '\n';
                    } else if (line[i] == 't') {
                        word += '\t';
                    } else {
                        word += line[i];
                    }
                } else if (line[i] == '"') {
                    closed = true;
                    ++i;
                    break;
                } else {
                    word += line[i];
                }
                ++i;
            }
            if (!closed) {
                throw InvalidInputException("Unterminated quoted argument");
            }
            tokens.push_back(word);
        } else {
            string word;
            while (i < n && line[i] != ' ' && line[i] != '\t' && line[i] != '\r' && line[i] != '\n') {
                word += line[i];
                ++i;
            }
            tokens.push_back(word);
        }
    }
    return tokens;
}

// ---- fleet and hub data (same 30 vehicles & 14 hubs as main.cpp) ----------
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
    // Electric vehicles
    fleet.add(make_unique<ElectricVehicle>("VIN2001", "UK08EV0001", "Tata Nexon EV", 2500, 40.5, 80));
    fleet.add(make_unique<ElectricVehicle>("VIN2002", "DL02EV0042", "MG ZS EV", 2800, 50.3, 25));
    fleet.add(make_unique<ElectricVehicle>("VIN2003", "UK07EV0077", "Tata Tiago EV", 1900, 19.2, 65));
    fleet.add(make_unique<ElectricVehicle>("VIN2004", "DL05EV0310", "Mahindra XUV400", 2700, 39.4, 90));
    fleet.add(make_unique<ElectricVehicle>("VIN2005", "HR26EV0512", "BYD Atto 3", 3600, 60.5, 55));
    fleet.add(make_unique<ElectricVehicle>("VIN2006", "UK08EV0099", "Tata Punch EV", 2100, 35.0, 20));
    fleet.add(make_unique<ElectricVehicle>("VIN2007", "DL06EV0777", "Hyundai Kona EV", 3300, 39.2, 72));
    // Luxury buses
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

struct RouteItem {
    string from;
    string to;
    double cost;
    bool twoWay;
};

static vector<RouteItem> buildRouteList() {
    vector<RouteItem> list;
    list.push_back({"Dehradun", "Rishikesh", 42, true});
    list.push_back({"Rishikesh", "Haridwar", 24, true});
    list.push_back({"Dehradun", "Haridwar", 54, true});
    list.push_back({"Dehradun", "Mussoorie", 35, true});
    list.push_back({"Haridwar", "Roorkee", 32, true});
    list.push_back({"Roorkee", "Delhi", 175, true});
    list.push_back({"Haridwar", "Delhi", 215, true});
    list.push_back({"Dehradun", "Chandigarh", 170, true});
    list.push_back({"Chandigarh", "Delhi", 250, false});
    list.push_back({"Delhi", "Chandigarh", 245, false});
    list.push_back({"Roorkee", "Saharanpur", 30, true});
    list.push_back({"Saharanpur", "Meerut", 105, true});
    list.push_back({"Meerut", "Delhi", 70, true});
    list.push_back({"Saharanpur", "Ambala", 75, true});
    list.push_back({"Ambala", "Chandigarh", 45, true});
    list.push_back({"Ambala", "Delhi", 210, true});
    list.push_back({"Delhi", "Agra", 230, true});
    list.push_back({"Delhi", "Jaipur", 280, true});
    list.push_back({"Agra", "Jaipur", 240, true});
    list.push_back({"Haridwar", "Haldwani", 180, true});
    list.push_back({"Haldwani", "Nainital", 40, true});
    list.push_back({"Delhi", "Haldwani", 270, true});
    return list;
}

static void loadRoutes(HubGraph& g, const vector<RouteItem>& list) {
    for (size_t i = 0; i < list.size(); ++i) {
        g.addRoute(list[i].from, list[i].to, list[i].cost, list[i].twoWay);
    }
}

static string vehicleJson(const Vehicle& v) {
    ostringstream os;
    os << "{"
       << "\"vin\":" << quoteJson(v.getVIN()) << ","
       << "\"plate\":" << quoteJson(v.getLicensePlate()) << ","
       << "\"brand\":" << quoteJson(v.getBrand()) << ","
       << "\"type\":" << quoteJson(v.type()) << ","
       << "\"baseRate\":" << formatDouble(v.getBaseRate()) << ","
       << "\"status\":" << quoteJson(v.getStatus()) << ","
       << "\"available\":" << (v.isAvailable() ? "true" : "false") << ","
       << "\"tariff1Day\":" << formatDouble(v.calculateTariff(1)) << ","
       << "\"tariff7Days\":" << formatDouble(v.calculateTariff(7))
       << "}";
    return os.str();
}

static string bookingJson(const Booking& b, const string& label) {
    ostringstream os;
    os << "{"
       << "\"id\":" << b.id << ","
       << "\"customer\":" << quoteJson(b.customer) << ","
       << "\"vin\":" << quoteJson(b.vin) << ","
       << "\"days\":" << b.days << ","
       << "\"tier\":" << quoteJson(b.tier == VIP ? "VIP" : "Regular") << ","
       << "\"amount\":" << formatDouble(b.amount) << ","
       << "\"status\":" << quoteJson(label)
       << "}";
    return os.str();
}

int main() {
    Fleet fleet;
    HubGraph hubs;
    AVLTree<RateEntry> rateIndex;
    BookingManager bookings;
    Ledger ledger("ledger.bin");
    vector<RouteItem> routeList = buildRouteList();
    vector<Booking> pendingList;

    try {
        loadFleet(fleet);
        loadRoutes(hubs, routeList);
        vector<Vehicle*> allVehicles = fleet.all();
        for (size_t i = 0; i < allVehicles.size(); ++i) {
            rateIndex.insert(RateEntry{allVehicles[i]->getBaseRate(), allVehicles[i]->getVIN()});
        }
        vector<Booking> saved = ledger.loadAll();
        bookings.restoreConfirmed(saved);
        for (size_t i = 0; i < saved.size(); ++i) {
            Vehicle* v = fleet.findByVin(saved[i].vin);
            if (v) {
                v->setStatus("Booked");
            }
        }
    } catch (const VeloException& e) {
        cout << "{\"ok\":false,\"error\":" << quoteJson(e.what()) << "}" << endl;
        return 1;
    }

    string line;
    while (getline(cin, line)) {
        try {
            vector<string> args = splitTokens(line);
            if (args.empty()) {
                continue;
            }

            const string& cmd = args[0];

            if (cmd == "PING") {
                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"status\":\"online\",\"vehicles\":" << fleet.size()
                   << ",\"hubs\":" << hubs.hubCount()
                   << ",\"confirmedBookings\":" << bookings.confirmed().size()
                   << ",\"pendingBookings\":" << bookings.pendingCount()
                   << ",\"canUndo\":" << (bookings.canUndo() ? "true" : "false") << "}}";
                cout << os.str() << endl;

            } else if (cmd == "VEHICLES") {
                vector<Vehicle*> sorted = fleet.sortedByRate();
                ostringstream os;
                os << "{\"ok\":true,\"data\":[";
                for (size_t i = 0; i < sorted.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << vehicleJson(*sorted[i]);
                }
                os << "]}";
                cout << os.str() << endl;

            } else if (cmd == "VEHICLE") {
                if (args.size() < 2) {
                    throw InvalidInputException("Usage: VEHICLE <vin_or_plate>");
                }
                Vehicle* v = fleet.findByVin(args[1]);
                if (!v) {
                    v = fleet.findByPlate(args[1]);
                }
                if (!v) {
                    throw NotFoundException("No vehicle found for '" + args[1] + "'");
                }
                cout << "{\"ok\":true,\"data\":" << vehicleJson(*v) << "}" << endl;

            } else if (cmd == "QUOTE") {
                if (args.size() < 3) {
                    throw InvalidInputException("Usage: QUOTE <vin> <days>");
                }
                Vehicle& v = fleet.getByVin(args[1]);
                int days = parseInt(args[2], "days");
                double amt = v.calculateTariff(days);
                cout << "{\"ok\":true,\"data\":{\"vin\":" << quoteJson(v.getVIN())
                     << ",\"days\":" << days << ",\"amount\":" << formatDouble(amt) << "}}" << endl;

            } else if (cmd == "RANGE") {
                if (args.size() < 3) {
                    throw InvalidInputException("Usage: RANGE <min> <max>");
                }
                double minRate = parseDouble(args[1], "min");
                double maxRate = parseDouble(args[2], "max");
                vector<RateEntry> found = rangeByRate(rateIndex, minRate, maxRate);
                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"avlHeight\":" << rateIndex.height()
                   << ",\"avlSize\":" << rateIndex.size() << ",\"vehicles\":[";
                for (size_t i = 0; i < found.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    Vehicle* v = fleet.findByVin(found[i].vin);
                    os << vehicleJson(*v);
                }
                os << "]}}";
                cout << os.str() << endl;

            } else if (cmd == "HUBS") {
                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"hubs\":[";
                for (int i = 0; i < hubs.hubCount(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << "{\"id\":" << i << ",\"name\":" << quoteJson(hubs.hubName(i)) << "}";
                }
                os << "],\"routes\":[";
                for (size_t i = 0; i < routeList.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << "{\"from\":" << quoteJson(routeList[i].from)
                       << ",\"to\":" << quoteJson(routeList[i].to)
                       << ",\"cost\":" << formatDouble(routeList[i].cost)
                       << ",\"bidirectional\":" << (routeList[i].twoWay ? "true" : "false") << "}";
                }
                os << "]}}";
                cout << os.str() << endl;

            } else if (cmd == "ROUTE") {
                if (args.size() < 4) {
                    throw InvalidInputException("Usage: ROUTE <from> <to> <same|oneway|peak>");
                }
                const string& from = args[1];
                const string& to = args[2];
                const string& mode = args[3];
                PathResult r = hubs.dijkstra(from, to);
                if (!r.reachable) {
                    throw NotFoundException("No route from " + from + " to " + to);
                }

                unique_ptr<DropOffStrategy> strat;
                if (mode == "same") {
                    strat = make_unique<SameHubDrop>();
                } else if (mode == "oneway") {
                    strat = make_unique<OneWayDrop>();
                } else if (mode == "peak") {
                    strat = make_unique<PeakSeasonDrop>();
                } else {
                    throw InvalidInputException("Invalid pricing mode (use same, oneway, or peak)");
                }

                DropOffCalculator calc(move(strat));
                double charge = calc.calculate(r.cost);

                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"from\":" << quoteJson(from)
                   << ",\"to\":" << quoteJson(to)
                   << ",\"distanceKm\":" << formatDouble(r.cost)
                   << ",\"strategy\":" << quoteJson(calc.strategyName())
                   << ",\"charge\":" << formatDouble(charge)
                   << ",\"path\":[";
                for (size_t i = 0; i < r.path.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << quoteJson(hubs.hubName(r.path[i]));
                }
                os << "]}}";
                cout << os.str() << endl;

            } else if (cmd == "MST" || cmd == "MST_PRIM") {
                MSTResult m = (cmd == "MST_PRIM") ? hubs.primMST() : hubs.kruskalMST();
                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"algorithm\":"
                   << quoteJson(cmd == "MST_PRIM" ? "Prim" : "Kruskal + Union-Find")
                   << ",\"totalCost\":" << formatDouble(m.totalCost) << ",\"edges\":[";
                for (size_t i = 0; i < m.edges.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << "{\"from\":" << quoteJson(hubs.hubName(m.edges[i].u))
                       << ",\"to\":" << quoteJson(hubs.hubName(m.edges[i].v))
                       << ",\"cost\":" << formatDouble(m.edges[i].cost) << "}";
                }
                os << "]}}";
                cout << os.str() << endl;

            } else if (cmd == "REACH") {
                if (args.size() < 2) {
                    throw InvalidInputException("Usage: REACH <hub>");
                }
                int src = hubs.hubId(args[1]);
                vector<int> bfsOrder = hubs.bfs(src);
                vector<int> dfsOrder = hubs.dfs(src);
                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"hub\":" << quoteJson(args[1]) << ",\"bfs\":[";
                for (size_t i = 0; i < bfsOrder.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << quoteJson(hubs.hubName(bfsOrder[i]));
                }
                os << "],\"dfs\":[";
                for (size_t i = 0; i < dfsOrder.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << quoteJson(hubs.hubName(dfsOrder[i]));
                }
                os << "]}}";
                cout << os.str() << endl;

            } else if (cmd == "BF") {
                if (args.size() < 2) {
                    throw InvalidInputException("Usage: BF <hub>");
                }
                int src = hubs.hubId(args[1]);
                HubGraph::BFResult bf = hubs.bellmanFord(src);
                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"hub\":" << quoteJson(args[1])
                   << ",\"negativeCycle\":" << (bf.negativeCycle ? "true" : "false")
                   << ",\"distances\":[";
                for (int i = 0; i < hubs.hubCount(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    bool reach = (bf.dist[i] != HubGraph::INF);
                    os << "{\"hub\":" << quoteJson(hubs.hubName(i))
                       << ",\"reachable\":" << (reach ? "true" : "false")
                       << ",\"distanceKm\":" << (reach ? formatDouble(bf.dist[i]) : "null") << "}";
                }
                os << "]}}";
                cout << os.str() << endl;

            } else if (cmd == "BOOK") {
                if (args.size() < 5) {
                    throw InvalidInputException("Usage: BOOK \"<customer>\" <vin> <days> <vip|regular>");
                }
                const string& cust = args[1];
                const string& vin = args[2];
                int days = parseInt(args[3], "days");
                const string& tierStr = args[4];
                int tierVal = REGULAR;
                if (tierStr == "vip" || tierStr == "VIP") {
                    tierVal = VIP;
                } else if (tierStr == "regular" || tierStr == "REGULAR") {
                    tierVal = REGULAR;
                } else {
                    throw InvalidInputException("Tier must be vip or regular");
                }

                Vehicle& v = fleet.getByVin(vin);
                if (!v.isAvailable()) {
                    throw InvalidInputException(vin + " is not available");
                }
                double amt = v.calculateTariff(days);
                int id = bookings.submit(cust, vin, days, tierVal, amt);
                Booking created{id, cust, vin, days, tierVal, amt};
                pendingList.push_back(created);

                cout << "{\"ok\":true,\"data\":" << bookingJson(created, "Pending") << "}" << endl;

            } else if (cmd == "PROCESS") {
                Booking b = bookings.processNext();
                for (size_t i = 0; i < pendingList.size(); ++i) {
                    if (pendingList[i].id == b.id) {
                        pendingList.erase(pendingList.begin() + static_cast<long>(i));
                        break;
                    }
                }
                Vehicle& v = fleet.getByVin(b.vin);
                if (!v.isAvailable()) {
                    bookings.undoLast();
                    throw InvalidInputException("Vehicle " + b.vin + " was already booked by an earlier processed request");
                }
                v.setStatus("Booked");
                ledger.append(b);
                cout << "{\"ok\":true,\"data\":" << bookingJson(b, "Confirmed") << "}" << endl;

            } else if (cmd == "UNDO") {
                Booking b = bookings.undoLast();
                fleet.getByVin(b.vin).setStatus("Available");
                ledger.saveAll(bookings.confirmed());
                cout << "{\"ok\":true,\"data\":" << bookingJson(b, "RolledBack") << "}" << endl;

            } else if (cmd == "LEDGER") {
                vector<Booking> saved = ledger.loadAll();
                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"confirmed\":[";
                for (size_t i = 0; i < saved.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << bookingJson(saved[i], "Confirmed");
                }
                os << "],\"pending\":[";
                for (size_t i = 0; i < pendingList.size(); ++i) {
                    if (i > 0) {
                        os << ",";
                    }
                    os << bookingJson(pendingList[i], "Pending");
                }
                os << "],\"canUndo\":" << (bookings.canUndo() ? "true" : "false") << "}}";
                cout << os.str() << endl;

            } else if (cmd == "HUFFMAN") {
                ostringstream receipt;
                receipt << "RECEIPT VeloRent\n";
                const vector<Booking>& conf = bookings.confirmed();
                for (size_t i = 0; i < conf.size(); ++i) {
                    receipt << conf[i].customer << " rented " << conf[i].vin << " for " << conf[i].days << " days\n";
                }
                if (conf.empty()) {
                    receipt << "Ravi rented VIN1001 for 3 days\nAsha rented VIN4002 for 2 days\n";
                }
                string text = receipt.str();
                HuffmanCoder huff;
                huff.build(text);
                string bits = huff.encode(text);
                string decoded = huff.decode(bits);

                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"sampleText\":" << quoteJson(text)
                   << ",\"originalBits\":" << (text.size() * 8)
                   << ",\"compressedBits\":" << bits.size()
                   << ",\"ratioPercent\":" << formatDouble(huff.compressionRatio(text) * 100.0)
                   << ",\"roundTripMatch\":" << (decoded == text ? "true" : "false")
                   << ",\"encodedPreview\":" << quoteJson(bits.substr(0, 160))
                   << ",\"codes\":[";
                const unordered_map<char, string>& codes = huff.codes();
                bool first = true;
                for (unordered_map<char, string>::const_iterator it = codes.begin(); it != codes.end(); ++it) {
                    if (!first) {
                        os << ",";
                    }
                    first = false;
                    string label;
                    if (it->first == ' ') {
                        label = "space";
                    } else if (it->first == '\n') {
                        label = "\\n";
                    } else {
                        label = string(1, it->first);
                    }
                    os << "{\"char\":" << quoteJson(label) << ",\"code\":" << quoteJson(it->second) << "}";
                }
                os << "]}}";
                cout << os.str() << endl;

            } else if (cmd == "INSPECT") {
                ostringstream buf;
                streambuf* oldBuf = cout.rdbuf(buf.rdbuf());
                vector<Vehicle*> allV = fleet.all();
                for (size_t i = 0; i < allV.size(); ++i) {
                    allV[i]->inspectMaintenance();
                }
                cout.rdbuf(oldBuf);

                ostringstream os;
                os << "{\"ok\":true,\"data\":{\"report\":" << quoteJson(buf.str())
                   << ",\"count\":" << allV.size() << "}}";
                cout << os.str() << endl;

            } else {
                throw InvalidInputException("Unknown command: " + cmd);
            }
        } catch (const VeloException& e) {
            cout << "{\"ok\":false,\"error\":" << quoteJson(e.what()) << "}" << endl;
        } catch (const exception& e) {
            cout << "{\"ok\":false,\"error\":" << quoteJson(e.what()) << "}" << endl;
        }
    }
    return 0;
}
