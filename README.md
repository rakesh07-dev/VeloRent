# VeloRent — Smart Vehicle Rental Management System

## 1. Project Title and Introduction

**Team The Hamsters** · B.Tech CSE, Semester III · Graphic Era (Deemed to be University), Dehradun
Combined PBL: **TCS302 Data Structures** + **TCS307 Object-Oriented Programming with C++**
Repository: https://github.com/rakesh07-dev/VeloRent

> **Phase 3 status:** the four modules are joined in one console menu (`src/main.cpp`) and covered by 91 automated checks.
> Remaining gaps are listed honestly in [Current Implementation Status](#18-current-implementation-status).

VeloRent is a console-based vehicle rental system built to apply Data Structures (TCS302) and OOP in C++ (TCS307) on one realistic problem.

---

## 2. Project Overview

VeloRent models a multi-city vehicle rental company. A customer can browse the fleet, find a vehicle, pick pickup and drop hubs, get the cheapest route and the drop-off charge, and make a booking. VIP bookings are served first, the last booking can be undone, and confirmed bookings are saved to a file.

It is a **C++14 console application**. There is no GUI, no database and no third-party library. Most data structures (hash table, AVL tree, heap, linked-list queue and stack, Huffman tree) are written by hand; the STL is used for `vector`, `unordered_map`, `priority_queue` and `sort` in the graph and Huffman code.

## 3. Key Features

- Vehicle hierarchy: `Car`, `Bike`, `ElectricVehicle`, `LuxuryBus` (inheritance and polymorphism).
- Fast VIN and plate lookup with a custom hash table; fleet sorted by rate with merge sort.
- Price-range search with an AVL tree.
- Hub network with shortest route (Dijkstra), all-pairs distances (Floyd-Warshall), minimum spanning tree (Kruskal and Prim), reachability (BFS, DFS), Bellman-Ford.
- Drop-off charge chosen at run time with the Strategy pattern.
- VIP min-heap, regular queue, undo stack.
- Bookings saved in a binary file (`ledger.bin`).
- Huffman compression of receipt text.
- Custom exception hierarchy and input validation for handling invalid user input and file errors.

## 4. System Workflow

```mermaid
flowchart TD
    A[Start: initialize fleet and hubs; load ledger.bin] --> B[Main menu 0-14]
    B --> C[Browse / search vehicles]
    B --> D[Route + drop-off charge]
    B --> E[New booking request]
    E -->|VIP| F[Min-heap]
    E -->|Regular| G[Linked-list queue]
    F --> H[Process next booking]
    G --> H
    H --> I[Vehicle marked Booked]
    I --> J[Append record to ledger.bin]
    H -.-> K[Undo stack]
    K --> L[Undo last confirmed booking: rewrite ledger.bin]
```

## 5. Technology Stack

| Item | Detail |
|---|---|
| Language | C++14 (`g++ -std=c++14`) |
| Tested compilers | GCC 13 (Linux) for the Phase 3 build. Project written for MinGW GCC 6.3 on Windows; **rebuild on that machine before the demo**. |
| Tools | VS Code, Git/GitHub, Makefile (optional) |
| Libraries | C++ standard library only |

## 6. System Architecture

```mermaid
flowchart LR
    M[src/main.cpp menu] --> V[Vehicle.h Fleet + hash table + merge sort]
    M --> G[HubGraph.h graph algorithms + Strategy]
    M --> R[RateTree.h AVL tree + Huffman]
    M --> B[Booking.h heap + queue + stack + Ledger]
    V & G & R & B --> X[Exceptions.h]
```

## 7. Module Description

| Module | Owner | File | Contents |
|---|---|---|---|
| 1 Fleet Core | Paras Rawat | `include/Vehicle.h` | Abstract `Vehicle`, `Car`, `Bike`, `ElectricVehicle`, `LuxuryBus`, `VehicleHashTable`, `Fleet`, merge sort, binary search by rate |
| 2 Graph and Routing | Rakesh Kumar Sanwal (lead) | `include/HubGraph.h` | `HubGraph`, Dijkstra, Floyd-Warshall, Kruskal + Union-Find, Prim, BFS, DFS, Bellman-Ford, drop-off Strategy classes |
| 3 Search and Trees | Brijesh Gaur | `include/RateTree.h` | `AVLTree<T>`, `RateEntry`, range search, `HuffmanCoder` |
| 4 Bookings | Tianshu Aswal | `include/Booking.h`, `include/Exceptions.h` | `BookingHeap`, `BookingQueue`, `UndoStack`, `BookingManager`, `Ledger`, `FileGuard` (RAII), exceptions |
| Integration | Team | `src/main.cpp`, `tests/tests.cpp` | Unified menu and test suite |

## 8. Data Structures and Algorithms

| Structure / algorithm | Used for | Why |
|---|---|---|
| Hash table (separate chaining, rehash at load 0.75, djb2 hash) | VIN and plate lookup | O(1) average lookup, better than scanning the fleet |
| Merge sort (recursive) | Fleet sorted by rate | Stable, O(n log n) in every case |
| Binary search (recursive) | Search by rate in the rate-sorted list. VIN and plate lookup use the hash table, not binary search. | O(log n) on sorted data |
| AVL tree | Price index and range search | Stays balanced, so search is O(log n). Ties in price are broken by VIN. |
| Min-heap (array) | VIP booking priority | Next VIP in O(log n) |
| Linked-list queue | Regular bookings | FIFO with O(1) enqueue and dequeue |
| Linked-list stack | Undo of bookings | LIFO rollback |
| Graph, adjacency list | Hub network | Sparse graph; 14 hubs |
| Dijkstra | Cheapest route between two hubs | Non-negative distances |
| Floyd-Warshall | All-pairs distances | Cross-check for Dijkstra |
| Kruskal + Union-Find | Cheapest network linking all hubs | Edge-sorted MST |
| Prim | Same network, second method | Vertex-growing MST; its total is compared with Kruskal in menu 13 |
| BFS (queue), DFS (recursion) | Which hubs can be reached | Fewest hops first / deepest first |
| Bellman-Ford | Distances from a hub | Handles negative weights and detects negative cycles |
| Huffman coding | Compress receipt text | Frequent characters get shorter codes |

**Graph assumptions.** Routes are stored as a *directed* graph (for example Chandigarh to Delhi is 250 km but Delhi to Chandigarh is 245 km). Dijkstra and Floyd-Warshall follow the direction. `addRoute()` rejects negative costs, so Dijkstra's requirement (non-negative weights) always holds. Kruskal and Prim need an *undirected* graph, so for the MST every route is treated as two-way and the cheaper cost is kept. Bellman-Ford is tested on a separate edge list with a negative edge, because the hub graph itself never contains one.

## 9. Repository Structure

```
VeloRent/
├── include/
│   ├── Exceptions.h   custom exceptions
│   ├── Vehicle.h      Module 1
│   ├── HubGraph.h     Module 2
│   ├── RateTree.h     Module 3
│   └── Booking.h      Module 4
├── docs/
│   └── ui-prototype/index.html   dashboard UI prototype (not part of the build)
├── src/main.cpp       integrated menu
├── tests/tests.cpp    self-checking tests
├── Makefile
├── .gitignore
└── README.md
```

## 10. Prerequisites

- A C++14 compiler (`g++`; MinGW on Windows is fine) and Git.
- `make` is optional.

## 11. Installation and Setup

```
git clone https://github.com/rakesh07-dev/VeloRent.git
cd VeloRent
```

## 12. Compilation and Execution

Windows PowerShell (MinGW):

```powershell
g++ -std=c++14 -Iinclude src/main.cpp -o velorent.exe
.\velorent.exe

g++ -std=c++14 -Iinclude tests/tests.cpp -o run_tests.exe
.\run_tests.exe
```

Linux / macOS:

```bash
g++ -std=c++14 -Iinclude src/main.cpp -o velorent
./velorent

g++ -std=c++14 -Iinclude tests/tests.cpp -o run_tests
./run_tests
```

With `make`: `make`, `make test`, `make clean`. Run from the project folder, because `ledger.bin` is created in the current directory.

## 13. Application Usage

| Menu | Action |
|---|---|
| 1 | Show fleet sorted by rate |
| 2 | Find vehicle by VIN or plate |
| 3 | Vehicles in a price range |
| 4 | Cheapest route between two hubs plus drop-off charge |
| 5 | Branch network (Kruskal) |
| 6 | New booking request (VIP or regular) |
| 7 | Process next booking |
| 8 | Undo last booking (current session only; rewrites `ledger.bin`) |
| 9 | Show saved bookings from `ledger.bin` |
| 10 | Huffman compression demo |
| 11 | Fleet maintenance inspection (polymorphism) |
| 12 | BFS and DFS from a hub |
| 13 | Prim vs Kruskal |
| 14 | Bellman-Ford distances from a hub |
| 0 | Exit |

Example from the built-in data: the cheapest route Dehradun to Delhi (Dehradun, Haridwar, Roorkee, Delhi) is 261 km; the one-way charge is Rs.200 + 12 x 261 = Rs.3332.

## 14. File Handling and Data Persistence

| File | Stores | When written |
|---|---|---|
| `ledger.bin` | Confirmed bookings: id, tier, days, amount, customer (32 chars), VIN (20 chars) as fixed-size binary records | Appended when a booking is processed; fully rewritten after an undo |

- **Startup:** `ledger.bin` is read, the recovered bookings are loaded, their vehicles are marked `Booked`, and the number recovered is shown.
- **Missing file:** treated as "no bookings yet".
- **Corrupted or half-written last record:** the incomplete record is skipped on load.
- **Errors:** an unwritable path throws `FileIOException`. Files are closed automatically through the RAII class `FileGuard`.
- **Not persisted:** the vehicle list (so any other status change, such as maintenance), hubs, pending booking requests, the undo history and Huffman receipts are rebuilt or lost on every restart. Only confirmed bookings survive, and their vehicles are re-marked `Booked` at startup.
- **Undo scope:** undo works on bookings confirmed in the current session. After a restart, recovered bookings cannot be undone. An undo rewrites `ledger.bin` with the remaining confirmed bookings, including those recovered from earlier sessions.

## 15. Testing and Validation

`tests/tests.cpp` has self-checking tests for all four modules and prints `[PASS]` or `[FAIL]` for each. Result of the Phase 3 build on GCC 13, C++14: **Passed: 91, Failed: 0**. Not yet run on MinGW 6.3.

Examples: Dijkstra picks the cheaper two-hop route; Floyd-Warshall agrees with Dijkstra; Kruskal and Prim give the same total (11 on the test graph, 1148 km on the 14-hub data); BFS and DFS reach the same hubs and skip an isolated hub; Bellman-Ford handles a negative edge and detects a negative cycle; strategy charges are 0, 1400 and 1750 for 100 km; the ledger survives a missing file and a bad path.

## 16. Algorithm Complexity Analysis

| Operation | Time | Space |
|---|---|---|
| Hash lookup / insert | O(1) average, O(n) worst | O(n) |
| Merge sort | O(n log n) | O(n) |
| Binary search | O(log n) | O(1) iterative, O(log n) recursive |
| AVL insert / search | O(log n) | O(log n) recursion stack for insert (the tree itself stores O(n) nodes) |
| AVL range search | O(log n + k) | O(k) |
| Heap push / pop | O(log n) | O(n) |
| Queue / stack push, pop | O(1) | O(n) |
| Dijkstra | O((V+E) log V) | O(V+E) (the lazy priority queue can hold up to E entries) |
| Floyd-Warshall | O(V^3) | O(V^2) |
| Kruskal | O(E log E) | O(V+E) |
| Prim | O(E log V) | O(V+E) |
| BFS / DFS | O(V+E) | O(V) |
| Bellman-Ford | O(V·E) | O(V) |
| Huffman: count + build tree | O(n + k log k) | O(k) |
| Huffman: encode | O(L), L = length of the encoded output | O(L) |
| Huffman: decode | O(L) | O(n) for the decoded output string |

Space complexity here means extra memory used while the operation runs, except where the row says the structure itself (for example, the AVL tree and hash table store O(n) items; recursive binary search uses O(log n) call-stack space; merge sort needs O(n) extra space). n = items, k = distinct characters, V = hubs, E = routes.

## 17. Team Contributions

| Member | Roll No | Contribution |
|---|---|---|
| Paras Rawat | 2518504 | Fleet Core |
| Rakesh Kumar Sanwal (Lead) | 2518573 | Graph and Routing, integration, tests |
| Brijesh Gaur | 2518302 | AVL tree, Huffman |
| Tianshu Aswal | 2518708 | Bookings, ledger |

> The Phase 3 additions (BFS, DFS, Prim, Bellman-Ford, menu options 12–14, the C++14 fixes, tests grown from 81 to 91, the larger sample data and the console layout in `main.cpp`) and the dashboard UI prototype were written with AI assistance and then reviewed by the team. Every member should be ready to explain their module.

## 18. Current Implementation Status

| Feature | Status |
|---|---|
| Shared `Vehicle` hierarchy, one integrated menu | Implemented, tested |
| Custom hash table for VIN and plate | Implemented, tested |
| Merge sort, binary search | Implemented, tested |
| AVL tree, price range, equal-price ties | Implemented, tested (64 equal prices stay balanced) |
| Dijkstra, Floyd-Warshall, Kruskal, Strategy pricing | Implemented, tested |
| BFS, DFS, Prim, Bellman-Ford | Implemented, tested (new in Phase 3) |
| Heap, queue, undo stack | Implemented, tested |
| Booking persistence (`ledger.bin`) | Implemented, tested |
| Huffman coding | Implemented, tested; output is a `0`/`1` text string, not packed bits |
| Automated tests | 91 checks passing on GCC 13 |
| AVL deletion | **Pending** |
| Saving vehicles and receipts to files | **Pending** (only bookings are saved) |
| Packed-bit Huffman file | **Pending** |
| Verified build on MinGW GCC 6.3 | **Not yet verified** |
| Dashboard UI (four screens) | **Design prototype only.** Sample data, not connected to the C++ backend, not part of this repository's build. See section 19. |

## 19. Future Scope (Possible Enhancements)

VeloRent can be extended in several directions beyond the current scope. The main idea is a graphical dashboard on top of the existing C++ backend, followed by smaller feature additions.

### Graphical dashboard (design prototype exists, integration not done)

A dark-themed web dashboard has been designed as a separate **UI prototype**. It has four screens that match the console menu:

| Screen | What it shows |
|---|---|
| Dashboard | Total, available and booked vehicles, active bookings, revenue, recent booking activity |
| Fleet management | Vehicle table with VIN/registration search, type and availability filters, sorting |
| Bookings | Pending, confirmed and completed bookings, VIP/Regular tags, booking history |
| Route planner | Pickup and drop-off hubs, route, total distance, drop-off charge |

**Current status of the prototype:**

- It uses **sample data** (the demo fleet and hub distances from this project, plus invented customers, dates and revenue).
- It is **not connected** to the C++ backend. Nothing done in it changes `ledger.bin` or any vehicle.
- Its route screen runs its own small shortest-path calculation in the browser, so it does not prove that the C++ code is called.
- It is not included in the repository build and the C++ code does not depend on it.

**Planned integration (not implemented):**

1. Add a lightweight local HTTP API layer around the existing C++14 modules (for example `GET /vehicles`, `POST /bookings`, `POST /bookings/process`, `POST /bookings/undo`, `GET /route?from=&to=&mode=`).
2. Keep the C++ code as the single source of truth for fleet, booking, routing and pricing, so no business logic is duplicated in JavaScript.
3. Make the dashboard read and write only through that API.
4. Test the API, the persistence in `ledger.bin` and the existing 91 checks before calling the integration done.

Known risk for this step: the project targets MinGW GCC 6.3, where C++ threading and networking support is limited, so the server design must be chosen and tested on that compiler.

### Other enhancements

- Dates and times in bookings.
- Loading hubs and routes from a file.
- Saving the vehicle list and receipts to files.
- Packed-bit Huffman storage for receipts.
- AVL deletion.

## 20. Academic Information

Course: TCS302 Data Structures and TCS307 OOP with C++, Semester III, Department of Computer Science and Engineering, Graphic Era (Deemed to be University), Dehradun. Project phase: PBL Phase 3.

