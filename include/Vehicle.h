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