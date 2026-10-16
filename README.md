# VeloRent — Smart Vehicle Rental Management System

Team **The Hamsters** · B.Tech CSE, 3rd Semester · Graphic Era Hill University (GEHU)
Combined PBL project: **Data Structures in C (TCS302)** + **Object-Oriented Programming with C++ (TCS307)**

VeloRent models a multi-city vehicle rental company as four console modules written in C++17. Each module uses data structures written by hand (arrays, linked lists, heap, AVL tree, Huffman tree, graph) together with OOP techniques (inheritance, polymorphism, Strategy pattern, exceptions). No database and no third-party library is used.

> **Status: Phase 2 (about 40% of the planned project).** The four modules are separate console programs. They are not yet joined into one menu.

## Quick Start

1. Clone the repository: `git clone https://github.com/rakesh07-dev/VeloRent.git`
2. Open a terminal in the `VeloRent` folder.
3. Compile the module you want to run.
4. Run the generated executable.

Example (Graph and Routing, Windows PowerShell):

```powershell
g++ -std=c++17 -x c++ include/HubGraph.h -o hubgraph.exe
.\hubgraph.exe
```

The commands for all four modules are listed in the
[Build and run](#build-and-run) section below.

## Team and modules

| Member | Roll No | Module | File |
|---|---|---|---|
| Paras Rawat | 2518504 | 1. Fleet Core: vehicle classes, registry, sort and search | `include/Vehicle.h` |
| Rakesh Kumar Sanwal (Lead) | 2518573 | 2. Graph and Routing: Dijkstra, MST, drop-off pricing | `include/HubGraph.h` |
| Brijesh Gaur | 2518302 | 3. Search and Trees: AVL price search, Huffman receipts | `include/RateTree.h` |
| Tianshu Aswal | 2518708 | 4. Bookings: VIP heap, regular queue, undo stack | `include/Booking.h` |

## What each module does

- **Fleet Core:** abstract `Vehicle` with `Car`, `Bike` and `ElectricVehicle`; add vehicles with duplicate VIN/plate checks; merge sort and binary search by VIN; rental tariff; maintenance inspection; compare two vehicles by rate; change vehicle status.
- **Graph and Routing:** hubs and routes as a weighted directed graph; cheapest route with Dijkstra; cheapest network linking all hubs with Kruskal (Union-Find); all-pairs distances with Floyd-Warshall; drop-off charge using the Strategy pattern (same-hub, one-way, peak season).
- **Search and Trees:** AVL tree of vehicles by price per hour; sorted listing and price-range search; Huffman compression and decompression of rental receipts.
- **Bookings:** linked-list queue for regular customers; array-based min-heap for VIP customers; stack that rolls back a failed booking step by step.

## Build and run

Run from the project folder. Each module is its own program:

| Module | Command |
|---|---|
| Fleet Core | `g++ -std=c++17 -x c++ include/Vehicle.h -o vehicle.exe` |
| Graph and Routing | `g++ -std=c++17 -x c++ include/HubGraph.h -o hubgraph.exe` |
| Search and Trees | `g++ -std=c++17 -x c++ include/RateTree.h -o ratetree.exe` |
| Bookings | `g++ -std=c++17 -x c++ include/Booking.h -o booking.exe` |

On Windows PowerShell, run the executable using `.\hubgraph.exe`.
Use the corresponding executable name for the other modules. The `-x c++` flag is required because the files end in `.h`; without it `g++` builds a precompiled header instead of a program. Written for MinGW GCC 6.3 (Windows); also builds with GCC 13 (Linux).

## Implementation status

| Area | Status |
|---|---|
| Vehicle hierarchy, registry, merge sort, binary search | Implemented |
| Dijkstra, Kruskal MST, Floyd-Warshall, Strategy pricing | Implemented and tested |
| AVL tree with price-range search | Implemented and tested |
| Huffman compress / decompress (bit string) | Implemented and tested |
| Regular queue, VIP heap, undo stack | Implemented (in-memory demo) |
| Integrated single menu across modules | Pending |
| File handling and saved bookings | Pending |
| Hash-table vehicle index | Pending |
| Automated test suite | Pending |

## Known limitations

- The modules do not share one `Vehicle` type, so they cannot be run together yet.
- Bookings are not saved to a file; everything is lost when a program exits.
- The AVL tree does not stay balanced when many vehicles have exactly the same price.
- Hub names are single words and case-sensitive; the graph holds at most 20 hubs.
- Huffman output is a text string of `0` and `1`, not packed bits, and archived receipts live only in memory.
- Prim's algorithm and Bellman-Ford are not implemented.

## Next steps (Phase 3)

1. Use one shared `Vehicle` class and join the four modules in a single menu.
2. Save and reload bookings with binary file handling, with error handling around it.
3. Add a custom hash table for fast VIN lookup.
4. Add automated tests and fix the limitations above.

## Repository layout

```
VeloRent/
├── include/
│   ├── Vehicle.h     Module 1: Fleet Core
│   ├── HubGraph.h    Module 2: Graph and Routing
│   ├── RateTree.h    Module 3: Search and Trees
│   └── Booking.h     Module 4: Bookings
├── .gitignore
└── README.md
```

Compiled `.exe` files are not committed.