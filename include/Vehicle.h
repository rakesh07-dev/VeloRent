// Vehicle.h  -- MODULE 1: Fleet Core
// Owner: Paras Rawat
//
// Topics covered:
//   OOP : classes/objects, constructors, inheritance, polymorphism (virtual
//         functions + abstract class), operator overloading (<, <<),
//         exception handling
//   DSA : custom hash table (separate chaining) with rehashing, linked-list
//         chains, pointers, RECURSIVE merge sort, RECURSIVE binary search,
//         arrays/vectors
#ifndef VELORENT_VEHICLE_H
#define VELORENT_VEHICLE_H

#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Exceptions.h"

// ---------------------------------------------------------------------------
// Abstract base class. A Vehicle cannot be created directly: every concrete
// type must say how it calculates its own tariff (polymorphism).
// ---------------------------------------------------------------------------
class Vehicle {
protected:
    std::string vin_;
    std::string plate_;
    std::string brand_;
    double baseRate_;        // price per day
    std::string status_;     // "Available" or "Booked"

    static void checkDays(int days) {
        if (days <= 0) throw InvalidInputException("Number of days must be positive");
    }

public:
    Vehicle(std::string vin, std::string plate, std::string brand, double rate)
        : vin_(std::move(vin)), plate_(std::move(plate)), brand_(std::move(brand)),
          baseRate_(rate), status_("Available") {
        if (vin_.empty() || plate_.empty())
            throw InvalidInputException("VIN and licence plate cannot be empty");
        if (rate <= 0) throw InvalidInputException("Base rate must be positive");
    }

    // Virtual destructor: needed so deleting a Car through a Vehicle* is safe.
    virtual ~Vehicle() = default;

    const std::string& getVIN() const { return vin_; }
    const std::string& getLicensePlate() const { return plate_; }
    const std::string& getBrand() const { return brand_; }
    double getBaseRate() const { return baseRate_; }
    const std::string& getStatus() const { return status_; }
    void setStatus(const std::string& s) { status_ = s; }
    bool isAvailable() const { return status_ == "Available"; }

    // Pure virtual = every subclass MUST provide these.
    virtual double calculateTariff(int days) const = 0;
    virtual void inspectMaintenance() const = 0;
    virtual std::string type() const = 0;

    virtual void display(std::ostream& os = std::cout) const {
        os << std::left << std::setw(11) << type() << " | VIN " << std::setw(8) << vin_
           << " | " << std::setw(11) << plate_ << " | " << std::setw(15) << brand_
           << " | Rs." << std::setw(6) << std::fixed << std::setprecision(0) << baseRate_
           << "/day | " << status_;
    }

    // Operator overloading: vehicles compare by daily rate.
    bool operator<(const Vehicle& other) const { return baseRate_ < other.baseRate_; }
    bool operator>(const Vehicle& other) const { return baseRate_ > other.baseRate_; }
};

// Operator overloading: lets us write  cout << vehicle;
inline std::ostream& operator<<(std::ostream& os, const Vehicle& v) {
    v.display(os);
    return os;
}

// ---------------------------------------------------------------------------
// Concrete vehicle types (inheritance). Each overrides calculateTariff().
// ---------------------------------------------------------------------------
class Car : public Vehicle {
    int seats_;
public:
    Car(std::string vin, std::string plate, std::string brand, double rate, int seats = 5)
        : Vehicle(std::move(vin), std::move(plate), std::move(brand), rate), seats_(seats) {}

    double calculateTariff(int days) const override {
        checkDays(days);
        return baseRate_ * days;
    }
    void inspectMaintenance() const override {
        std::cout << "[Car " << vin_ << "] Engine oil, tyre pressure and brakes checked ("
                  << seats_ << " seats).\n";
    }
    std::string type() const override { return "Car"; }
};

class ElectricVehicle : public Vehicle {
    double batteryKWh_;
    int batteryPct_;
public:
    ElectricVehicle(std::string vin, std::string plate, std::string brand, double rate,
                    double batteryKWh, int batteryPct)
        : Vehicle(std::move(vin), std::move(plate), std::move(brand), rate),
          batteryKWh_(batteryKWh), batteryPct_(batteryPct) {}

