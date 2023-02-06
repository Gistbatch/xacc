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
 *   Philipp Seitz - Async
 *******************************************************************************/
#include "xacc.hpp"
#include "future"
#include "thread"
int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  xacc::set_verbose(true);
  xacc::setLoggingLevel(2);

  // Get reference to the Accelerator
  xacc::info("Accelerator init!");
  auto accelerator = xacc::getAccelerator("aer");
  xacc::info("AcceleratorDecorator init!");
  accelerator = xacc::getAcceleratorDecorator("hpc-scheduled", accelerator);
  // Allocate some qubits
  xacc::info("Alloc Qubits!");
  auto buffer1 = xacc::qalloc(2);
  auto buffer2 = xacc::qalloc(3);
  auto quilCompiler = xacc::getCompiler("quil");
  auto ir1 = quilCompiler->compile(R"(__qpu__ void bell(qbit q) {
H 0
CX 0 1
MEASURE 0 [0]
MEASURE 1 [1]
})",
                                   accelerator);

  auto ir2 = quilCompiler->compile(R"(__qpu__ void bell(qbit q) {
H 0
CX 0 1
CX 1 2
MEASURE 0 [0]
MEASURE 1 [1]
MEASURE 2 [2]
})",
                                   accelerator);
  // update before calling
  xacc::info("Start Execution!");
  const std::string call1 = "my_call1";
  const std::string call2 = "my_call2";
  accelerator->updateConfiguration({{"reference-handle", call1}});
  accelerator->execute(buffer1, ir1->getComposites()[0]);
  accelerator->updateConfiguration({{"reference-handle", call2}});
  accelerator->execute(buffer2, ir2->getComposites()[0]);

  xacc::info("Accelerator is not blocking!");
  auto properties = accelerator->getProperties();

  auto acc_future = properties.get<std::shared_future<void>>(call1);
  xacc::info("Getting reference to first future...");
  acc_future.get();
  xacc::info("Got future!");
  buffer1->print();

  acc_future = properties.get<std::shared_future<void>>(call2);
  xacc::info("Getting reference to second future...");
  acc_future.get();
  xacc::info("Got future!");
  buffer2->print();
  xacc::Finalize();

  return 0;
}
