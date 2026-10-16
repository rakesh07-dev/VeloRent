#include <iostream>
#include <string>
using namespace std;

class Vehicle
{
protected:
    string vin;
    string licensePlate;
    string brand;
    double baseRate;
    string status;

public:
    Vehicle(string v, string plate, string b, double rate)
    {
        vin = v;
        licensePlate = plate;
        brand = b;
        baseRate = rate;
        status = "Available";
    }

    string getVIN() { return vin; }
    string getLicensePlate() { return licensePlate; }
    string getBrand() { return brand; }
    double getBaseRate() { return baseRate; }

    void setStatus(string newStatus)
    {
        status = newStatus;
    }

    bool isAvailable()
    {
        return status == "Available";
    }

    virtual double calculateTariff(int days) = 0;
    virtual void inspectMaintenance() = 0;

    void display()
    {
        cout << "VIN: " << vin << endl;
        cout << "License Plate: " << licensePlate << endl;
        cout << "Brand: " << brand << endl;
        cout << "Base Rate: " << baseRate << endl;
        cout << "Status: " << status << endl;
    }
};

class Car : public Vehicle
{
public:
    Car(string v, string plate, string b, double rate)
        : Vehicle(v, plate, b, rate)
    {
    }

    double calculateTariff(int days) override
    {
        return baseRate * days;
    }

    void inspectMaintenance() override
    {
        cout << "Car maintenance checked." << endl;
    }
};


struct CustomerNode
{
    string name;
    CustomerNode* next;

    CustomerNode(string n)
    {
        name = n;
        next = nullptr;
    }
};

class RegularQueue
{
private:
    CustomerNode* front;
    CustomerNode* rear;

public:
    RegularQueue()
    {
        front = nullptr;
        rear = nullptr;
    }

    void enqueue(string name)
    {
        CustomerNode* newNode = new CustomerNode(name);

        if (rear == nullptr)
        {
            front = rear = newNode;
        }
        else
        {
            rear->next = newNode;
            rear = newNode;
        }

        cout << name << " added to regular waiting queue." << endl;
    }

    string dequeue()
    {
        if (front == nullptr)
            return "";

        CustomerNode* temp = front;
        string name = temp->name;

        front = front->next;

        if (front == nullptr)
            rear = nullptr;

        delete temp;
        return name;
    }

    void display()
    {
        if (front == nullptr)
        {
            cout << "Regular waiting queue is empty." << endl;
            return;
        }

        cout << "Regular Queue: ";
        CustomerNode* temp = front;

        while (temp != nullptr)
        {
            cout << temp->name << " -> ";
            temp = temp->next;
        }

        cout << "NULL" << endl;
    }

    ~RegularQueue()
    {
        while (front != nullptr)
        {
            CustomerNode* temp = front;
            front = front->next;
            delete temp;
        }
    }
};

struct VIPCustomer
{
    string name;
    int priority;

    VIPCustomer()
    {
        name = "";
        priority = -1;
    }

    VIPCustomer(string n, int p)
    {
        name = n;
        priority = p;
    }
};

class MinHeap
{
private:
    VIPCustomer heap[100];
    int size;

    void swapCustomers(int first, int second)
    {
        VIPCustomer temp = heap[first];
        heap[first] = heap[second];
        heap[second] = temp;
    }

    void heapifyUp(int index)
    {
        while (index > 0)
        {
            int parent = (index - 1) / 2;

            if (heap[parent].priority <= heap[index].priority)
                break;

            swapCustomers(parent, index);
            index = parent;
        }
    }

    void heapifyDown(int index)
    {
        while (true)
        {
            int left = 2 * index + 1;
            int right = 2 * index + 2;
            int smallest = index;

            if (left < size &&
                heap[left].priority < heap[smallest].priority)
            {
                smallest = left;
            }

            if (right < size &&
                heap[right].priority < heap[smallest].priority)
            {
                smallest = right;
            }

            if (smallest == index)
                break;

            swapCustomers(index, smallest);
            index = smallest;
        }
    }

public:
    MinHeap()
    {
        size = 0;
    }

