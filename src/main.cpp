#include "../include/base_types.h"
#include "../include/shared_class.h"
#include "../lib/shared_library.h"
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <vector>
#include <iomanip>
#include <typeinfo>

// Forward declarations for host-side functions (defined in host_implementation.cpp)
namespace WeakSymbolExample {
    std::unique_ptr<AbstractWorker> createHostSharedWorker(int value);
    std::unique_ptr<IBaseObject> createHostBaseObject(int value);
    std::unique_ptr<AbstractWorker> createHostTemplatedWorkerInt(int value);
    std::unique_ptr<AbstractWorker> createHostTemplatedWorkerString(const std::string& value);
    
    bool testHostDynamicCast(IBaseObject* obj);
    std::string getHostTypeInfo(IBaseObject* obj);
    void printHostObjectInfo(IBaseObject* obj);
    void demonstrateHostWeakSymbolUnification();
    void verifyTypeUnificationAcrossBoundary();
}

using namespace WeakSymbolExample;

// Test basic object creation and method calls
TEST(WeakSymbolLinking, BasicFunctionality) {
    // Create objects from both host and DLL
    auto hostWorker = createHostSharedWorker(10);
    auto dllWorker = createDLLSharedWorker(20);
    
    // Verify objects were created successfully
    ASSERT_NE(hostWorker, nullptr);
    ASSERT_NE(dllWorker, nullptr);
    
    // Test basic properties
    EXPECT_EQ(hostWorker->getValue(), 10);
    EXPECT_EQ(dllWorker->getValue(), 20);
    EXPECT_EQ(hostWorker->getTypeName(), "SharedWorker");
    EXPECT_EQ(dllWorker->getTypeName(), "SharedWorker");
    
    // Test readiness
    EXPECT_TRUE(hostWorker->isReady());
    EXPECT_TRUE(dllWorker->isReady());
    
    // Test that we can call methods without crashing
    EXPECT_NO_THROW(hostWorker->performAction());
    EXPECT_NO_THROW(hostWorker->doWork());
    EXPECT_NO_THROW(dllWorker->performAction());
    EXPECT_NO_THROW(dllWorker->doWork());
}

// Test RTTI functionality across boundaries
TEST(WeakSymbolLinking, RTTIFunctionality) {
    // Create various types of objects
    auto hostSharedWorker = createHostSharedWorker(100);
    auto dllSharedWorker = createDLLSharedWorker(200);
    auto hostTemplated = createHostTemplatedWorkerInt(300);
    auto dllTemplated = createDLLTemplatedWorkerInt(400);
    
    // Verify all objects were created
    ASSERT_NE(hostSharedWorker, nullptr);
    ASSERT_NE(dllSharedWorker, nullptr);
    ASSERT_NE(hostTemplated, nullptr);
    ASSERT_NE(dllTemplated, nullptr);
    
    // Test SharedWorker objects
    EXPECT_EQ(hostSharedWorker->getTypeName(), "SharedWorker");
    EXPECT_EQ(dllSharedWorker->getTypeName(), "SharedWorker");
    
    // Test TemplatedWorker objects
    EXPECT_EQ(hostTemplated->getTypeName(), "TemplatedWorker<i>");
    EXPECT_EQ(dllTemplated->getTypeName(), "TemplatedWorker<i>");
    
    // Test RTTI consistency - type names should match even if hash codes differ
    const auto& hostRef = *hostSharedWorker;
    const auto& dllRef = *dllSharedWorker;
    EXPECT_STREQ(typeid(hostRef).name(), typeid(dllRef).name());
    
    // Test dynamic_cast functionality across boundaries
    EXPECT_TRUE(testHostDynamicCast(hostSharedWorker.get()));
    EXPECT_TRUE(testHostDynamicCast(dllSharedWorker.get()));
    EXPECT_TRUE(testDynamicCast(hostSharedWorker.get()));
    EXPECT_TRUE(testDynamicCast(dllSharedWorker.get()));
    
    // Test virtual method calls don't crash
    EXPECT_NO_THROW(hostSharedWorker->performAction());
    EXPECT_NO_THROW(dllSharedWorker->performAction());
    EXPECT_NO_THROW(hostTemplated->performAction());
    EXPECT_NO_THROW(dllTemplated->performAction());
}

// Test that types created in host and DLL work correctly together
TEST(WeakSymbolLinking, TypeUnification) {
    // Create SharedWorker instances from both sides
    auto hostWorker = createHostSharedWorker(500);
    auto dllWorker = createDLLSharedWorker(600);
    
    ASSERT_NE(hostWorker, nullptr);
    ASSERT_NE(dllWorker, nullptr);
    
    // Store references to avoid typeid side effect warnings
    const auto& hostWorker_ref = *hostWorker;
    const auto& dllWorker_ref = *dllWorker;
    
    // Test that type names are consistent (even if hash codes differ on macOS)
    EXPECT_STREQ(typeid(hostWorker_ref).name(), typeid(dllWorker_ref).name());
    
    // Test cross-boundary casting - this is the key functionality
    IBaseObject* hostBase = hostWorker.get();
    IBaseObject* dllBase = dllWorker.get();
    
    // Cast DLL object using host-compiled cast
    SharedWorker* castedDLL = dynamic_cast<SharedWorker*>(dllBase);
    SharedWorker* castedHost = dynamic_cast<SharedWorker*>(hostBase);
    
    // These should succeed for weak symbol linking to be working
    EXPECT_NE(castedDLL, nullptr) << "Failed to cast DLL object to SharedWorker";
    EXPECT_NE(castedHost, nullptr) << "Failed to cast HOST object to SharedWorker";
    
    // Test that we can call methods on cross-cast objects
    if (castedDLL) {
        EXPECT_NO_THROW(castedDLL->performAction());
        EXPECT_EQ(castedDLL->getValue(), 600);
    }
    
    if (castedHost) {
        EXPECT_NO_THROW(castedHost->performAction());
        EXPECT_EQ(castedHost->getValue(), 500);
    }
}