    // Flat Rs.120 charging fee added to every rental.
    double calculateTariff(int days) const override {
        checkDays(days);
        return baseRate_ * days + 120.0;
    }
    void inspectMaintenance() const override {
        std::ios::fmtflags old = std::cout.flags();
        std::cout << "[EV " << vin_ << "] Battery " << batteryPct_ << "% of " << std::fixed
                  << std::setprecision(1) << batteryKWh_ << " kWh. "
                  << (batteryPct_ < 30 ? "NEEDS CHARGING.\n" : "Battery OK.\n");
        std::cout.flags(old);
    }
    std::string type() const override { return "Electric"; }
};

class LuxuryBus : public Vehicle {
    int seats_;
    double driverPerDay_;
public:
    LuxuryBus(std::string vin, std::string plate, std::string brand, double rate,
              int seats, double driverPerDay)
        : Vehicle(std::move(vin), std::move(plate), std::move(brand), rate),
          seats_(seats), driverPerDay_(driverPerDay) {}

    // Bus always comes with a driver, charged per day.
    double calculateTariff(int days) const override {
        checkDays(days);
        return (baseRate_ + driverPerDay_) * days;
    }
    void inspectMaintenance() const override {
        std::cout << "[Bus " << vin_ << "] Seats (" << seats_ << "), AC, emergency exits and "
                  << "driver log checked.\n";
    }
    std::string type() const override { return "LuxuryBus"; }
};

class Bike : public Vehicle {
public:
    Bike(std::string vin, std::string plate, std::string brand, double rate)
        : Vehicle(std::move(vin), std::move(plate), std::move(brand), rate) {}

    // 10% weekly discount when rented for 7 days or more.
    double calculateTariff(int days) const override {
        checkDays(days);
        double total = baseRate_ * days;
        return days >= 7 ? total * 0.90 : total;
    }
    void inspectMaintenance() const override {
        std::cout << "[Bike " << vin_ << "] Chain, helmet and brakes checked.\n";
    }
    std::string type() const override { return "Bike"; }
};

// ---------------------------------------------------------------------------
// Custom hash table with SEPARATE CHAINING (each bucket is a linked list).
// Keys are strings (VIN or plate). Average search = O(1).
// Rehashes (doubles the buckets) when load factor exceeds 0.75.
// The table does NOT own the Vehicle objects, it only points at them.
// ---------------------------------------------------------------------------
class VehicleHashTable {
    struct Node {
        std::string key;
        Vehicle* value;
        Node* next;
    };

    std::vector<Node*> buckets_;
    std::size_t count_ = 0;

    // djb2 string hash
    static std::size_t hashKey(const std::string& key) {
        std::size_t h = 5381;
        for (unsigned char c : key) h = h * 33 + c;
        return h;
    }

    void rehash(std::size_t newCap) {
        std::vector<Node*> fresh(newCap, nullptr);
        for (Node* head : buckets_) {
            while (head) {
                Node* next = head->next;                 // detach node and reinsert
                std::size_t idx = hashKey(head->key) % newCap;
                head->next = fresh[idx];
                fresh[idx] = head;
                head = next;
            }
        }
        buckets_.swap(fresh);
    }

public:
    explicit VehicleHashTable(std::size_t capacity = 8) : buckets_(capacity ? capacity : 8, nullptr) {}

    VehicleHashTable(const VehicleHashTable&) = delete;             // owns raw nodes,
    VehicleHashTable& operator=(const VehicleHashTable&) = delete;  // so no copying

    ~VehicleHashTable() {
        for (Node* head : buckets_) {
            while (head) {
                Node* next = head->next;
                delete head;
                head = next;
            }
        }
    }

    // Returns false (and does nothing) if the key already exists.
    bool insert(const std::string& key, Vehicle* v) {
        if (find(key)) return false;
        if (static_cast<double>(count_ + 1) / buckets_.size() > 0.75) rehash(buckets_.size() * 2);
        std::size_t idx = hashKey(key) % buckets_.size();
        buckets_[idx] = new Node{key, v, buckets_[idx]};   // insert at head of chain
        ++count_;
        return true;
    }

    Vehicle* find(const std::string& key) const {
        for (Node* n = buckets_[hashKey(key) % buckets_.size()]; n; n = n->next)
            if (n->key == key) return n->value;
        return nullptr;
    }

