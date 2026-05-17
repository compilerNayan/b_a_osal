#ifndef REPOSITORY_TESTS_H
#define REPOSITORY_TESTS_H

// Conditionally include headers based on platform
#ifdef ARDUINO
    #include <Arduino.h>
    #include <string>
    #include <vector>
#else
    #include <iostream>
    #include <cassert>
    #include <string>
    #include <vector>
#endif

#include <StandardDefines.h>
#include "ProductRepository.h"
#include "OrderRepository.h"
#include "CustomerRepository.h"
#include "UserAccountRepository.h"
#include "Product.h"
#include "Order.h"
#include "Customer.h"
#include "UserAccount.h"
#include "../tests/TestUtils.h"

/* @Autowired */
ProductRepositoryPtr productRepository = Implementation<ProductRepository>::type::GetInstance();

/* @Autowired */
OrderRepositoryPtr orderRepository = Implementation<OrderRepository>::type::GetInstance();

/* @Autowired */
CustomerRepositoryPtr customerRepository = Implementation<CustomerRepository>::type::GetInstance();

/* @Autowired */
UserAccountRepositoryPtr userAccountRepository = Implementation<UserAccountRepository>::type::GetInstance();

// Test ProductRepository
void TestProductRepository() {
    std_println("\n=== Testing ProductRepository ===");
    
    // Test 1: Save a product
    Product product1;
    product1.id = 1;
    product1.name = "Laptop";
    product1.price = 999.99;
    product1.category = "Electronics";
    
    Product savedProduct = productRepository->Save(product1);
    PrintTestResult("Save Product", savedProduct.id.has_value() && savedProduct.id.value() == 1);
    
    // Test 2: FindById
    optional<Product> foundProduct = productRepository->FindById(1);
    PrintTestResult("FindById - Product exists", foundProduct.has_value() && foundProduct.value().id.value() == 1);
    
    // Test 3: FindByName
    optional<Product> productByName = productRepository->FindByName("Laptop");
    PrintTestResult("FindByName - Found by name", productByName.has_value() && productByName.value().name.value() == "Laptop");
    
    // Test 4: FindByName - Not found
    optional<Product> productNotFound = productRepository->FindByName("NonExistent");
    PrintTestResult("FindByName - Not found", !productNotFound.has_value());
    
    // Test 5: Save multiple products with same category
    Product product2;
    product2.id = 2;
    product2.name = "Smartphone";
    product2.price = 699.99;
    product2.category = "Electronics";
    productRepository->Save(product2);
    
    Product product3;
    product3.id = 3;
    product3.name = "Tablet";
    product3.price = 499.99;
    product3.category = "Electronics";
    productRepository->Save(product3);
    
    Product product4;
    product4.id = 4;
    product4.name = "Desk Chair";
    product4.price = 199.99;
    product4.category = "Furniture";
    productRepository->Save(product4);
    
    // Test 6: FindByCategory - should return multiple products
    StdVector<Product> electronics = productRepository->FindByCategory("Electronics");
    PrintTestResult("FindByCategory - Multiple results", electronics.size() == 3);
    
    // Test 7: FindByCategory - Single result
    StdVector<Product> furniture = productRepository->FindByCategory("Furniture");
    PrintTestResult("FindByCategory - Single result", furniture.size() == 1 && furniture[0].name.value() == "Desk Chair");
    
    // Test 8: FindByCategory - No results
    StdVector<Product> empty = productRepository->FindByCategory("NonExistent");
    PrintTestResult("FindByCategory - No results", empty.size() == 0);
    
    // Test 9: FindAll
    StdVector<Product> allProducts = productRepository->FindAll();
    PrintTestResult("FindAll - All products", allProducts.size() >= 4);
    
    // Test 10: Update product
    Product updatedProduct = foundProduct.value();
    updatedProduct.price = 899.99;
    Product savedUpdated = productRepository->Update(updatedProduct);
    optional<Product> verifyUpdate = productRepository->FindById(1);
    PrintTestResult("Update Product", verifyUpdate.has_value() && verifyUpdate.value().price.value() == 899.99);
    
    // Test 11: ExistsById
    bool exists = productRepository->ExistsById(1);
    PrintTestResult("ExistsById - Product exists", exists);
    
    bool notExists = productRepository->ExistsById(999);
    PrintTestResult("ExistsById - Product not exists", !notExists);
    
    // Test 12: DeleteById
    productRepository->DeleteById(4);
    bool deletedExists = productRepository->ExistsById(4);
    PrintTestResult("DeleteById - Product deleted", !deletedExists);
}

