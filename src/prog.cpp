#include <cstddef>
#include <iostream>
#include <queue>
#include <string>
#include <unistd.h>
#include <pthread.h>

// Global mutex for cout.
pthread_mutex_t cout_mutex;

// Represents a department in a store.
class Department {
public:
    Department() {
        id = nextId++;
        // Initialize department mutex.
        pthread_mutex_init(&department_mutex, NULL);
    }

    ~Department() {
        // Destroy department mutex in destructor.
        pthread_mutex_destroy(&department_mutex);
    }

    int getId() const {
        return id;
    }

    void serveCustomer(int customer_id) {
        // Lock department mutex.
        // The next customer can only be served after the current customer is done.
        pthread_mutex_lock(&department_mutex);

        // Lock cout mutex.
        pthread_mutex_lock(&cout_mutex);
        std::cout << "[Department #" << id << "] Serving customer #" << customer_id << std::endl;
        // Unlock cout mutex.
        pthread_mutex_unlock(&cout_mutex);

        // Simulate customer service (1 second sleep).
        sleep(1);

        // Lock cout mutex.
        pthread_mutex_lock(&cout_mutex);
        std::cout << "[Department #" << id << "] Finished serving customer #" << customer_id << std::endl;
        // Unlock cout mutex.
        pthread_mutex_unlock(&cout_mutex);

        // Unlock department mutex -> next customer can be served.
        pthread_mutex_unlock(&department_mutex);
    }

private:
    // Department's id.
    int id;
    static int nextId;

    // Mutex for department queue, made to be exclusive to each department
    // so that every department has its own queue.
    pthread_mutex_t department_mutex;
};

int Department::nextId = 1;

// Represents a customer in a store.
// A customer has a queue of departments they want to visit.
// If the department is busy, the customer waits joins the end of the queue.
class Customer {
public:
    // Customer's id.
    int id;

    // Queue of departments the customer wants to visit.
    std::queue<int> department_queue;

    Customer() {
        id = nextId++;
    }

    void enqueue(int department_id) {
        department_queue.push(department_id);
    }

private:
    static int nextId;    
};

int Customer::nextId = 1;

// All 3 departments in the store.
static Department department1, department2, department3; 

Department& departmentFromId(int id) {
    switch (id) {
        case 1: return department1;
        case 2: return department2;
        case 3: return department3;
        default: throw "Invalid department id";
    }
}

void* startShopping(void* args) {
    Customer* customer = (Customer*) args;

    // Lock cout mutex.
    pthread_mutex_lock(&cout_mutex);
    std::cout << "[Customer #" 
              << customer->id 
              << "] Started shopping, want to visit " 
              << customer->department_queue.size() 
              << " departments" << std::endl;
    // Unlock cout mutex.
    pthread_mutex_unlock(&cout_mutex);

    while (!customer->department_queue.empty()) {
        auto department_id = customer->department_queue.front();
        auto& department = departmentFromId(department_id);
        customer->department_queue.pop();

        // Lock cout mutex.
        pthread_mutex_lock(&cout_mutex);
        std::cout << "[Customer #" 
                  << customer->id 
                  << "] Going to department #" 
                  << department_id 
                  << std::endl;
        // Unlock cout mutex.
        pthread_mutex_unlock(&cout_mutex);

        department.serveCustomer(customer->id);

        // Lock cout mutex.
        pthread_mutex_lock(&cout_mutex);
        std::cout << "[Customer #" 
                  << customer->id 
                  << "] Returning from department #" 
                  << department.getId() 
                  << std::endl;
        // Unlock cout mutex.
        pthread_mutex_unlock(&cout_mutex);
    }

    // Lock cout mutex.
    pthread_mutex_lock(&cout_mutex);
    std::cout << "[Customer #" << customer->id << "] Finished shopping" << std::endl;
    // Unlock cout mutex.
    pthread_mutex_unlock(&cout_mutex);
    
    return nullptr;
}

// Parse customer in the input string and return a customer
// The input string looks like this: <department_id1><department_id2>...<department_idn>
// For example: 123 means the customer wants to visit departments 1, 2, and 3
Customer parseCustomer(const std::string& input) {
    if (input.empty()) {
        std::cout << "[ERROR] Department queue cannot be empty" << std::endl;
        exit(1);
    }

    Customer customer;
    for (int i = 0; i < input.size(); i++) {
        int department_id = input[i] - '0';
        if (department_id < 1 || department_id > 3) {
            std::cout << "[ERROR] Invalid department id (expected a number between 1 and 3): " << (char)(department_id + '0') << std::endl;
            exit(1);
        }
        customer.enqueue(department_id);
    }

    return customer;
}

// Reads input from stdin and returns a vector of customers.
// First, the user is prompted to enter the number of customers.
// Then for each customer, the user enters the departments the customer wants to visit.
// The input format for departments is the same as the input format for parseCustomersFromArgs
std::vector<Customer> parseCustomersFromInput() {
    std::vector<Customer> customers;

    int num_customers;
    std::cout << "Enter the number of customers: ";
    std::cin >> num_customers;

    if (std::cin.fail()) {
        std::cout << "[ERROR] Expected integer" << std::endl;
        exit(1);
    }

    for (int i = 0; i < num_customers; i++) {
        std::string input;
        std::cout << "Enter the departments for customer " << i + 1 << ": ";
        std::cin >> input;
        customers.push_back(parseCustomer(input));
    }

    return customers;
}

int main(int argc, char** argv) {
    // Typing customers from console.
    std::vector<Customer> customers;
    if (argc <= 1) {
        std::cout << "(using console input for customers)" << std::endl;
        customers = parseCustomersFromInput();
    } 
    // Parsing customers from command line args.
    else {
        std::cout << "(using command line args for customers)" << std::endl;
        for (int i = 1; i < argc; i++) {
            customers.push_back(parseCustomer(argv[i]));
        }
    }

    // Initialize cout mutex.
    // Needed to print information about customers and departments in a thread-safe manner.
    pthread_mutex_init(&cout_mutex, NULL);

    std::vector<pthread_t> customer_threads(customers.size());
    // Create customer threads and start shopping.
    for (int i = 0; i < customers.size(); i++) {
        // &customer[i]: passing a customer to startShopping function.
        pthread_create(&customer_threads[i], NULL, startShopping, &customers[i]);
    }

    // Wait for all customers to finish shopping.
    for (int i = 0; i < customers.size(); i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Destroy cout mutex.
    pthread_mutex_destroy(&cout_mutex);

    return 0;
}