// Test template instantiation unification
TEST(WeakSymbolLinking, TemplateUnification) {
    // Create templated instances from both sides
    auto hostTemplatedInt = createHostTemplatedWorkerInt(1000);
    auto dllTemplatedInt = createDLLTemplatedWorkerInt(2000);
    auto hostTemplatedString = createHostTemplatedWorkerString("HOST_STRING");
    auto dllTemplatedString = createDLLTemplatedWorkerString("DLL_STRING");
    
    ASSERT_NE(hostTemplatedInt, nullptr);
    ASSERT_NE(dllTemplatedInt, nullptr);
    ASSERT_NE(hostTemplatedString, nullptr);
    ASSERT_NE(dllTemplatedString, nullptr);
    
    // Store references to avoid typeid side effect warnings
    const auto& hostTemplatedInt_ref = *hostTemplatedInt;
    const auto& dllTemplatedInt_ref = *dllTemplatedInt;
    const auto& hostTemplatedString_ref = *hostTemplatedString;
    const auto& dllTemplatedString_ref = *dllTemplatedString;
    
    // Test type name consistency for templates
    EXPECT_STREQ(typeid(hostTemplatedInt_ref).name(), typeid(dllTemplatedInt_ref).name());
    EXPECT_STREQ(typeid(hostTemplatedString_ref).name(), typeid(dllTemplatedString_ref).name());
    
    // Test cross-boundary template casting - the critical functionality
    auto* castedDLLInt = dynamic_cast<TemplatedWorker<int>*>(dllTemplatedInt.get());
    auto* castedHostInt = dynamic_cast<TemplatedWorker<int>*>(hostTemplatedInt.get());
    auto* castedDLLString = dynamic_cast<TemplatedWorker<std::string>*>(dllTemplatedString.get());
    auto* castedHostString = dynamic_cast<TemplatedWorker<std::string>*>(hostTemplatedString.get());
    
    // These casts should succeed for proper template unification
    EXPECT_NE(castedDLLInt, nullptr);
    EXPECT_NE(castedHostInt, nullptr);
    EXPECT_NE(castedDLLString, nullptr);
    EXPECT_NE(castedHostString, nullptr);
    
    // Test that template instances work correctly
    if (castedDLLInt) {
        EXPECT_EQ(castedDLLInt->getValue(), static_cast<int>(reinterpret_cast<intptr_t>(&castedDLLInt->getData())));
    }
    if (castedHostString) {
        EXPECT_EQ(castedHostString->getData(), "HOST_STRING");
    }
}

// Test C interface
TEST(WeakSymbolLinking, CInterface) {
    // Create object through C interface
    IBaseObject* cObj = create_dll_object_c(9999);
    ASSERT_NE(cObj, nullptr) << "Failed to create object through C interface";
    
    // Test C interface functions
    EXPECT_STREQ(get_type_name_c(cObj), "N17WeakSymbolExample12SharedWorkerE");
    EXPECT_EQ(test_dynamic_cast_c(cObj), 1) << "C interface dynamic cast failed";
    
    // Test that we can also cast it using C++ interface
    SharedWorker* cppCasted = dynamic_cast<SharedWorker*>(cObj);
    EXPECT_NE(cppCasted, nullptr) << "C++ dynamic_cast on C-created object failed";
    
    if (cppCasted) {
        EXPECT_NO_THROW(cppCasted->performAction());
        EXPECT_EQ(cppCasted->getValue(), 9999);
        EXPECT_EQ(cppCasted->getTypeName(), "SharedWorker");
    }
    
    // Test C interface info function (should not crash)
    EXPECT_NO_THROW(print_object_info_c(cObj));
    
    // Clean up
    destroy_dll_object_c(cObj);
}

// Test weak symbol function unification
TEST(WeakSymbolLinking, WeakSymbolFunctions) {
    // Test that host weak symbol functions work
    EXPECT_NO_THROW(demonstrateHostWeakSymbolUnification());
    
    // Test that DLL weak symbol functions work
    EXPECT_NO_THROW(demonstrateWeakSymbolUnification());
    
    // Test additional cross-boundary functionality
    EXPECT_NO_THROW(verifyTypeUnificationAcrossBoundary());
}

// Main function - Google Test entry point
int main(int argc, char** argv) {
    std::cout << "Weak Symbol Linking Demonstration with Google Test" << std::endl;
    std::cout << "Platform: macOS" << std::endl;
    std::cout << "Compiler: " << __VERSION__ << std::endl;
    std::cout << "RTTI Enabled: " << (__cpp_rtti ? "YES" : "NO") << std::endl;
    std::cout << std::endl;
    
    // Initialize Google Test
    ::testing::InitGoogleTest(&argc, argv);
    
    // Run all tests
    return RUN_ALL_TESTS();
} 