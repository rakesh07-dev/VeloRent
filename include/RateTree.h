#include <iostream>
#include <string>
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

// ==========================================
// 1. SHARED VEHICLE STRUCTURE (Module 1-4)
// ==========================================
struct Vehicle {
    string vehicleID;
    string name;
    string type;
    double pricePerHour;
    bool isAvailable;
};

// ==========================================
// 2. HUFFMAN COMPRESSION & DECOMPRESSION ENGINE
// ==========================================
struct HuffmanNode {
    char ch;
    int freq;
    HuffmanNode *left, *right;

    HuffmanNode(char character, int frequency) {
        ch = character;
        freq = frequency;
        left = right = nullptr;
    }
    
    ~HuffmanNode() {
        delete left;
        delete right;
    }
};

struct CompareNodes {
    bool operator()(HuffmanNode* l, HuffmanNode* r) {
        return l->freq > r->freq;
    }
};

class HuffmanCompressor {
private:
    void generateCodes(HuffmanNode* root, string str, unordered_map<char, string>& huffmanCodes) {
        if (!root) return;

        if (!root->left && !root->right) {
            huffmanCodes[root->ch] = str;
        }

        generateCodes(root->left, str + "0", huffmanCodes);
        generateCodes(root->right, str + "1", huffmanCodes);
    }

public:
    string compressText(const string& text, unordered_map<char, string>& outCodes) {
        if (text.empty()) return "";

        unordered_map<char, int> freqMap;
        for (char c : text) {
            freqMap[c]++;
        }

        priority_queue<HuffmanNode*, vector<HuffmanNode*>, CompareNodes> minHeap;
        for (auto pair : freqMap) {
            minHeap.push(new HuffmanNode(pair.first, pair.second));
        }

        if (minHeap.size() == 1) {
            HuffmanNode* single = minHeap.top();
            minHeap.pop();
            minHeap.push(new HuffmanNode('\0', single->freq));
            minHeap.top()->left = single;
        }

        while (minHeap.size() > 1) {
            HuffmanNode* left = minHeap.top(); minHeap.pop();
            HuffmanNode* right = minHeap.top(); minHeap.pop();

            HuffmanNode* parent = new HuffmanNode('\0', left->freq + right->freq);
            parent->left = left;
            parent->right = right;
            minHeap.push(parent);
        }

        HuffmanNode* root = minHeap.top();
        generateCodes(root, "", outCodes);

        string encodedString = "";
        for (char c : text) {
            encodedString += outCodes[c];
        }

        delete root;
        return encodedString;
    }
};


// ==========================================
// 3. AVL TREE FOR PRICE INDEXING & SEARCH
// ==========================================
struct AVLNode {
    Vehicle vehicle;
    int height;
    AVLNode* left;
    AVLNode* right;

    AVLNode(const Vehicle& v) {
        vehicle = v;
        height = 1;
        left = right = nullptr;
    }

    ~AVLNode() {
        delete left;
        delete right;
    }
};

class PriceAVLTree {
private:
    AVLNode* root;

    int height(AVLNode* node) {
        return node ? node->height : 0;
    }

    int getBalanceFactor(AVLNode* node) {
        return node ? height(node->left) - height(node->right) : 0;
    }

    AVLNode* rightRotate(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        y->height = max(height(y->left), height(y->right)) + 1;
        x->height = max(height(x->left), height(x->right)) + 1;

        return x;
    }

    AVLNode* leftRotate(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        x->height = max(height(x->left), height(x->right)) + 1;
        y->height = max(height(y->left), height(y->right)) + 1;

        return y;
    }

