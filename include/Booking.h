// Booking.h  -- MODULE 4: State & Durability
// Owner: Tianshu Aswal
//
// Topics covered:
//   DSA : binary min-heap (array based), linked-list queue (FIFO), linked-list
//         stack (LIFO), pointers, file structures (fixed-size binary records)
//   OOP : classes/objects, constructors/destructors, RAII, custom exceptions,
//         exception handling, file handling (fstream, binary mode)
//   STL : vector, fstream, optional-style API with exceptions
#ifndef VELORENT_BOOKING_H
#define VELORENT_BOOKING_H

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "Exceptions.h"

enum Tier { VIP = 0, REGULAR = 1 };       // smaller number = higher priority

struct Booking {
    int id = 0;
    std::string customer;
    std::string vin;
    int days = 0;
    int tier = REGULAR;
    double amount = 0.0;
};

inline std::ostream& operator<<(std::ostream& os, const Booking& b) {
    return os << "#" << b.id << " " << b.customer << " | " << b.vin << " | " << b.days
              << " day(s) | " << (b.tier == VIP ? "VIP" : "Regular") << " | Rs." << b.amount;
}

// ---------------------------------------------------------------------------
// Binary MIN-HEAP stored in a vector. The "smallest" booking is the most urgent:
// VIP before Regular, and earlier id (earlier request) first inside the same tier.
// push / pop = O(log n), top = O(1).
// ---------------------------------------------------------------------------
class BookingHeap {
    std::vector<Booking> a_;

    static bool before(const Booking& x, const Booking& y) {
        if (x.tier != y.tier) return x.tier < y.tier;
        return x.id < y.id;
    }
    void siftUp(std::size_t i) {
        while (i > 0) {
            std::size_t parent = (i - 1) / 2;
            if (!before(a_[i], a_[parent])) break;
            std::swap(a_[i], a_[parent]);
            i = parent;
        }
    }
    void siftDown(std::size_t i) {
        for (;;) {
            std::size_t l = 2 * i + 1, r = 2 * i + 2, best = i;
            if (l < a_.size() && before(a_[l], a_[best])) best = l;
            if (r < a_.size() && before(a_[r], a_[best])) best = r;
            if (best == i) break;
            std::swap(a_[i], a_[best]);
            i = best;
        }
    }

public:
    void push(const Booking& b) {
        a_.push_back(b);
        siftUp(a_.size() - 1);
    }
    const Booking& top() const {
        if (a_.empty()) throw NotFoundException("Heap is empty");
        return a_.front();
    }
    Booking pop() {
        if (a_.empty()) throw NotFoundException("Heap is empty");
        Booking best = a_.front();
        a_.front() = a_.back();
        a_.pop_back();
        if (!a_.empty()) siftDown(0);
        return best;
    }
    std::size_t size() const { return a_.size(); }
    bool empty() const { return a_.empty(); }
};

// ---------------------------------------------------------------------------
// FIFO queue built on a singly linked list (head = front, tail = back).
// enqueue / dequeue = O(1).
// ---------------------------------------------------------------------------
class BookingQueue {
    struct Node {
        Booking data;
        Node* next;
    };
    Node* head_ = nullptr;
    Node* tail_ = nullptr;
    std::size_t size_ = 0;

public:
    BookingQueue() = default;
    BookingQueue(const BookingQueue&) = delete;
    BookingQueue& operator=(const BookingQueue&) = delete;
    ~BookingQueue() {
        while (head_) {
            Node* n = head_;
            head_ = head_->next;
            delete n;
        }
    }

    void enqueue(const Booking& b) {
        Node* n = new Node{b, nullptr};
        if (tail_) tail_->next = n;
        else head_ = n;
        tail_ = n;
        ++size_;
    }
    Booking dequeue() {
        if (!head_) throw NotFoundException("Queue is empty");
        Node* n = head_;
        Booking b = n->data;
        head_ = n->next;
        if (!head_) tail_ = nullptr;
        delete n;
        --size_;
        return b;
    }
    const Booking& front() const {
        if (!head_) throw NotFoundException("Queue is empty");
        return head_->data;
    }
    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
};

// ---------------------------------------------------------------------------
// LIFO stack on a linked list. Used to undo the most recent booking first.
// push / pop / top = O(1).
// ---------------------------------------------------------------------------
class UndoStack {
    struct Node {
        Booking data;
        Node* next;
    };
    Node* top_ = nullptr;
    std::size_t size_ = 0;

public:
    UndoStack() = default;
    UndoStack(const UndoStack&) = delete;
    UndoStack& operator=(const UndoStack&) = delete;
    ~UndoStack() {
        while (top_) {
            Node* n = top_;
            top_ = top_->next;
            delete n;
        }
    }

    void push(const Booking& b) {
        top_ = new Node{b, top_};
        ++size_;
    }
    Booking pop() {
        if (!top_) throw NotFoundException("Nothing to undo");
        Node* n = top_;
        Booking b = n->data;
        top_ = n->next;
        delete n;
        --size_;
        return b;
    }
    const Booking& top() const {
        if (!top_) throw NotFoundException("Nothing to undo");
        return top_->data;
    }
    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }
};

