/*
 * Copyright 2023 Square Kilometre Array Observatory
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

#include <string>

#include "ska/pst/common/utils/DataWeightsBlockLoader.h"

#ifndef __SKA_PST_COMMON_UTILS_DataWeightsFileBlockLoader_h
#define __SKA_PST_COMMON_UTILS_DataWeightsFileBlockLoader_h

namespace ska::pst::common {

  /**
   * @brief Class used for reading voltage data and weights from file.
   *
   */
  class DataWeightsFileBlockLoader : public DataWeightsBlockLoader
  {
    public:
      /**
       * @brief Create instance of a DataWeightsFileBlockLoader object.
       *
       * @param data_file_path path to the data file to process.
       * @param weights_file_path the path to the weights file for the data file.
       */
      DataWeightsFileBlockLoader(
        const std::string& data_file_path,
        const std::string& weights_file_path
      );

      /**
       * @brief Destroy the DataWeightsFileBlockLoader object.
       *
       */
      ~DataWeightsFileBlockLoader();
  };

} // namespace ska::pst::common

#endif // __SKA_PST_COMMON_UTILS_DataWeightsFileBlockLoader_h
