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

int main(int argc, char **argv) {
  xacc::Initialize(argc, argv);
  xacc::set_verbose(true);
  xacc::setLoggingLevel(1);

  // Get reference to the Accelerator
  auto accelerator = xacc::getAccelerator("aer");
  accelerator = xacc::getAcceleratorDecorator("hpc-async", accelerator);
  // Allocate some qubits
  auto buffer = xacc::qalloc(2);

  auto quilCompiler = xacc::getCompiler("quil");
  auto ir = quilCompiler->compile(R"(__qpu__ void bell(qbit q) {
H 0
CX 0 1
MEASURE 0 [0]
MEASURE 1 [1]
})",
                                  accelerator);
  accelerator->execute(buffer, ir->getComposites()[0]);
  xacc::info("Accelerator is not blocking!");
  auto properties = accelerator->getProperties();
  // auto acc_future = accelerator->getProperties();
  // acc_future.get<int>("call-reference");
  auto acc_future =
      properties.getPointerLike<std::future<void>>("call-reference");
  acc_future->get();
  buffer->print();

  xacc::Finalize();

  return 0;
}