    AVLNode* insert(AVLNode* node, const Vehicle& vehicle) {
        if (!node) return new AVLNode(vehicle);

        if (vehicle.pricePerHour < node->vehicle.pricePerHour)
            node->left = insert(node->left, vehicle);
        else
            node->right = insert(node->right, vehicle); // Allows duplicates on right branch

        node->height = 1 + max(height(node->left), height(node->right));
        int balance = getBalanceFactor(node);

        // Left Left Case
        if (balance > 1 && vehicle.pricePerHour < node->left->vehicle.pricePerHour)
            return rightRotate(node);

        // Right Right Case
        if (balance < -1 && vehicle.pricePerHour > node->right->vehicle.pricePerHour)
            return leftRotate(node);

        // Left Right Case
        if (balance > 1 && vehicle.pricePerHour > node->left->vehicle.pricePerHour) {
            node->left = leftRotate(node->left);
            return rightRotate(node);
        }

        // Right Left Case
        if (balance < -1 && vehicle.pricePerHour < node->right->vehicle.pricePerHour) {
            node->right = rightRotate(node->right);
            return leftRotate(node);
        }

        return node;
    }

    void inOrder(AVLNode* node, Vehicle results[], int& count, int maxResults) {
        if (!node || count >= maxResults) return;
        inOrder(node->left, results, count, maxResults);
        if (count < maxResults) {
            results[count++] = node->vehicle;
        }
        inOrder(node->right, results, count, maxResults);
    }

    void rangeSearchRecursive(AVLNode* node, double minP, double maxP, Vehicle results[], int& count, int maxResults) {
        if (!node || count >= maxResults) return;

        // Prune left subtree if current price is greater than minP
        if (node->vehicle.pricePerHour >= minP) {
            rangeSearchRecursive(node->left, minP, maxP, results, count, maxResults);
        }

        // Check current node
        if (node->vehicle.pricePerHour >= minP && node->vehicle.pricePerHour <= maxP) {
            if (count < maxResults) {
                results[count++] = node->vehicle;
            }
        }

        // Prune right subtree if current price is less than maxP
        if (node->vehicle.pricePerHour <= maxP) {
            rangeSearchRecursive(node->right, minP, maxP, results, count, maxResults);
        }
    }

public:
    PriceAVLTree() : root(nullptr) {}

    void buildTree(Vehicle* inventory, int size) {
        root = nullptr;
        for (int i = 0; i < size; ++i) {
            root = insert(root, inventory[i]);
        }
    }

    int getAllSorted(Vehicle results[], int maxResults) {
        int count = 0;
        inOrder(root, results, count, maxResults);
        return count;
    }

    int searchRange(double minP, double maxP, Vehicle results[], int maxResults) {
        int count = 0;
        rangeSearchRecursive(root, minP, maxP, results, count, maxResults);
        return count;
    }
};


// ==========================================
// 4. ARCHIVED RECEIPT RECORD STRUCTURE
// ==========================================
struct ArchivedReceiptRecord {
    string compressedBits;
    unordered_map<string, char> reverseCodes;
    string originalPreview;
};


// ==========================================
// 5. MODULE 3 CONTROLLER & MENU DRIVER
// ==========================================
class Module3Controller {
private:
    Vehicle* sharedInventory;
    int& inventorySize;
    PriceAVLTree avlTree;
    HuffmanCompressor compressor;
    
    ArchivedReceiptRecord archivedReceipts[50];
    int archiveCount;

public:
    Module3Controller(Vehicle* inventory, int& size) 
        : sharedInventory(inventory), inventorySize(size), archiveCount(0) {
        // Build AVL tree initially from shared inventory
        avlTree.buildTree(sharedInventory, inventorySize);
    }