// Test OrderRepository
void TestOrderRepository() {
    std_println("\n=== Testing OrderRepository ===");
    
    // Test 1: Save an order
    Order order1;
    order1.id = 1;
    order1.orderNumber = "ORD-001";
    order1.customerId = 100;
    order1.totalAmount = 1299.98;
    
    Order savedOrder = orderRepository->Save(order1);
    PrintTestResult("Save Order", savedOrder.id.has_value() && savedOrder.id.value() == 1);
    
    // Test 2: FindByOrderNumber
    optional<Order> orderByNumber = orderRepository->FindByOrderNumber("ORD-001");
    PrintTestResult("FindByOrderNumber - Found", orderByNumber.has_value() && orderByNumber.value().orderNumber.value() == "ORD-001");
    
    // Test 3: FindByOrderNumber - Not found
    optional<Order> orderNotFound = orderRepository->FindByOrderNumber("ORD-999");
    PrintTestResult("FindByOrderNumber - Not found", !orderNotFound.has_value());
    
    // Test 4: Save multiple orders for same customer
    Order order2;
    order2.id = 2;
    order2.orderNumber = "ORD-002";
    order2.customerId = 100;
    order2.totalAmount = 599.99;
    orderRepository->Save(order2);
    
    Order order3;
    order3.id = 3;
    order3.orderNumber = "ORD-003";
    order3.customerId = 100;
    order3.totalAmount = 199.99;
    orderRepository->Save(order3);
    
    Order order4;
    order4.id = 4;
    order4.orderNumber = "ORD-004";
    order4.customerId = 200;
    order4.totalAmount = 899.99;
    orderRepository->Save(order4);
    
    // Test 5: FindByCustomerId - Multiple orders
    StdVector<Order> customerOrders = orderRepository->FindByCustomerId(100);
    PrintTestResult("FindByCustomerId - Multiple orders", customerOrders.size() == 3);
    
    // Test 6: FindByCustomerId - Single order
    StdVector<Order> customer200Orders = orderRepository->FindByCustomerId(200);
    PrintTestResult("FindByCustomerId - Single order", customer200Orders.size() == 1);
    
    // Test 7: FindByCustomerId - No orders
    StdVector<Order> noOrders = orderRepository->FindByCustomerId(999);
    PrintTestResult("FindByCustomerId - No orders", noOrders.size() == 0);
    
    // Test 8: FindAll
    StdVector<Order> allOrders = orderRepository->FindAll();
    PrintTestResult("FindAll - All orders", allOrders.size() >= 4);
}

// Test CustomerRepository
void TestCustomerRepository() {
    std_println("\n=== Testing CustomerRepository ===");
    
    // Test 1: Save a customer
    Customer customer1;
    customer1.id = 1;
    customer1.email = "john.doe@example.com";
    customer1.firstName = "John";
    customer1.lastName = "Doe";
    customer1.phone = "555-0101";
    
    Customer savedCustomer = customerRepository->Save(customer1);
    PrintTestResult("Save Customer", savedCustomer.id.has_value() && savedCustomer.id.value() == 1);
    
    // Test 2: FindByEmail
    optional<Customer> customerByEmail = customerRepository->FindByEmail("john.doe@example.com");
    PrintTestResult("FindByEmail - Found", customerByEmail.has_value() && customerByEmail.value().email.value() == "john.doe@example.com");
    
    // Test 3: FindByEmail - Not found
    optional<Customer> emailNotFound = customerRepository->FindByEmail("nonexistent@example.com");
    PrintTestResult("FindByEmail - Not found", !emailNotFound.has_value());
    
    // Test 4: Save multiple customers with same last name
    Customer customer2;
    customer2.id = 2;
    customer2.email = "jane.doe@example.com";
    customer2.firstName = "Jane";
    customer2.lastName = "Doe";
    customer2.phone = "555-0102";
    customerRepository->Save(customer2);
    
    Customer customer3;
    customer3.id = 3;
    customer3.email = "bob.doe@example.com";
    customer3.firstName = "Bob";
    customer3.lastName = "Doe";
    customer3.phone = "555-0103";
    customerRepository->Save(customer3);
    
    Customer customer4;
    customer4.id = 4;
    customer4.email = "alice.smith@example.com";
    customer4.firstName = "Alice";
    customer4.lastName = "Smith";
    customer4.phone = "555-0201";
    customerRepository->Save(customer4);
    
    // Test 5: FindByLastName - Multiple customers
    StdVector<Customer> does = customerRepository->FindByLastName("Doe");
    PrintTestResult("FindByLastName - Multiple customers", does.size() == 3);
    
    // Test 6: FindByLastName - Single customer
    StdVector<Customer> smiths = customerRepository->FindByLastName("Smith");
    PrintTestResult("FindByLastName - Single customer", smiths.size() == 1 && smiths[0].firstName.value() == "Alice");
    
    // Test 7: FindByLastName - No customers
    StdVector<Customer> noCustomers = customerRepository->FindByLastName("NonExistent");
    PrintTestResult("FindByLastName - No customers", noCustomers.size() == 0);
    
    // Test 8: FindByPhone
    optional<Customer> customerByPhone = customerRepository->FindByPhone("555-0101");
    PrintTestResult("FindByPhone - Found", customerByPhone.has_value() && customerByPhone.value().id.value() == 1);
    
    // Test 9: FindByPhone - Not found
    optional<Customer> phoneNotFound = customerRepository->FindByPhone("555-9999");
    PrintTestResult("FindByPhone - Not found", !phoneNotFound.has_value());
    
    // Test 10: FindAll
    StdVector<Customer> allCustomers = customerRepository->FindAll();
    PrintTestResult("FindAll - All customers", allCustomers.size() >= 4);
    
    // Test 11: Update customer
    Customer updatedCustomer = customerByEmail.value();
    updatedCustomer.phone = "555-9999";
    Customer savedUpdated = customerRepository->Update(updatedCustomer);
    optional<Customer> verifyUpdate = customerRepository->FindById(1);
    PrintTestResult("Update Customer", verifyUpdate.has_value() && verifyUpdate.value().phone.value() == "555-9999");
    
    // Test 12: Delete customer
    customerRepository->DeleteById(4);
    bool deletedExists = customerRepository->ExistsById(4);
    PrintTestResult("Delete Customer", !deletedExists);
}

