# VeloRent - Smart Vehicle Rental Management System

Team **The Hamsters** | 3rd Semester B.Tech CSE | Graphic Era Hill University
Combined PBL: Data Structures (TCS302) + OOP with C++ (TCS307)

A standalone C++17 console application. No database, no third-party libraries.

## Build and run

```
make          # builds ./velorent
./velorent    # starts the menu
make test     # builds and runs the 81 self-checking tests
make clean
```

No `make`? Use the compiler directly:

```
g++ -std=c++17 -Iinclude src/main.cpp -o velorent
g++ -std=c++17 -Iinclude tests/tests.cpp -o run_tests
```

## Who owns what

| Module | Owner | File | Main topics |
|---|---|---|---|
| 1. Fleet core | Paras Rawat | `include/Vehicle.h` | Abstract class, inheritance, polymorphism, operator overloading, custom hash table (separate chaining + rehash), recursive merge sort, recursive binary search |
| 2. Graph and routing | Rakesh Kumar Sanwal | `include/HubGraph.h` | Graph (adjacency list), Dijkstra, Floyd-Warshall, Kruskal MST + Union-Find, Strategy pattern, STL priority_queue |
| 3. Search and trees | Brijesh Gaur | `include/RateTree.h` | AVL tree (class template, rotations), range search, operator overloading, Huffman coding |
| 4. State and durability | Tianshu Aswal | `include/Booking.h`, `include/Exceptions.h` | Min-heap, linked-list queue, linked-list stack, RAII, binary file I/O (`ledger.bin`), custom exceptions |
| Integration | everyone | `src/main.cpp` | Menu that connects all four modules |

## How the modules connect

```
                    main.cpp (menu)
   +----------+-----------+-------------+--------------+
   |          |           |             |              |
 Fleet     HubGraph   AVLTree<Rate>   BookingManager  Ledger
(hash,    (Dijkstra,  (price range,   (heap + queue   (ledger.bin,
 sort)     MST)        Huffman)        + undo stack)   RAII)
```

## What is done in Phase 2 and what is pending

Done: everything listed in the table above, plus the menu and 81 passing tests.

Pending (planned for Phase 3):
- Paras: `ReportGenerator` using `std::sort`, `std::accumulate`, `std::count_if`; remove a vehicle from the fleet; load vehicles from a file.
- Rakesh: Bellman-Ford; Prim's MST as a comparison with Kruskal; load hubs and routes from a file; add Floyd-Warshall to the menu.
- Brijesh: AVL deletion; write Huffman output as real packed bits to a file (it is a '0'/'1' text string now); keep the price index in sync when a rate changes.
- Tianshu: keep the undo history across restarts; add dates to bookings; more crash-recovery tests.

## Using Git and GitHub (first time)

Git saves the history of your code. GitHub is a website that stores your Git project online so all four of you share one copy.

1. One person (the team lead) creates a free account at github.com, clicks **New repository**, names it `VeloRent`, and leaves "Add a README" unticked.
2. In this folder run:
   ```
   git init
   git add .
   git commit -m "Initial VeloRent code: all four modules"
   git branch -M main
   git remote add origin https://github.com/<username>/VeloRent.git
   git push -u origin main
   ```
3. On the repo page go to **Settings > Collaborators** and add the other three by their GitHub usernames.
4. Each teammate runs `git clone https://github.com/<username>/VeloRent.git`.
5. Daily routine for everyone:
   ```
   git pull                       # get the latest
   # ... edit YOUR module file ...
   git add include/YourFile.h
   git commit -m "Add <what you did>"
   git push
   ```
Each member edits mainly their own file, so you will rarely conflict. Make several small commits instead of one big one: the commit history shows evaluators who contributed.