    void insert(string name, int priority)
    {
        if (size == 100)
        {
            cout << "VIP queue is full." << endl;
            return;
        }

        heap[size] = VIPCustomer(name, priority);
        heapifyUp(size);
        size++;

        cout << name << " added to VIP priority queue." << endl;
    }

    VIPCustomer extractMin()
    {
        if (size == 0)
            return VIPCustomer("", -1);

        VIPCustomer result = heap[0];

        size--;

        if (size > 0)
        {
            heap[0] = heap[size];
            heapifyDown(0);
        }

        return result;
    }

    void display()
    {
        if (size == 0)
        {
            cout << "VIP queue is empty." << endl;
            return;
        }

        cout << "VIP Heap: ";
        for (int i = 0; i < size; i++)
        {
            cout << heap[i].name << "(P" << heap[i].priority << ") ";
        }
        cout << endl;
    }
};

class UndoStack
{
private:
    string stack[100];
    int topIndex;

public:
    UndoStack()
    {
        topIndex = -1;
    }

    void push(string action)
    {
        if (topIndex == 99)
        {
            cout << "Undo stack is full." << endl;
            return;
        }

        topIndex++;
        stack[topIndex] = action;
    }

    string pop()
    {
        if (topIndex == -1)
            return "";

        string action = stack[topIndex];
        topIndex--;
        return action;
    }

    bool empty()
    {
        return topIndex == -1;
    }

    void display()
    {
        if (topIndex == -1)
        {
            cout << "Undo stack is empty." << endl;
            return;
        }

        cout << "Undo Stack (TOP first):" << endl;

        for (int i = topIndex; i >= 0; i--)
        {
            cout << "- " << stack[i] << endl;
        }
    }
};


int main()
{
    string vin, plate_no, brand;
    double rate;
    int no_days;

    cout << "===== VeloRent - Vehicle Rental System =====" << endl;

    cout << "\nEnter VIN, Plate Number, Brand and Base Rate: ";
    cin >> vin >> plate_no >> brand >> rate;

    Car c(vin, plate_no, brand, rate);

    cout << "\nEnter number of rental days: ";
    cin >> no_days;

    cout << "\n----- Vehicle Details -----" << endl;
    c.display();

    cout << "Total Tariff: "
         << c.calculateTariff(no_days) << endl;

    c.inspectMaintenance();

    cout << "\n===== Regular Customer Queue =====" << endl;

    RegularQueue regularQueue;

    regularQueue.enqueue("brijesh");
    regularQueue.enqueue("Aman");
    regularQueue.enqueue("paras");

    regularQueue.display();

    string regularCustomer = regularQueue.dequeue();

    if (!regularCustomer.empty())
    {
        cout << "Vehicle assigned to regular customer: "
             << regularCustomer << endl;
    }

    regularQueue.display();

   
    cout << "\n===== VIP Priority Queue =====" << endl;

    MinHeap vipHeap;

    vipHeap.insert("Tianshu", 2);
    vipHeap.insert(" Rakesh", 1);
    vipHeap.insert("Neeraj", 3);

    vipHeap.display();
    VIPCustomer vip = vipHeap.extractMin();

    if (!vip.name.empty())
    {
        cout << "Highest priority VIP served first: "
             << vip.name
             << " (Priority " << vip.priority << ")" << endl;
    }
    cout << "\n===== Booking Rollback using Stack =====" << endl;

    UndoStack undoStack;

    // Simulate booking steps
    c.setStatus("Reserved");
    undoStack.push("Vehicle status changed: Available -> Reserved");

    undoStack.push("Customer assigned to vehicle");

    undoStack.push("Payment transaction recorded");

    undoStack.display();

    cout << "\nBooking failed! Starting rollback..." << endl;

    // LIFO: last action is undone first
    while (!undoStack.empty())
    {
        string action = undoStack.pop();
        cout << "Undo: " << action << endl;
    }

    // Restore original vehicle state
    c.setStatus("Available");

    cout << "\nBooking rolled back successfully." << endl;
    cout << "Vehicle status after rollback: "
         << (c.isAvailable() ? "Available" : "Not Available") << endl;

    return 0;
}
