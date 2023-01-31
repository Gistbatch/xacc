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
 *******************************************************************************/
#include "hpc_het_decorator.hpp"
#include "hpc_virt_decorator.hpp"

#include "cppmicroservices/BundleActivator.h"
#include "cppmicroservices/BundleContext.h"
#include "cppmicroservices/ServiceProperties.h"


using namespace cppmicroservices;

namespace {

/**
 */
class US_ABI_LOCAL HPCDecoratorsActivator: public BundleActivator {

public:

	HPCDecoratorsActivator() {
	}

	void Start(BundleContext context) {
		auto c1 = std::make_shared<xacc::quantum::HPCHetDecorator>();
		auto c2 = std::make_shared<xacc::quantum::HPCVirtDecorator>();

		context.RegisterService<xacc::AcceleratorDecorator>(c1);
        context.RegisterService<xacc::Accelerator>(c1);

        context.RegisterService<xacc::AcceleratorDecorator>(c2);
        context.RegisterService<xacc::Accelerator>(c2);
	}

	/**
	 */
	void Stop(BundleContext /*context*/) {
	}

};

}

CPPMICROSERVICES_EXPORT_BUNDLE_ACTIVATOR(HPCDecoratorsActivator)
