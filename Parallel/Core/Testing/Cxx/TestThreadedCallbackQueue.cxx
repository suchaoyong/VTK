/*=========================================================================

  Program:   Visualization Toolkit
  Module:    TestThreadedCallbackQueue.cxx

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkThreadedCallbackQueue.h"

#include "vtkIntArray.h"
#include "vtkLogger.h"
#include "vtkNew.h"
#include "vtkSmartPointer.h"

#include <atomic>
#include <chrono>
#include <functional>
#include <thread>

namespace
{
//-----------------------------------------------------------------------------
void RunThreads(int nthreadsBegin, int nthreadsEnd)
{
  vtkNew<vtkThreadedCallbackQueue> queue;
  queue->SetNumberOfThreads(nthreadsBegin);
  queue->Start();
  std::atomic_int count(0);
  int N = 100000;

  // We are testing if the queue can properly resize itself and doesn't have deadlocks
  for (vtkIdType i = 0; i < N; ++i)
  {
    if (i == N / 2)
    {
      queue->Start();
    }
    if (i == N / 4)
    {
      queue->Stop();
    }
    vtkSmartPointer<vtkIntArray> array = vtkSmartPointer<vtkIntArray>::New();
    queue->Push(
      [&count](const int& n, const double&&, char, vtkIntArray* a1, vtkIntArray* a2) {
        a1->SetName(std::to_string(n).c_str());
        a2->SetName(std::to_string(n).c_str());
        ++count;
      },
      i, 0, 'a', vtkNew<vtkIntArray>(), array);
  }

  queue->SetNumberOfThreads(nthreadsEnd);

  // If the jobs are not run, this test will do an infinite loop
  while (count != N)
    ;
}

//=============================================================================
struct A
{
  A() { vtkLog(INFO, "Constructor"); }
  A(A&& other)
  noexcept
    : array(std::move(other.array))
  {
    vtkLog(INFO, "Move constructor");
  }
  A(const A& other)
    : array(other.array)
  {
    vtkLog(INFO, "Copy constructor called.");
  }
  void f(A&, A&&) {}
  void const_f(A&, A&&) const {}
  void operator()(A&, A&&) { std::cout << *array << std::endl; }

  vtkSmartPointer<vtkIntArray> array = vtkSmartPointer<vtkIntArray>::New();
};

//-----------------------------------------------------------------------------
void f(A&, A&&) {}

//-----------------------------------------------------------------------------
void TestFunctionTypeCompleteness()
{
  // We create a queue outside of the score where things are pushed to ensure that the pushed
  // objects are persistent.
  vtkNew<vtkThreadedCallbackQueue> queue;
  {
    // Testing the queue on some exotic inputs

    // lambdas
    queue->Push([](A&&) {}, ::A());
    queue->Push([](::A&, const ::A&, ::A&&, const ::A&&) {}, ::A(), ::A(), ::A(), ::A());

    // member function pointers
    queue->Push(&::A::f, ::A(), ::A(), ::A());
    queue->Push(&::A::const_f, ::A(), ::A(), ::A());

    // functor
    queue->Push(::A(), ::A(), ::A());

    // function pointer
    queue->Push(&::f, ::A(), ::A());

    // Passing an lvalue reference, which needs to be copied.
    A a;
    queue->Push(a, ::A(), ::A());

    // Passing a pointer wrapped functor
    queue->Push(std::unique_ptr<A>(new ::A()), ::A(), ::A());

    // Passing a pointer wrapped object with a member function pointer
    queue->Push(&::A::f, std::unique_ptr<A>(new ::A()), ::A(), ::A());

    // Passing a std::function
    std::function<void(::A&, ::A &&)> func = f;
    queue->Push(func, ::A(), ::A());
  }
  queue->Start();
}

//-----------------------------------------------------------------------------
bool TestTokens()
{
  int N = 100;
  bool retVal = true;
  while (--N && retVal)
  {
    vtkNew<vtkThreadedCallbackQueue> queue;
    queue->SetNumberOfThreads(4);

    int count = 0;

    auto f = [&count](std::string&, int low, int up) {
      if (count < low || count > up)
      {
        ++count;
        return false;
      }
      ++count;
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      return true;
    };

    using Array = std::vector<vtkThreadedCallbackQueue::TokenPointer>;

    auto token1 = queue->Push(f, std::string("t1"), 0, 1);
    auto token2 = queue->PushDependent(Array{ token1 }, f, std::string("t2"), 1, 4);
    auto token3 = queue->PushDependent(Array{ token1, token2 }, f, std::string("t3"), 2, 5);
    auto token4 = queue->PushDependent(Array{ token1 }, f, std::string("t4"), 1, 4);
    auto token5 = queue->Push(f, std::string("t5"), 0, 4);
    queue->Start();

    token3->Future.wait();

    retVal &= (token1->Future.get() && token2->Future.get() && token3->Future.get() &&
      token4->Future.get() && token5->Future.get());
  }
  return retVal;
}
} // anonymous namespace

int TestThreadedCallbackQueue(int, char*[])
{
  vtkLog(INFO, "Testing tokens");
  bool retVal = ::TestTokens();

  ::TestFunctionTypeCompleteness();

  vtkLog(INFO, "Testing expanding from 2 to 8 threads");
  // Testing expanding the number of threads
  ::RunThreads(2, 8);

  vtkLog(INFO, "Testing shrinking from 8 to 2 threads");
  // Testing shrinking the number of threads
  ::RunThreads(8, 2);

  return retVal ? EXIT_SUCCESS : EXIT_FAILURE;
}