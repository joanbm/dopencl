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
 * \file DataCompressionWorkPool.cpp
 *
 * \date 2020-05-05
 * \author Joan Bruguera Micó
 */

// TODOXXX: Should add the code to spread the threads among NUMA zones for HW? (From lib842 sample)

#ifdef IO_LINK_COMPRESSION

#include "DataCompressionWorkPool.h"

// If INDEPTH_TRACE is defined, more traces and statistics are generated
//#define INDEPTH_TRACE

namespace dclasio {

namespace comm {

DataCompressionWorkPool::DataCompressionWorkPool(
    lib842_compress_func compress842_func,
    unsigned int num_threads,
    std::function<std::ostream&(void)> error_logger,
    std::function<std::ostream&(void)> debug_logger) :
    _compress842_func(compress842_func),
    _error_logger(std::move(error_logger)),
    _debug_logger(std::move(debug_logger)),
    _trigger(false), _quit(false),
    _ptr(nullptr), _size(0), _skip_compress_step(false),
    _current_offset(0),
    _error(false),
    _start_barrier(num_threads),
    _finish_barrier(num_threads+1) {
    _threads.reserve(num_threads);
    for (size_t i = 0; i < num_threads; i++)
        _threads.emplace_back(&DataCompressionWorkPool::loop_compress_thread, this, i);
}

DataCompressionWorkPool::~DataCompressionWorkPool() {
    {
        std::lock_guard<std::mutex> lock(_trigger_mutex);
        _trigger = true;
        _quit = true;
        _trigger_changed.notify_all();
    }
    for (auto &t : _threads)
        t.join();
}

void DataCompressionWorkPool::start(
    const void *ptr, size_t size, bool skip_compress_step,
    std::function<void(compress_block &&)> block_available_callback) {
    _block_available_callback = std::move(block_available_callback);
    _ptr = ptr;
    _size = size;
    _skip_compress_step = skip_compress_step;
    _current_offset = 0;

    std::lock_guard<std::mutex> lock(_trigger_mutex);
    _trigger = true;
    _trigger_changed.notify_all();
}

void DataCompressionWorkPool::finish(bool cancel) {
    if (cancel) {
        _error = true;
    }
    _finish_barrier.wait();
}

void DataCompressionWorkPool::loop_compress_thread(size_t thread_id) {
    static constexpr size_t CHUNK_SIZE = dcl::DataTransfer::COMPR842_CHUNK_SIZE;

#ifdef INDEPTH_TRACE
    _debug_logger()
        << "(DataStream to " << _remote_endpoint << ") "
        << "Start compression thread with id " << thread_id
        << std::endl;
    size_t stat_handled_blocks = 0;
#endif
    while (true) {
        if (thread_id == 0 && _error) {
            _error = false;
        }

        {
            std::unique_lock<std::mutex> lock(_trigger_mutex);
            _trigger_changed.wait(lock, [this] { return _trigger; });
            if (_quit)
                break;
        }

        _start_barrier.wait();
        if (thread_id == 0) {
            std::lock_guard<std::mutex> lock(_trigger_mutex);
            _trigger = false;
        }

        auto last_valid_offset = _size & ~(dcl::DataTransfer::NETWORK_BLOCK_SIZE-1);

        while (true) {
            size_t offset = _current_offset.fetch_add(dcl::DataTransfer::NETWORK_BLOCK_SIZE);
            if (offset >= last_valid_offset) {
                break;
            }

#ifdef INDEPTH_TRACE
            stat_handled_blocks++;
#endif

            compress_block block;
            block.source_offset = offset;
            if (_skip_compress_step) {
                for (size_t i = 0; i < dcl::DataTransfer::NUM_CHUNKS_PER_NETWORK_BLOCK; i++) {
                    auto source = static_cast<const uint8_t *>(_ptr) + offset + i * CHUNK_SIZE;

                    auto is_compressed = std::equal(source,source + sizeof(CL842_COMPRESSED_CHUNK_MAGIC), CL842_COMPRESSED_CHUNK_MAGIC);

                    auto chunk_buffer_size = is_compressed
                                             ? *reinterpret_cast<const uint64_t *>((source + sizeof(CL842_COMPRESSED_CHUNK_MAGIC)))
                                             : CHUNK_SIZE;
                    auto chunk_buffer = is_compressed
                            ? source + CHUNK_SIZE - chunk_buffer_size
                            : source;

                    block.datas[i] = chunk_buffer;
                    block.sizes[i] = chunk_buffer_size;
                }
            } else {
                // TODOXXX: This can be reduced to e.g. COMPRESSIBLE_THRESHOLD or CHUNK_SIZE,
                // as long as the lib842 compressor respects the destination buffer size
                static constexpr size_t CHUNK_PADDING = 2 * CHUNK_SIZE;
                block.compress_buffer.reset(
                    new uint8_t[CHUNK_PADDING * dcl::DataTransfer::NUM_CHUNKS_PER_NETWORK_BLOCK]);

                bool any_compressible = false;
                for (size_t i = 0; i < dcl::DataTransfer::NUM_CHUNKS_PER_NETWORK_BLOCK; i++) {
                    auto source = static_cast<const uint8_t *>(_ptr) + offset + i * CHUNK_SIZE;
                    auto compressed_destination = block.compress_buffer.get() + i * CHUNK_PADDING;

                    // Compress chunk
                    size_t compressed_size = CHUNK_PADDING;

                    int ret = _compress842_func(source, CHUNK_SIZE, compressed_destination, &compressed_size);
                    if (ret != 0) {
                        _error_logger()
                            << "Data compression failed, aborting operation"
                            << std::endl;
                        block.source_offset = SIZE_MAX;
                        break;
                    }

                    // Push into the chunk queue
                    auto compressible = compressed_size <= dcl::DataTransfer::COMPRESSIBLE_THRESHOLD;

                    block.datas[i] = compressible ? compressed_destination : source;
                    block.sizes[i] = compressible ? compressed_size : CHUNK_SIZE;
                    any_compressible |= compressible;
                }

                // If no chunk is compressible, release the unused compression buffer now
                if (!any_compressible)
                    block.compress_buffer.reset();
            }

            if (!_error || block.source_offset == SIZE_MAX) {
                _block_available_callback(std::move(block));
            }
        }

        _finish_barrier.wait();
    }

#ifdef INDEPTH_TRACE
    _debug_logger()
        << "(DataStream to " << _remote_endpoint << ") "
        << "End compression thread with id " << thread_id << " (stat_handled_blocks=" << stat_handled_blocks << ")"
        << std::endl;
#endif
}

} // namespace comm

} // namespace dclasio

#endif // IO_LINK_COMPRESSION
