/******************************************************************************
 * This file is part of dOpenCL.
 * 
 * dOpenCL is an implementation of the OpenCL application programming
 * interface for distributed systems. See <http://dopencl.uni-muenster.de/>
 * for more information.
 * 
 * Developed by: Research Group Parallel and Distributed Systems
 *               Department of Mathematics and Computer Science
 *               University of Muenster, Germany
 *               <http://pvs.uni-muenster.de/>
 * 
 * Copyright (C) 2013  Philipp Kegel <philipp.kegel@uni-muenster.de>
 *
 * dOpenCL is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * dOpenCL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with dOpenCL. If not, see <http://www.gnu.org/licenses/>.
 * 
 * Permission to use dOpenCL for scientific, non-commercial work is
 * granted under the terms of the dOpenCL Academic License provided
 * appropriate credit is given. See the dOpenCL Academic License for
 * more details.
 * 
 * You should have received a copy of the dOpenCL Academic License
 * along with dOpenCL. If not, see <http://dopencl.uni-muenster.de/>.
 ******************************************************************************/

/*!
 * \file DCLTypes.h
 *
 * \date 2011-02-05
 * \author Philipp Kegel
 *
 * dOpenCL C++ API type declarations
 */

#ifndef DCLTYPES_H_
#define DCLTYPES_H_

#if defined(IO_LINK_COMPRESSION) && defined(USE_CL_IO_LINK_COMPRESSION)
#include <lib842/cl.h>
#endif

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/functional/hash.hpp>
#include <boost/version.hpp>

#include <cstdint>

#if BOOST_VERSION < 108600
namespace std {

template<>
struct hash<boost::uuids::uuid> {
    size_t operator () (const boost::uuids::uuid& uid) const {
        return boost::hash<boost::uuids::uuid>()(uid);
    }
};

}  /* namespace std */
#endif

namespace dcl {

typedef uint32_t object_id; //!< a application object identifier
typedef boost::uuids::uuid process_id; //!< a unique process identifier
typedef boost::uuids::uuid transfer_id; //!< identifier for a host-device data transfer

static dcl::transfer_id create_transfer_id() {
    return boost::uuids::random_generator()();
}

// Allows getting another transfer ID from an existing transfer ID
// This is useful in case an operation requires multiple actual data transfers,
// since only the first transfer ID for the operation needs to be transferred between nodes
static void next_transfer_id(transfer_id &transfer_id) {
    for (size_t i = 0; i < boost::uuids::uuid::static_size(); i++) {
        transfer_id.data[boost::uuids::uuid::static_size()-i-1]++;
        if (transfer_id.data[boost::uuids::uuid::static_size()-i-1] != 0)
            break;
    }
}

enum class kernel_arg_type {
	BINARY, MEMORY, SAMPLER
}; //!< A kernel argument type

} /* namespace dcl */

#endif /* DCLTYPES_H_ */
