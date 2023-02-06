
/*******************************************************************************
 * Copyright (c) 2019 UT-Battelle, LLC.
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
#include "hpc_scheduled_decorator.hpp"
#include "InstructionIterator.hpp"
#include "xacc.hpp"

namespace xacc {
namespace quantum {

void HPCScheduledDecorator::initialize(const HeterogeneousMap &params) {
  xacc::info("Calling init worker");
  if (params.keyExists<std::string>("reference-handle")) {
    last = params.get<std::string>("reference-handle");
  }
  decoratedAccelerator->initialize(params);
  xacc::info("launching worker");
  MyQueue my_jobs;
  jobs = std::make_shared<MyQueue>(my_jobs);
  my_thread = std::thread(&HPCScheduledDecorator::handle_jobs, this);
  xacc::info("Worker launched!");
}

void HPCScheduledDecorator::updateConfiguration(
    const HeterogeneousMap &config) {
  xacc::info("Call update");
  if (config.keyExists<std::string>("reference-handle")) {
    last = config.get<std::string>("reference-handle");
    xacc::info("Updated config to: " + last);
  }
  decoratedAccelerator->updateConfiguration(config);
}

void HPCScheduledDecorator::execute(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::shared_ptr<CompositeInstruction> function) {
  xacc::info("1Calling execute!");
  if (decoratedAccelerator) {
    xacc::info("1Creating job");
    auto task = std::make_shared<MyTask>([=]{
      buffer->resetBuffer();
      this->decoratedAccelerator->execute(buffer, function);
      buffer->print();
      return;
    });
    xacc::info("1Submitting job at " + last);
    auto job = std::make_shared<std::pair<std::string, MyTaskPtr>>(last, task);
    jobs->push(job);
    xacc::info("1Job submitted!");
  }
  return;
}

void HPCScheduledDecorator::execute(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::vector<std::shared_ptr<CompositeInstruction>> functions) {
  xacc::info("Calling execute on multiple!");
  if (decoratedAccelerator) {
    xacc::info("Creating job");
    auto task = std::make_shared<MyTask>([=]{
      xacc::info("Calling Task: " + this->last);
      buffer->resetBuffer();
      this->decoratedAccelerator->execute(buffer, functions);
      xacc::info("Task called!");
      buffer->print();
      return;
    });
    xacc::info("Submitting job with ref " + last);
    auto job = std::make_shared<std::pair<std::string, MyTaskPtr>>(last, task);
    jobs->push(job);
    xacc::info("Job submitted!");
  }
  return;
}

} // namespace quantum
} // namespace xacc