// ---------------------------------------------------------------------------
// BookingManager ties the three structures together:
//   VIP requests  -> min-heap   (always served first)
//   Regular       -> FIFO queue (served in arrival order)
//   Confirmed     -> pushed on the undo stack so the latest can be rolled back
// ---------------------------------------------------------------------------
class BookingManager {
    BookingHeap vipHeap_;
    BookingQueue regularQueue_;
    UndoStack undo_;
    std::vector<Booking> confirmed_;
    int nextId_ = 1;

public:
    int submit(const std::string& customer, const std::string& vin, int days, int tier,
               double amount) {
        if (customer.empty()) throw InvalidInputException("Customer name is required");
        if (vin.empty()) throw InvalidInputException("Vehicle VIN is required");
        if (days <= 0) throw InvalidInputException("Days must be positive");
        if (tier != VIP && tier != REGULAR) throw InvalidInputException("Invalid tier");
        Booking b{nextId_++, customer, vin, days, tier, amount};
        if (tier == VIP) vipHeap_.push(b);
        else regularQueue_.enqueue(b);
        return b.id;
    }

    std::size_t pendingCount() const { return vipHeap_.size() + regularQueue_.size(); }

    // Confirms the next booking: VIPs first, then regular in FIFO order.
    Booking processNext() {
        if (pendingCount() == 0) throw NotFoundException("No pending bookings");
        Booking b = !vipHeap_.empty() ? vipHeap_.pop() : regularQueue_.dequeue();
        confirmed_.push_back(b);
        undo_.push(b);
        return b;
    }

    // Rolls back the most recently confirmed booking.
    Booking undoLast() {
        Booking b = undo_.pop();
        confirmed_.erase(std::remove_if(confirmed_.begin(), confirmed_.end(),
                                        [&](const Booking& x) { return x.id == b.id; }),
                         confirmed_.end());
        return b;
    }

    // Used on start-up: reload bookings that were saved in ledger.bin.
    void restoreConfirmed(const std::vector<Booking>& saved) {
        for (const Booking& b : saved) {
            confirmed_.push_back(b);
            if (b.id >= nextId_) nextId_ = b.id + 1;
        }
    }

    const std::vector<Booking>& confirmed() const { return confirmed_; }
    bool canUndo() const { return !undo_.empty(); }
};

// ---------------------------------------------------------------------------
// RAII file wrapper: the file is opened in the constructor and ALWAYS closed
// in the destructor, even if an exception is thrown in between.
// ---------------------------------------------------------------------------
class FileGuard {
    std::fstream file_;
    std::string path_;

public:
    FileGuard(const std::string& path, std::ios::openmode mode) : file_(path, mode), path_(path) {
        if (!file_.is_open()) throw FileIOException("Cannot open file: " + path);
    }
    ~FileGuard() {
        if (file_.is_open()) file_.close();
    }
    FileGuard(const FileGuard&) = delete;
    FileGuard& operator=(const FileGuard&) = delete;

    std::fstream& stream() { return file_; }
};

// ---------------------------------------------------------------------------
// Ledger: stores confirmed bookings in a BINARY file (ledger.bin).
// Each booking is one fixed-size record, so the file can be read back record
// by record. Every append is flushed, so a crash loses at most the booking
// being written; a half-written last record is detected and ignored on load.
// ---------------------------------------------------------------------------
class Ledger {
    struct Record {                  // plain data only (no std::string) -> safe to write raw
        int id;
        int tier;
        int days;
        double amount;
        char customer[32];
        char vin[20];
    };

    std::string path_;

    static Record toRecord(const Booking& b) {
        Record r;
        std::memset(&r, 0, sizeof(r));
        r.id = b.id;
        r.tier = b.tier;
        r.days = b.days;
        r.amount = b.amount;
        std::strncpy(r.customer, b.customer.c_str(), sizeof(r.customer) - 1);
        std::strncpy(r.vin, b.vin.c_str(), sizeof(r.vin) - 1);
        return r;
    }

    static Booking fromRecord(const Record& r) {
        return Booking{r.id, r.customer, r.vin, r.days, r.tier, r.amount};
    }

public:
    explicit Ledger(std::string path) : path_(std::move(path)) {}

    void append(const Booking& b) {
        FileGuard g(path_, std::ios::binary | std::ios::out | std::ios::app);
        Record r = toRecord(b);
        g.stream().write(reinterpret_cast<const char*>(&r), sizeof(r));
        g.stream().flush();
        if (!g.stream()) throw FileIOException("Write failed: " + path_);
    }

    // Overwrites the file with exactly these bookings (used after an undo).
    void saveAll(const std::vector<Booking>& list) {
        FileGuard g(path_, std::ios::binary | std::ios::out | std::ios::trunc);
        for (const Booking& b : list) {
            Record r = toRecord(b);
            g.stream().write(reinterpret_cast<const char*>(&r), sizeof(r));
        }
        g.stream().flush();
        if (!g.stream()) throw FileIOException("Write failed: " + path_);
    }

    // A missing file simply means "no bookings saved yet".
    std::vector<Booking> loadAll() const {
        std::vector<Booking> out;
        std::ifstream probe(path_, std::ios::binary);
        if (!probe.is_open()) return out;
        probe.close();

        FileGuard g(path_, std::ios::binary | std::ios::in);
        Record r;
        while (g.stream().read(reinterpret_cast<char*>(&r), sizeof(r)))
            out.push_back(fromRecord(r));
        return out;                  // a partial last record fails read() and is skipped
    }

    void clear() { saveAll({}); }
    const std::string& path() const { return path_; }
};

#endif
