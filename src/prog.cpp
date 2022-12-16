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
        pthread_mutex_init(&mutex, NULL);
    }

    ~Department() {
        // Destroy department mutex.
        pthread_mutex_destroy(&mutex);
    }

    int getId() const {
        return id;
    }

    void serveCustomer(int customer_id) {
        // Lock cout mutex.
        pthread_mutex_lock(&cout_mutex);
        std::cout << "[Department #" << id << "] Serving customer #" << customer_id << std::endl;
        // Unlock cout mutex.
        pthread_mutex_unlock(&cout_mutex);

        // Lock department mutex.
        pthread_mutex_lock(&mutex);
        // Simulate customer service (1 second sleep).
        sleep(1);
        // Unlock department mutex.
        pthread_mutex_unlock(&mutex);

        // Lock cout mutex.
        pthread_mutex_lock(&cout_mutex);
        std::cout << "[Department #" << id << "] Finished serving customer #" << customer_id << std::endl;
        // Unlock cout mutex.
        pthread_mutex_unlock(&cout_mutex);
    }

private:
    int id;
    static int nextId;

    // Mutex for department queue.
    pthread_mutex_t mutex;
};

int Department::nextId = 1;

// Represents a customer in a store.
// A customer has a queue of departments they want to visit.
// If the department is busy, the customer waits joins the end of the queue.
class Customer {
public:
    int id;
    std::queue<Department> department_queue;

    Customer() {
        id = nextId++;
    }

    void enqueue(const Department& d) {
        department_queue.push(d);
    }

private:
    static int nextId;    
};

int Customer::nextId = 1;

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
        Department department = customer->department_queue.front();
        customer->department_queue.pop();

        // Lock cout mutex.
        pthread_mutex_lock(&cout_mutex);
        std::cout << "[Customer #" 
                  << customer->id 
                  << "] Going to department #" 
                  << department.getId() 
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
Customer parseCustomer(const std::string& input, const std::vector<Department>& departments) {
    Customer customer;
    for (int i = 0; i < input.size(); i++) {
        int department_id = input[i] - '0';
        customer.enqueue(departments[department_id - 1]);
    }
    return customer;
}

// Reads input from stdin and returns a vector of customers.
// First, the user is prompted to enter the number of customers.
// Then for each customer, the user enters the departments the customer wants to visit.
// The input format for departments is the same as the input format for parseCustomersFromArgs
std::vector<Customer> parseCustomersFromInput(const std::vector<Department>& departments) {
    std::vector<Customer> customers;

    int num_customers;
    std::cout << "Enter the number of customers: ";
    std::cin >> num_customers;

    for (int i = 0; i < num_customers; i++) {
        std::string input;
        std::cout << "Enter the departments for customer " << i + 1 << ": ";
        std::cin >> input;
        customers.push_back(parseCustomer(input, departments));
    }

    return customers;
}

int main(int argc, char** argv) {
    std::vector<Department> departments = {Department(), Department(), Department()};

    // Typing customers from console.
    std::vector<Customer> customers;
    if (argc <= 1) {
        std::cout << "(using console input for customers)" << std::endl;
        customers = parseCustomersFromInput(departments);
    } 
    // Parsing customers from command line args.
    else {
        std::cout << "(using command line args for customers)" << std::endl;
        for (int i = 1; i < argc; i++) {
            customers.push_back(parseCustomer(argv[i], departments));
        }
    }

    // Initialize cout mutex.
    pthread_mutex_init(&cout_mutex, NULL);

    std::vector<pthread_t> customer_threads(customers.size());
    // Create customer threads and start shopping.
    for (int i = 0; i < customers.size(); i++) {
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