    void runModuleMenu() {
        int choice = 0;
        do {
            cout << "\n==================================================\n";
            cout << " VELORENT - MODULE 3: AVL SEARCH & COMPRESSED LOGS\n";
            cout << "==================================================\n";
            cout << "1. View AVL-Sorted Vehicle Inventory\n";
            cout << "2. O(log N) Price Range Search (AVL Tree)\n";
            cout << "3. Compress & Archive a Rental Receipt (Huffman)\n";
            cout << "4. Decompose & View Archived Receipts\n";
            cout << "5. Return to Main Program\n";
            cout << "==================================================\n";
            cout << "Enter your choice (1-5): ";
            
            if (!(cin >> choice)) {
                cout << "[Error]: Invalid input. Please enter a number.\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            switch (choice) {
                case 1: {
                    // Rebuild tree in case availability status changed
                    avlTree.buildTree(sharedInventory, inventorySize);
                    Vehicle sortedResults[50];
                    int totalFound = avlTree.getAllSorted(sortedResults, 50);

                    cout << "\n-----------------------------------------------------------------------\n";
                    cout << left << setw(14) << "Vehicle ID" 
                              << setw(26) << "Vehicle Name" 
                              << setw(12) << "Type" 
                              << setw(12) << "Price/Hr" 
                              << "Status\n";
                    cout << "-----------------------------------------------------------------------\n";
                    for (int i = 0; i < totalFound; ++i) {
                        cout << left << setw(14) << sortedResults[i].vehicleID 
                                  << setw(26) << sortedResults[i].name 
                                  << setw(12) << sortedResults[i].type 
                                  << "Rs. " << setw(8) << sortedResults[i].pricePerHour 
                                  << (sortedResults[i].isAvailable ? "Available" : "Rented") << "\n";
                    }
                    cout << "-----------------------------------------------------------------------\n";
                    break;
                }
                case 2: {
                    double minP, maxP;
                    cout << "\n--- AVL PRICE RANGE FILTER ---\n";
                    cout << "Enter minimum price per hour (Rs.): ";
                    cin >> minP;
                    cout << "Enter maximum price per hour (Rs.): ";
                    cin >> maxP;

                    if (minP > maxP) {
                        cout << "[Error]: Minimum price cannot be greater than maximum price!\n";
                        break;
                    }

                    avlTree.buildTree(sharedInventory, inventorySize);
                    Vehicle results[50];
                    int foundCount = avlTree.searchRange(minP, maxP, results, 50);

                    cout << "\nFound " << foundCount << " vehicle(s) in range Rs. " << minP << " - Rs. " << maxP << ":\n";
                    
                    if (foundCount > 0) {
                        cout << "-------------------------------------------------------------------\n";
                        cout << left << setw(14) << "Vehicle ID" << setw(26) << "Name" << setw(12) << "Type" << "Price/Hr\n";
                        cout << "-------------------------------------------------------------------\n";
                        for (int i = 0; i < foundCount; ++i) {
                            cout << left << setw(14) << results[i].vehicleID 
                                      << setw(26) << results[i].name 
                                      << setw(12) << results[i].type
                                      << "Rs. " << results[i].pricePerHour << "\n";
                        }
                        cout << "-------------------------------------------------------------------\n";
                    }
                    break;
                }
                case 3: {
                    string vID, userName;
                    int hours;
                    cout << "\n--- HUFFMAN RECEIPT COMPRESSION ---\n";
                    cout << "Enter Vehicle ID rented (e.g., UK07-PA-1011): ";
                    cin >> vID;
                    cout << "Enter Customer Name: ";
                    cin >> ws; // clear leading whitespace/newlines
                    getline(cin, userName); // supports spaces in names
                    cout << "Enter Rental Duration (Hours): ";
                    cin >> hours;

                    double rate = 150.0;
                    bool vehicleFound = false;
                    for (int i = 0; i < inventorySize; ++i) {
                        if (sharedInventory[i].vehicleID == vID) {
                            rate = sharedInventory[i].pricePerHour;
                            sharedInventory[i].isAvailable = false; // Mark as rented
                            vehicleFound = true;
                            break;
                        }
                    }

                    if (!vehicleFound) {
                        cout << "[Warning]: Vehicle ID not found, using default rate.\n";
                    }

                    double total = rate * hours;

                    stringstream ss;
                    ss << "RENTAL|ID:" << vID << "|Cust:" << userName << "|Hrs:" << hours << "|Tot:Rs" << total;
                    string rawReceipt = ss.str();

                    unordered_map<char, string> codes;
                    string compressedBitStream = compressor.compressText(rawReceipt, codes);

                    // Build reverse code map for decompression
                    unordered_map<string, char> reverseCodes;
                    for (auto pair : codes) {
                        reverseCodes[pair.second] = pair.first;
                    }

                    size_t originalSize = rawReceipt.size() * 8;
                    size_t compressedSize = compressedBitStream.size();
                    double savings = originalSize > 0 ? (1.0 - (double)compressedSize / originalSize) * 100.0 : 0.0;

                    cout << "\n[Original Receipt]     : " << rawReceipt << "\n";
                    cout << "Original Size          : " << originalSize << " bits\n";
                    cout << "Compressed Bit-Stream  : " << compressedBitStream << "\n";
                    cout << "Compressed Size        : " << compressedSize << " bits\n";
                    cout << "Space Saved            : " << fixed << setprecision(1) << savings << "%\n";

                    if (archiveCount < 50) {
                        archivedReceipts[archiveCount].compressedBits = compressedBitStream;
                        archivedReceipts[archiveCount].reverseCodes = reverseCodes;
                        archivedReceipts[archiveCount].originalPreview = rawReceipt;
                        archiveCount++;
                        cout << "[Success]: Receipt compressed and saved to secure AVL-backed archive!\n";
                    } else {
                        cout << "[Error]: Archive storage is full!\n";
                    }
                    break;
                }
                case 4: {
                    cout << "\n--- ARCHIVED LOGS DECOMPRESSION (HUFFMAN) ---\n";
                    if (archiveCount == 0) {
                        cout << "No archived receipts found yet. Compress a receipt first (Option 3).\n";
                        break;
                    }

                    for (int i = 0; i < archiveCount; ++i) {
                        cout << "\n----------------------------------------------------\n";
                        cout << "[Archive Record Index #" << (i + 1) << "]\n";
                        cout << "Compressed Bits  : " << archivedReceipts[i].compressedBits << "\n";

                        // Real Huffman Decompression logic
                        string decodedText = "";
                        string currentBits = "";
                        for (char bit : archivedReceipts[i].compressedBits) {
                            currentBits += bit;
                            if (archivedReceipts[i].reverseCodes.find(currentBits) != archivedReceipts[i].reverseCodes.end()) {
                                decodedText += archivedReceipts[i].reverseCodes[currentBits];
                                currentBits = ""; // reset for next character
                            }
                        }

                        cout << "Decompressed Text: " << decodedText << "\n";
                        cout << "----------------------------------------------------\n";
                    }
                    break;
                }
                case 5:
                    cout << "Exiting Module 3...\n";
                    break;
                default:
                    cout << "[Error]: Invalid choice. Please select between 1 and 5.\n";
            }
        } while (choice != 5);
    }
};