    bool remove(const std::string& key) {
        std::size_t idx = hashKey(key) % buckets_.size();
        Node* prev = nullptr;
        for (Node* n = buckets_[idx]; n; prev = n, n = n->next) {
            if (n->key == key) {
                (prev ? prev->next : buckets_[idx]) = n->next;
                delete n;
                --count_;
                return true;
            }
        }
        return false;
    }

    std::size_t size() const { return count_; }
    std::size_t bucketCount() const { return buckets_.size(); }
    double loadFactor() const { return static_cast<double>(count_) / buckets_.size(); }

    std::size_t longestChain() const {
        std::size_t best = 0;
        for (Node* head : buckets_) {
            std::size_t len = 0;
            for (Node* n = head; n; n = n->next) ++len;
            if (len > best) best = len;
        }
        return best;
    }
};

// ---------------------------------------------------------------------------
// Fleet: owns all vehicles (unique_ptr) and keeps two hash indexes, one by
// VIN and one by licence plate, so either lookup is O(1) on average.
// ---------------------------------------------------------------------------
class Fleet {
    std::vector<std::unique_ptr<Vehicle>> owned_;
    VehicleHashTable byVin_;
    VehicleHashTable byPlate_;

    // Recursive merge sort on the range [lo, hi) -- sorts by daily rate.
    static void mergeSortRec(std::vector<Vehicle*>& a, std::vector<Vehicle*>& tmp,
                             std::size_t lo, std::size_t hi) {
        if (hi - lo < 2) return;                          // base case
        std::size_t mid = lo + (hi - lo) / 2;
        mergeSortRec(a, tmp, lo, mid);                    // sort left half
        mergeSortRec(a, tmp, mid, hi);                    // sort right half
        std::size_t i = lo, j = mid, k = lo;              // merge
        while (i < mid && j < hi)
            tmp[k++] = (a[j]->getBaseRate() < a[i]->getBaseRate()) ? a[j++] : a[i++];
        while (i < mid) tmp[k++] = a[i++];
        while (j < hi) tmp[k++] = a[j++];
        for (k = lo; k < hi; ++k) a[k] = tmp[k];
    }

    static int binarySearchRec(const std::vector<Vehicle*>& a, double rate, int lo, int hi) {
        if (lo > hi) return -1;                           // base case: not found
        int mid = lo + (hi - lo) / 2;
        double r = a[mid]->getBaseRate();
        if (r == rate) return mid;
        return r < rate ? binarySearchRec(a, rate, mid + 1, hi)
                        : binarySearchRec(a, rate, lo, mid - 1);
    }

public:
    Vehicle& add(std::unique_ptr<Vehicle> v) {
        if (!v) throw InvalidInputException("Null vehicle");
        if (byVin_.find(v->getVIN()))
            throw DuplicateKeyException("Duplicate VIN: " + v->getVIN());
        if (byPlate_.find(v->getLicensePlate()))
            throw DuplicateKeyException("Duplicate plate: " + v->getLicensePlate());
        Vehicle* raw = v.get();
        byVin_.insert(raw->getVIN(), raw);
        byPlate_.insert(raw->getLicensePlate(), raw);
        owned_.push_back(std::move(v));
        return *raw;
    }

    Vehicle* findByVin(const std::string& vin) const { return byVin_.find(vin); }
    Vehicle* findByPlate(const std::string& plate) const { return byPlate_.find(plate); }

    Vehicle& getByVin(const std::string& vin) const {
        Vehicle* v = byVin_.find(vin);
        if (!v) throw NotFoundException("No vehicle with VIN " + vin);
        return *v;
    }

    std::size_t size() const { return owned_.size(); }

    std::vector<Vehicle*> all() const {
        std::vector<Vehicle*> out;
        for (const auto& p : owned_) out.push_back(p.get());
        return out;
    }

    static void mergeSort(std::vector<Vehicle*>& a) {
        std::vector<Vehicle*> tmp(a.size());
        mergeSortRec(a, tmp, 0, a.size());
    }

    std::vector<Vehicle*> sortedByRate() const {
        std::vector<Vehicle*> v = all();
        mergeSort(v);
        return v;
    }

    // `sorted` must already be sorted by rate. Returns index or -1.
    static int binarySearchRate(const std::vector<Vehicle*>& sorted, double rate) {
        return binarySearchRec(sorted, rate, 0, static_cast<int>(sorted.size()) - 1);
    }

    const VehicleHashTable& vinIndex() const { return byVin_; }
};

#endif
