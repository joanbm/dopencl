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
 * \file DataDecompressionWorkPool.h
 *
 * \date 2020-05-05
 * \author Joan Bruguera Micó
 */

#ifndef DATADECOMPRESSIONWORKPOOL_H_
#define DATADECOMPRESSIONWORKPOOL_H_

#ifdef IO_LINK_COMPRESSION

#include "Barrier.h"

#include <dcl/DataTransfer.h>

#include <condition_variable>
#include <mutex>
#include <thread>

#include <array>
#include <vector>
#include <queue>

#include <functional>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <ostream>

namespace dclasio {

namespace comm {

class DataDecompressionWorkPool {
public:
    struct decompress_chunk {
        const uint8_t *compressed_data;
        size_t compressed_length;
        void *destination;

        // Disable default copy constructor/assignment to prevent accidental performance hit
        decompress_chunk() :
            compressed_data(nullptr), compressed_length(0), destination(nullptr) { }
        decompress_chunk(const uint8_t *compressed_data, size_t compressed_length, void *destination) :
            compressed_data(compressed_data), compressed_length(compressed_length), destination(destination) { }
        decompress_chunk(const decompress_chunk &) = delete;
        decompress_chunk& operator=(const decompress_chunk &) = delete;
        decompress_chunk(decompress_chunk &&) = default;
        decompress_chunk& operator=(decompress_chunk &&) = default;
    };

    struct decompress_block {
        std::array<decompress_chunk, dcl::DataTransfer::NUM_CHUNKS_PER_NETWORK_BLOCK> chunks;

        // Buffer that owns the pointers used in 'compressed_data'. Used internally.
        std::unique_ptr<const uint8_t[]> compress_buffer;
    };

    DataDecompressionWorkPool(std::function<std::ostream&(void)> error_logger,
                              std::function<std::ostream&(void)> debug_logger);
    ~DataDecompressionWorkPool();
    /* Starts a new decompression operation. */
    void start();
    /* Enqueues a new to be decompressed */
    bool push_block(DataDecompressionWorkPool::decompress_block &&dm);
    /* Wait for the decompression queue to be cleared up and then call the specified callback.
     * If cancel = false, the decompression queue will be fully processed before
     *                    invoking the callback (unless an error happens).
     * If cancel = true, the decompression operation will be finished as soon as possible,
     *                   possibly dropping most or all of the decompression queue.
     * The parameter of the callback specifies a success (true) / error (false) status. */
    void finalize(bool cancel, const std::function<void(bool)> &finalize_callback);

private:
    void loop_decompress_thread(size_t thread_id);

    enum class decompress_state {
        // The decompressor is working (or waiting for new blocks)
        processing,
        // The decompressor is working, but the user of this class is waiting for the
        // remaining decompression blocks to be processed.
        finalizing,
        // The decompressor is working, but the user of this class has ordered that the
        // current decompression process should be cancelled.
        cancelling,
        // The decompressor has found an error during decompression and the current
        // decompression process is being cancelled
        handling_error,
        // The thread pool is being destroyed
        quitting
    };

    std::function<std::ostream&(void)> _error_logger;
    std::function<std::ostream&(void)> _debug_logger;

    // Instance of the decompression threads
    std::vector<std::thread> _threads;
    // Mutex for protecting concurrent accesses to
    // (_state, _queue, _report_error, _working_thread_count)
    std::mutex _queue_mutex;
    // Stores the current action being performed by the threads
    decompress_state _state;
    // Stores pending decompression operations
    std::queue<decompress_block> _queue;
    // Indicates that a decompression error happened and the user of this class should be notified
    bool _report_error;
    // Number of threads currently running decompression operations
    unsigned int _working_thread_count;
    // Callback to be called after finalizing or cancelling is done
    std::function<void(bool)> _finalize_callback;
    // Wakes up the decompression threads when new operations have been added to the queue
    std::condition_variable _queue_available;
    // Barrier for finishing decompression, necessary for ensuring that resources
    // are not released until all threads have finished
    barrier _finish_barrier;
};

} // namespace comm

} // namespace dclasio

#endif // IO_LINK_COMPRESSION

#endif // DATADECOMPRESSIONWORKPOOL_H_
