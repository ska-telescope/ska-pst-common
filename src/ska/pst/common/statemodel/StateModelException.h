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

#include <stdexcept>
#include <string>

#ifndef __SKA_PST_COMMON_StateModelException_h
#define __SKA_PST_COMMON_StateModelException_h

namespace ska::pst::common
{

  /**
   * @brief Exception thrown when an invalid state transition occurs
   *
   */
  class pst_state_transition_error : public std::logic_error
  {
    public:

      /**
       * @brief Construct a new pst state transition error object
       *
       * @param msg error messsage describing the invalid state transition
       */
      explicit pst_state_transition_error(const std::string& msg) : logic_error(msg) {};

      /**
       * @brief Construct a new pst state transition error object
       *
       * @param msg error messsage describing the invalid state transition
       */
      explicit pst_state_transition_error(const char* msg) : logic_error(msg) {};

      /**
       * @brief Destroy the pst state transition error object
       *
       */
      virtual ~pst_state_transition_error() override = default;
  };

} // namespace ska::pst::common

#endif // __SKA_PST_COMMON_StateModelException_h