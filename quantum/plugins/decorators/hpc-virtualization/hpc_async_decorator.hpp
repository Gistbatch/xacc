
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
#ifndef XACC_HPC_ASYNC_DECORATOR_HPP_
#define XACC_HPC_ASYNC_DECORATOR_HPP_

#include <future>
#include "xacc.hpp"
#include "AcceleratorDecorator.hpp"

namespace xacc {

namespace quantum {

class HPCAsyncDecorator : public AcceleratorDecorator {
protected:
  std::future<void> callReference;
  HeterogeneousMap decorator_properties;
  const int test = 1;

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

  const std::string name() const override { return "hpc-async"; }
  const std::string description() const override { return ""; }

  ~HPCAsyncDecorator() override {}
};

} // namespace quantum
} // namespace xacc
#endif