// ==========================================
// 6. MAIN DRIVER (For testing Module 3 standalone)
// ==========================================
int main() {
    const int MAX_CAPACITY = 50;
    Vehicle masterInventory[MAX_CAPACITY] = {
        {"UK07-PA-1011", "Royal Enfield Hunter 350", "Bike", 150.0, true},
        {"UK07-CH-2022", "KTM Duke 200", "Bike", 200.0, true},
        {"UP32-TN-3033", "Mahindra Thar", "Car", 600.0, true},
        {"DL01-SC-4044", "Activa 6G", "Scooter", 80.0, false},
        {"UK07-HM-5055", "Royal Enfield Himalayan", "Bike", 250.0, true},
        {"AS01-EV-6066", "Ola S1 Pro", "Scooter", 100.0, true},
        {"HR26-TF-7077", "Toyota Fortuner", "Car", 1200.0, true},
        {"UK07-MT-8088", "Yamaha MT-15", "Bike", 180.0, true},
        {"UP32-NS-9099", "Bajaj Pulsar NS200", "Bike", 130.0, true},
        {"AS01-AP-1111", "TVS Apache RTR 160", "Bike", 120.0, true},
        {"HR26-CB-2222", "Honda CB350", "Bike", 220.0, false},
        {"DL01-SA-3333", "Suzuki Access 125", "Scooter", 90.0, true},
        {"MH12-NX-4444", "Tata Nexon EV", "Car", 500.0, true}
    };
    
    int inventorySize = 13;

    Module3Controller module3(masterInventory, inventorySize);
    module3.runModuleMenu();

    return 0;
}