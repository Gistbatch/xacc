
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
#include "hpc_async_decorator.hpp"
#include "InstructionIterator.hpp"
#include "xacc.hpp"

namespace xacc {
namespace quantum {

void HPCAsyncDecorator::initialize(const HeterogeneousMap &params) {
  decoratedAccelerator->initialize(params);
}

void HPCAsyncDecorator::updateConfiguration(const HeterogeneousMap &config) {
  decoratedAccelerator->updateConfiguration(config);
}

void HPCAsyncDecorator::execute(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::shared_ptr<CompositeInstruction> function) {
  if (decoratedAccelerator) {
    callReference = std::async(std::launch::async, [=] {
      decoratedAccelerator->execute(buffer, function);
    });
    decorator_properties.insert("call-reference", &callReference);
  }
  return;
}

void HPCAsyncDecorator::execute(
    std::shared_ptr<AcceleratorBuffer> buffer,
    const std::vector<std::shared_ptr<CompositeInstruction>> functions) {
  if (decoratedAccelerator) {
    callReference = std::async(std::launch::async, [=] {
      decoratedAccelerator->execute(buffer, functions);
    });
    decorator_properties.insert("call-reference", &callReference);
  }
  return;
}

} // namespace quantum
} // namespace xacc
