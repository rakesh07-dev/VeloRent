# VeloRent Web Frontend & C++14 Bridge

## Architecture

```text
Browser (React + Vite :5173) ──HTTP/JSON──► Node.js Bridge (Express :3001) ──stdin/stdout──► velorent_api (C++14)
```

- **Single source of truth:** All fleet sorting (merge sort), lookups (djb2 hash table, AVL range search), routing (Dijkstra, Kruskal, Prim, BFS, DFS, Bellman-Ford), drop-off pricing (Strategy pattern), VIP min-heap, undo stack, Huffman coding, and binary `ledger.bin` persistence execute inside the C++14 `velorent_api` binary.
- **No duplicated business logic:** JavaScript never computes shortest paths, tariffs, or drop-off charges.
- **Original C++ headers & console app preserved:** `include/*.h`, `src/main.cpp`, and `tests/tests.cpp` remain untouched.

## How to Run (3 Commands)

From the `VeloRent/` repository root:

1. **Build the C++14 API binary:**
   ```powershell
   g++ -std=c++14 -Wall -Wextra -pedantic -Iinclude src/api_main.cpp -o velorent_api.exe
   ```
   *(On Linux/macOS or with `make`: `make api`)*

2. **Install & start the Node.js bridge:**
   ```powershell
   cd server; npm install; npm start
   ```

3. **In a second terminal, install & start the React frontend:**
   ```powershell
   cd web; npm install; npm run dev
   ```
   Open `http://localhost:5173`.

## Running Automated Acceptance Checks

With `velorent_api` built in the repository root:

```powershell
cd server
npm run check
```

## Honest Status Table

| Item | Status | Notes |
|---|---|---|
| C++14 `src/api_main.cpp` | Generated (ready to compile) | Uses `ostringstream` & `strtol` for MinGW GCC 6.3 compatibility; no C++17 features |
| Node.js Express bridge (`server/`) | Generated (ready to run) | Serializes requests, enforces 5 s timeout, handles restarts |
| React + Vite UI (`web/`) | Generated (ready to run) | 5 screens: Dashboard, Fleet, Bookings, Route Planner, DSA & Algorithms |
| Live execution / MinGW GCC 6.3 build | **Not yet run in this chat environment** | Compile and run `make test` + `npm run check` on your local machine |
