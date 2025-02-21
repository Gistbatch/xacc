
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

typedef std::packaged_task<void()> LambdaTask;
typedef std::shared_ptr<LambdaTask> TaskPtr;

class Compare {
public:
  bool operator()(TaskPtr first, TaskPtr second) {
    // implement some logic here
    return false;
  }
};

typedef std::priority_queue<TaskPtr, std::vector<TaskPtr>, Compare> MyQueue;

class HPCScheduledDecorator : public AcceleratorDecorator {
protected:
  std::string current_job_reference;
  std::thread worker_thread;
  HeterogeneousMap decorator_properties;
  std::shared_ptr<MyQueue> jobs;
  bool scheduler_running = false;

  void handle_jobs() {
    using namespace std::chrono_literals;
    while (this->scheduler_running) {
      if (!(this->jobs->empty())) {
        xacc::info("Executing next job...");
        auto job = this->jobs->top();
        job->operator()();
        xacc::info("Job executed!");
        this->jobs->pop();
      }
      xacc::info("Waiting for new jobs.");

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

  ~HPCScheduledDecorator() override {
    if (scheduler_running) {
      scheduler_running = false;
      worker_thread.join();
    }
  }
};

} // namespace quantum
} // namespace xacc
#endif
