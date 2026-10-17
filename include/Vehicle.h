#include <iostream>
#include <string>
using namespace std;
const int MAX = 100;
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
    virtual ~Vehicle() {}
    string getVIN() const
    {
        return vin;
    }
    string getLicensePlate() const
    {
        return licensePlate;
    }
    string getBrand() const
    {
        return brand;
    }
    double getBaseRate() const
    {
        return baseRate;
    }
    string getStatus() const
    {
        return status;
    }
    void setStatus(string s)
    {
        status = s;
    }
    bool isAvailable() const
    {
        return status == "Available";
    }
    bool operator>(const Vehicle& other) const
    {
        return baseRate > other.baseRate;
    }
    virtual double calculateTariff(int days) const = 0;
    virtual void inspectMaintenance() const = 0;
    virtual void display() const
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

    double calculateTariff(int days) const override
    {
        return baseRate * days;
    }

    void inspectMaintenance() const override
    {
        cout << "Car maintenance checked." << endl;
    }

    void display() const override
    {
        Vehicle::display();
        cout << "Type: Car" << endl;
    }
};
class Bike : public Vehicle
{
public:
    Bike(string v, string plate, string b, double rate)
        : Vehicle(v, plate, b, rate)
    {
    }
    double calculateTariff(int days) const override
    {
        return baseRate * days;
    }
    void inspectMaintenance() const override
    {
        cout << "Bike maintenance checked." << endl;
    }
    void display() const override
    {
        Vehicle::display();
        cout << "Type: Bike" << endl;
    }
};
class ElectricVehicle : public Vehicle
{
public:
    ElectricVehicle(string v, string plate, string b, double rate)
        : Vehicle(v, plate, b, rate)
    {
    }

    double calculateTariff(int days) const override
    {
        return baseRate * days * 0.95;
    }

    void inspectMaintenance() const override
    {
        cout << "Battery and electrical system checked." << endl;
    }

    void display() const override
    {
        Vehicle::display();
        cout << "Type: Electric Vehicle" << endl;
    }
};
void merge(Vehicle* arr[], int left, int mid, int right)
{
    Vehicle* temp[MAX];
    int i = left;
    int j = mid + 1;
    int k = 0;
    while (i <= mid && j <= right)
    {
        if (arr[i]->getVIN() <= arr[j]->getVIN())
            temp[k++] = arr[i++];
        else
            temp[k++] = arr[j++];
    }
    while (i <= mid)
        temp[k++] = arr[i++];
    while (j <= right)
        temp[k++] = arr[j++];
    for (int x = 0; x < k; x++)
        arr[left + x] = temp[x];
}
void mergeSort(Vehicle* arr[], int left, int right)
{
    if (left >= right)
        return;
    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
}
int binarySearch(Vehicle* arr[], int left, int right, string key)
{
    if (left > right)
        return -1;
    int mid = left + (right - left) / 2;
    if (arr[mid]->getVIN() == key)
        return mid;
    if (key < arr[mid]->getVIN())
        return binarySearch(arr, left, mid - 1, key);
    return binarySearch(arr, mid + 1, right, key);
}