// Test UserAccountRepository
void TestUserAccountRepository() {
    std_println("\n=== Testing UserAccountRepository ===");
    
    // Test 1: Save a user account
    UserAccount account1;
    account1.username = "john_doe";
    account1.password = "password123";
    account1.name = "John Doe";
    
    UserAccount savedAccount = userAccountRepository->Save(account1);
    PrintTestResult("Save UserAccount", savedAccount.username.has_value() && savedAccount.username.value() == "john_doe");
    
    // Test 2: FindById (by username)
    optional<UserAccount> foundAccount = userAccountRepository->FindById("john_doe");
    PrintTestResult("FindById - Account exists", foundAccount.has_value() && foundAccount.value().username.value() == "john_doe");
    
    // Test 3: FindByName
    optional<UserAccount> accountByName = userAccountRepository->FindByName("John Doe");
    PrintTestResult("FindByName - Found by name", accountByName.has_value() && accountByName.value().name.value() == "John Doe");
    
    // Test 4: FindByName - Not found
    optional<UserAccount> nameNotFound = userAccountRepository->FindByName("NonExistent");
    PrintTestResult("FindByName - Not found", !nameNotFound.has_value());
    
    // Test 5: Save multiple accounts with different names
    UserAccount account2;
    account2.username = "jane_smith";
    account2.password = "password456";
    account2.name = "Jane Smith";
    userAccountRepository->Save(account2);
    
    UserAccount account3;
    account3.username = "bob_wilson";
    account3.password = "password789";
    account3.name = "Bob Wilson";
    userAccountRepository->Save(account3);
    
    // Test 6: FindByName - Different account
    optional<UserAccount> janeAccount = userAccountRepository->FindByName("Jane Smith");
    PrintTestResult("FindByName - Different account", janeAccount.has_value() && janeAccount.value().username.value() == "jane_smith");
    
    // Test 7: FindAll
    StdVector<UserAccount> allAccounts = userAccountRepository->FindAll();
    PrintTestResult("FindAll - All accounts", allAccounts.size() >= 3);
    
    // Test 8: Update account
    UserAccount updatedAccount = foundAccount.value();
    updatedAccount.password = "newpassword123";
    UserAccount savedUpdated = userAccountRepository->Update(updatedAccount);
    optional<UserAccount> verifyUpdate = userAccountRepository->FindById("john_doe");
    PrintTestResult("Update UserAccount", verifyUpdate.has_value() && verifyUpdate.value().password.value() == "newpassword123");
    
    // Test 9: ExistsById
    bool exists = userAccountRepository->ExistsById("john_doe");
    PrintTestResult("ExistsById - Account exists", exists);
    
    bool notExists = userAccountRepository->ExistsById("nonexistent");
    PrintTestResult("ExistsById - Account not exists", !notExists);
    
    // Test 10: DeleteById
    userAccountRepository->DeleteById("bob_wilson");
    bool deletedExists = userAccountRepository->ExistsById("bob_wilson");
    PrintTestResult("DeleteById - Account deleted", !deletedExists);
    
    // Test 11: FindByName after deletion
    optional<UserAccount> deletedAccount = userAccountRepository->FindByName("Bob Wilson");
    PrintTestResult("FindByName - Deleted account not found", !deletedAccount.has_value());
    
    // Test 12: Save account with same name (different username)
    UserAccount account4;
    account4.username = "john_doe2";
    account4.password = "password999";
    account4.name = "John Doe";  // Same name as account1
    userAccountRepository->Save(account4);
    
    // FindByName should return the first match (account1)
    optional<UserAccount> sameNameAccount = userAccountRepository->FindByName("John Doe");
    PrintTestResult("FindByName - Same name (returns first match)", sameNameAccount.has_value());
}

// Run all tests
void RunAllRepositoryTests() {
    std_println("\n========================================");
    std_println("Starting Repository Tests");
    std_println("========================================");
    
    TestProductRepository();
    TestOrderRepository();
    TestCustomerRepository();
    TestUserAccountRepository();
    
    std_println("\n========================================");
    std_println("Repository Tests Completed");
    std_println("========================================\n");
}

#endif // REPOSITORY_TESTS_H

