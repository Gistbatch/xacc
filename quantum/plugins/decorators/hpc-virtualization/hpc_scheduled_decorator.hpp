
/*******************************************************************************
 * Copyright (c) 2018 UT-Battelle, LLC.
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompanies this
 * distribution. The Eclipse Public License is available at
 * http://www.eclipse.org/legal/epl-v10.html and the Eclipse Distribution
 *License is available at https://eclipse.org/org/documents/edl-v10.php
 *
 * Contributors:
 *   Alexander J. McCaskey - initial API and implementation
 *   Daniel Claudino - MPI native implementation
 *   Philipp Seitz - Async
 *******************************************************************************/
#ifndef XACC_HPC_SCHEDULED_DECORATOR_HPP_
#define XACC_HPC_SCHEDULED_DECORATOR_HPP_

#include <chrono>
#include <functional>
#include <future>
#include <queue>
#include <thread>

#include "xacc.hpp"
#include "AcceleratorDecorator.hpp"

namespace xacc {

namespace quantum {

typedef std::packaged_task<void()>
    MyTask;
typedef std::shared_ptr<MyTask> MyTaskPtr;
typedef std::shared_ptr<std::pair<std::string, MyTaskPtr>> MyPair;

class Compare {
public:
  
  //bool operator()(MyTaskPtr first, MyTaskPtr second) {
  bool operator()(MyPair first, MyPair second) {
    // implement some logic here
    return false;
  }
};
//typedef std::priority_queue<MyTaskPtr, std::vector<MyTaskPtr>, Compare> MyQueue;
typedef std::priority_queue<MyPair, std::vector<MyPair>, Compare> MyQueue;

class HPCScheduledDecorator : public AcceleratorDecorator {
protected:
  std::string last;
  std::thread my_thread;
  // std::future<void> callReference;
  HeterogeneousMap decorator_properties;
  std::shared_ptr<MyQueue> jobs;
  // static void
  // handle_jobs(std::priority_queue<std::packaged_task<void()>*,std::vector<std::packaged_task<void()>*>,Compare>
  // *my_jobs) {
  void handle_jobs() {
    using namespace std::chrono_literals;
    while (true) {
      if (!(this->jobs->empty())) {
        xacc::info("Executing next job");
        auto pair = this->jobs->top();
        this->jobs->pop();
        auto job = std::get<1>(*pair);
        auto ref = std::get<0>(*pair);
        job->operator()();
        xacc::info("Submitting job at " + ref);
        std::shared_future<void> job_future = job->get_future();
        this->decorator_properties.insert(ref, job_future);
        xacc::info("Called Job");
      }
      xacc::info("Waiting for next job");

      std::this_thread::sleep_for(1000ms);
    }
  }

public:
  void initialize(const HeterogeneousMap &params = {}) override;

  void updateConfiguration(const HeterogeneousMap &config) override;

  const std::vector<std::string> configurationKeys() override { return {}; }

  HeterogeneousMap getProperties() override {
    auto tmp = HeterogeneousMap();
    tmp.merge(decorator_properties);
    auto decorated_properties = decoratedAccelerator->getProperties();
    tmp.merge(decorated_properties);
    return tmp;
  }

  void execute(std::shared_ptr<AcceleratorBuffer> buffer,
               const std::shared_ptr<CompositeInstruction> function) override;
  void execute(std::shared_ptr<AcceleratorBuffer> buffer,
               const std::vector<std::shared_ptr<CompositeInstruction>>
                   functions) override;

  const std::string name() const override { return "hpc-scheduled"; }
  const std::string description() const override { return ""; }

  ~HPCScheduledDecorator() override { my_thread.join(); }
};

} // namespace quantum
} // namespace xacc
#endif
