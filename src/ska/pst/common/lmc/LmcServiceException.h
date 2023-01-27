/*
 * Copyright 2022 Square Kilometre Array Observatory
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __SKA_PST_COMMON_LmcServiceException_h
#define __SKA_PST_COMMON_LmcServiceException_h

#include "ska/pst/lmc/ska_pst_lmc.pb.h"
#include <grpc++/grpc++.h>

namespace ska::pst::common {

    /**
     * @brief Custom exception that can be used by @see LmcServiceHandler instances to return specific error codes.
     *
     * The @see LmcService can handle any exception but it will return to the client as an internal service error.
     * This class can be used by implementations of the @see LmcServiceHandler to raise a specific exception from
     * that can provide the ska::pst::lmc::ErrorCode and the grpc::StatusCode.
     *
     * The difference between the error_code and status_code is subtle but the error_code is specific to PST
     * and provides more detail. The status_code is a common enum used within gRPC.
     */
    class LmcServiceException : public std::exception {

        public:
            /**
             * @brief Constructor for an LmcServiceException.
             *
             * @param msg The error message for the exception. This is returned when calling @see what.
             * @param error_code The PST LMC Error code to use. Defaults to INTERNAL_ERROR.
             * @param status_code the gRPC status code to use. Defaults to INTERAL.
             */
            LmcServiceException(
                const char* msg,
                ska::pst::lmc::ErrorCode error_code = ska::pst::lmc::ErrorCode::INTERNAL_ERROR,
                grpc::StatusCode status_code = grpc::StatusCode::INTERNAL
            ) : _msg(msg), _error_code(error_code), _status_code(status_code), std::exception() {}
            virtual ~LmcServiceException() = default;

            /**
             * @brief Returns the error code for this exception.
             *
             */
            ska::pst::lmc::ErrorCode error_code() const { return _error_code; }

            /**
             * @brief Returns the status code for this exception.
             */
            grpc::StatusCode status_code() const { return _status_code; }

            /**
             * @brief Converts this exception into a grpc::Status object.
             *
             * gRPC expects its methods to return instances grpc::Status and
             * this method is used to convert this exception into a grpc::Status object.
             */
            grpc::Status as_grpc_status();

            /**
             * @brief Get the cause of the exception.
             */
            const char* what() const noexcept override {
                return _msg;
            }

        private:
            /**
             * @brief the message for the exception.
             */
            const char* _msg;

            /**
             * @brief the PST LMC error code for the exception.
             */
            ska::pst::lmc::ErrorCode _error_code;

            /**
             * @brief the gRPC status code for the exception.
             */
            grpc::StatusCode _status_code;

    };

} // namespace ska::pst::common


#endif // __SKA_PST_COMMON_LmcServiceException_h