int main()
{
    Vehicle* vehicles[MAX];
    int count = 0;
    int choice;
    do
    {
        cout << "\t WELCOME TO VEHICLE RENTING SYSTEM " << endl;
        cout << "1. Add Car" << "\n2. Add Bike"<< "\n3. Add Electric Vehicle"<< "\n4. Display All Vehicles"<< "\n5. Sort Vehicles by VIN"<< "\n6. Search Vehicle by VIN"<< "\n7. Calculate Rental Tariff"<< "\n8. Inspect Maintenance"<< "\n9. Compare Two Vehicles"<< "\n10. Change Vehicle Status"<<"\n11. Exit"<< endl;
        cin >> choice;
        switch (choice)
        {
            case 1:
            case 2:
            case 3:
            {
                if (count >= MAX)
                {
                    cout << "Vehicle storage is full." << endl;
                    break;
                }
                string vin, plate, brand;
                double rate;
                cout << "Enter VIN: ";
                cin >> vin;
                cout << "Enter License Plate: ";
                cin >> plate;
                cout << "Enter Brand: ";
                cin >> brand;
                cout << "Enter Daily Rental Rate: ";
                cin >> rate;
                if (rate <= 0)
                {
                    cout << "Rate must be positive." << endl;
                    break;
                }
                bool duplicate = false;
                for (int i = 0; i < count; i++)
                {
                    if (vehicles[i]->getVIN() == vin ||
                        vehicles[i]->getLicensePlate() == plate)
                    {
                        duplicate = true;
                        break;
                    }
                }
                if (duplicate)
                {
                    cout << "VIN or license plate already exists."
                         << endl;
                    break;
                }
                if (choice == 1)
                    vehicles[count] = new Car(vin, plate, brand, rate);
                else if (choice == 2)
                    vehicles[count] = new Bike(vin, plate, brand, rate);
                else
                    vehicles[count] =
                        new ElectricVehicle(vin, plate, brand, rate);

                count++;
                cout << "Vehicle added successfully." << endl;
                break;
            }
            case 4:
            {
                if (count == 0)
                {
                    cout << "No vehicles registered." << endl;
                    break;
                }

                for (int i = 0; i < count; i++)
                {
                    cout << "\n--------------------" << endl;
                    vehicles[i]->display();
                }
                break;
            }
            case 5:
            {
                if (count == 0)
                {
                    cout << "No vehicles to sort." << endl;
                    break;
                }
                mergeSort(vehicles, 0, count - 1);
                cout << "Vehicles sorted by VIN." << endl;

                for (int i = 0; i < count; i++)
                    cout << vehicles[i]->getVIN() << endl;

                break;
            }
            case 6:
            {
                if (count == 0)
                {
                    cout << "No vehicles registered." << endl;
                    break;
                }
                string key;
                cout << "Enter VIN to search: ";
                cin >> key;
                mergeSort(vehicles, 0, count - 1);

                int index = binarySearch(
                    vehicles, 0, count - 1, key);

                if (index != -1)
                {
                    cout << "Vehicle found." << endl;
                    vehicles[index]->display();
                }
                else
                {
                    cout << "Vehicle not found." << endl;
                }
                break;
            }
            case 7:
            {
                string key;
                int days;
                cout << "Enter VIN: ";
                cin >> key;
                int index = -1;
                for (int i = 0; i < count; i++)
                {
                    if (vehicles[i]->getVIN() == key)
                    {
                        index = i;
                        break;
                    }
                }
                if (index == -1)
                {
                    cout << "Vehicle not found." << endl;
                    break;
                }
                if (!vehicles[index]->isAvailable())
                {
                    cout << "Vehicle is not available." << endl;
                    break;
                }
                cout << "Enter number of rental days: ";
                cin >> days;
                if (days <= 0)
                {
                    cout << "Days must be positive." << endl;
                    break;
                }
                cout << "Total Tariff: "
                     << vehicles[index]->calculateTariff(days)
                     << endl;
                break;
            }
            case 8:
            {
                string key;
                cout << "Enter VIN: ";
                cin >> key;
                int index = -1;
                for (int i = 0; i < count; i++)
                {
                    if (vehicles[i]->getVIN() == key)
                    {
                        index = i;
                        break;
                    }
                }
                if (index == -1)
                    cout << "Vehicle not found." << endl;
                else
                    vehicles[index]->inspectMaintenance();

                break;
            }
            case 9:
            {
                string vin1, vin2;
                int first = -1, second = -1;
                cout << "Enter first VIN: ";
                cin >> vin1;
                cout << "Enter second VIN: ";
                cin >> vin2;
                for (int i = 0; i < count; i++)
                {
                    if (vehicles[i]->getVIN() == vin1)
                        first = i;

                    if (vehicles[i]->getVIN() == vin2)
                        second = i;
                }
                if (first == -1 || second == -1)
                {
                    cout << "Vehicle not found." << endl;
                }
                else if (*vehicles[first] > *vehicles[second])
                {
                    cout << vin1 << " has a higher daily rate."
                         << endl;
                }
                else if (*vehicles[second] > *vehicles[first])
                {
                    cout << vin2 << " has a higher daily rate."
                         << endl;
                }
                else
                {
                    cout << "Both vehicles have the same daily rate."
                         << endl;
                }
                break;
            }
            case 10:
            {
                string key, newStatus;
                cout << "Enter VIN: ";
                cin >> key;
                int index = -1;
                for (int i = 0; i < count; i++)
                {
                    if (vehicles[i]->getVIN() == key)
                    {
                        index = i;
                        break;
                    }
                }
                if (index == -1)
                {
                    cout << "Vehicle not found." << endl;
                    break;
                }
                cout << "Enter status (Available, Rented, In-Service): ";
                cin >> newStatus;
                if (newStatus == "Available" ||
                    newStatus == "Rented" ||
                    newStatus == "In-Service")
                {
                    vehicles[index]->setStatus(newStatus);
                    cout << "Status updated." << endl;
                }
                else
                {
                    cout << "Invalid status." << endl;
                }
                break;
            }
            case 11:
                cout << "Exiting system." << endl;
                break;
            default:
                cout << "Invalid choice." << endl;
        }
    } while (choice != 11);
    for (int i = 0; i < count; i++)
        delete vehicles[i];

    return 0;
}