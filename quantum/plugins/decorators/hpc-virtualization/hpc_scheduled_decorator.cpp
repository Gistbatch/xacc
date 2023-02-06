
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
  if (params.keyExists<std::string>("reference-handle")) {
    current_job_reference = params.get<std::string>("reference-handle");
  }
  decoratedAccelerator->initialize(params);
  xacc::info("Launching worker thread...");
  scheduler_running = true;
  MyQueue my_jobs;
  jobs = std::make_shared<MyQueue>(my_jobs);
  worker_thread = std::thread(&HPCScheduledDecorator::handle_jobs, this);
  xacc::info("Worker launched!");
}

void HPCScheduledDecorator::updateConfiguration(
    const HeterogeneousMap &config) {
  if (config.keyExists<std::string>("reference-handle")) {
    current_job_reference = config.get<std::string>("reference-handle");
  }
  decoratedAccelerator->updateConfiguration(config);
}

void HPCScheduledDecorator::execute(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::shared_ptr<CompositeInstruction> function) {
  if (decoratedAccelerator) {
    xacc::info("Creating new job ...");
    auto task = std::make_shared<LambdaTask>([=] {
      xacc::info("Calling Task for buffer " + buffer->name() + " ...");
      buffer->resetBuffer();
      this->decoratedAccelerator->execute(buffer, function);
      return;
    });
    jobs->push(task);
    std::shared_future<void> task_future = task->get_future();
    this->decorator_properties.insert(current_job_reference, task_future);
    xacc::info("Job submitted!");
  }
  return;
}

void HPCScheduledDecorator::execute(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::vector<std::shared_ptr<CompositeInstruction>> functions) {
  if (decoratedAccelerator) {
    xacc::info("Creating new job ...");
    auto task = std::make_shared<LambdaTask>([=] {
      xacc::info("Calling Task for buffer " + buffer->name() + " ...");
      buffer->resetBuffer();
      this->decoratedAccelerator->execute(buffer, functions);
      xacc::info("Task called!");
      return;
    });
    jobs->push(task);
    std::shared_future<void> task_future = task->get_future();
    this->decorator_properties.insert(current_job_reference, task_future);
    xacc::info("Job submitted!");
  }
  return;
}

} // namespace quantum
} // namespace xacc
