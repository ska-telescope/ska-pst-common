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

#include <cstddef>
#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>      // std::ostringstream

#ifndef SKA_PST_SMRB_UTIL_AsciiHeader_h
#define SKA_PST_SMRB_UTIL_AsciiHeader_h

#define DEFAULT_HEADER_SIZE 4096

namespace ska {
namespace pst {
namespace common {

  /**
   * @brief Provides an interface to a PSRDada style ASCII Header
   * The PSRDada ASCII Header is stored in a configurable, fixed length C-style
   * string (the header). The header contains meta data stored in key/value pairs.
   * Key/value * pairs are separated by newlines, with each key and value delimited
   * by white
   * space(s).
   */
  class AsciiHeader {

    public:
      /**
       * @brief Construct a new Ascii Header object with the default header size
       *
       */
      AsciiHeader();

      /**
       * @brief Construct a new Ascii Header object with the specified header size
       *
       * @param header_size size of the header in bytes
       */
      explicit AsciiHeader(size_t header_size);

      /**
       * @brief Construct a new Ascii Header object initializing from the specified object
       *
       * @param obj AsciiHeader from which to copy the size and content
       */
      explicit AsciiHeader(const AsciiHeader &obj);

      /**
       * @brief Destroy the Ascii Header object
       *
       */
      ~AsciiHeader() = default;

      /**
       * @brief Clone the provided AsciiHeader object.
       *
       * @param obj object to be cloned
       */
      void clone(const AsciiHeader &obj);

      /**
       * @brief Clone the provided AsciiHeader object
       * Any key in the provided object which is of the form [key]_[stream] will be
       * copied into the new object as [key], discarding the _[stream]
       *
       * @param obj object to be cloned
       * @param stream stream integer to be removed
       */
      void clone_stream(const AsciiHeader &obj, unsigned stream);

      /**
       * @brief Append provided header params
       *
       * @param obj object who's params to append
       */
      void append_header(const AsciiHeader &obj);

      /**
       * @brief Return a pointer the raw header stored
       *
       * @return char* C-style string
       */
      std::string raw() const;

      /**
       * @brief Resize the internal storage of the header
       *
       * @param new_size new size of the header in bytes
       */
      void resize(size_t new_size);

      /**
       * @brief Return the size of the internal storage for the header
       *
       * @return size_t size of header storage in bytes
       */
      size_t get_header_size() const;

      /**
       * @brief Return the length of the C-string stored in the internal storage
       *
       * @return size_t
       */
      size_t get_header_length() const;

      /**
       * @brief Clear the internal storage, resetting the header to an empty string
       *
       */
      void reset() { params.clear(); };

      /**
       * @brief Set the ascii key/value pairs from a file
       *
       * @param filename path to file containing ascii hea
       */
      void load_from_file(const std::string &filename);

       /**
       * @brief Set the ascii header key/value pairs from a string
       * The string must contain newline separated key/value parirs, each of which is white space delimited
       * @param header_str n
       */
      void load_from_str(const char* header_str);

      /**
       * @brief Set the ascii header key/value pairs from a string
       * The string must contain newline separated key/value parirs, each of which is white space delimited
       * @param header_str n
       */
      void load_from_string(const std::string &header_str);

      /**
       * @brief Append and update the key/value pairs in the header string to the header
       *
       * @param header_str string with key/value pairs
       */
      void append_from_str(const char* header_str);

      /**
       * @brief Return the value of a parameter
       *
       * @tparam T type of the parameter to return [uint32_t, int32_t, uint64_t, int64_t, float, double]
       * @param search_key keyword for the parameter
       * @param val pointer to variable in which to return the value
       */
      template <typename T>
      void get(const std::string &search_key, T * val)
      {
        std::string str_val = get_val(search_key);
        std::istringstream iss(str_val);
        iss.exceptions(std::ios::failbit);
        iss >> *val;
      }

      /**
       * @brief Return the value of a parameter with const
       *
       * @tparam T type of the parameter to return [uint32_t, int32_t, uint64_t, int64_t, float, double]
       * @param search_key keyword for the parameter
       * @param val pointer to variable in which to return the value
       */
      template <typename T>
      void get(const std::string &search_key, T * val) const
      {
        std::string str_val = get_val(search_key);
        std::istringstream iss(str_val);
        iss.exceptions(std::ios::failbit);
        iss >> *val;
      }

      /**
       * @brief Set keyword/value pair in the header
       *
       * @tparam T type of the value to set
       * @param key keyword to use
       * @param val value to use
       */
      template <typename T>
      void set(const std::string &key, T val)
      {
        std::ostringstream out;
        out.precision(20);
        out << val ;
        set_val(key, out.str());
      }

      /**
       * @brief delete the keyword from the header
       *
       * @param key key of the parameter to be deleted
       */
      void del(const std::string &key);

      /**
       * @brief Report on the prescence of a keyword in the header
       *
       * @param key ker of the parameter to be report
       * @return true if the parameter exists
       * @return false if the parameter does not exist
       */
      bool has(const std::string &key) const;

      /**
       * @brief Return a list of keys in the header
       *
       * @return std::vector<std::string> keys in the header
       */
      std::vector<std::string> header_get_keys() const;

      /**
       * @brief Get the HDR_SIZE attribute of the ascii header in filename
       *
       * @param filename
       * @return size_t
       */
      static size_t get_size(const char * filename);

      /**
       * @brief Return a string representation a parameter value stored in the header.
       * Returns an empty string if not found.
       *
       * @param key keyword to search for.
       * @return std::string value of the key
       */
      std::string get_val(const std::string &key) const;

      /**
       * @brief Return a uint32 representation a parameter value stored in the header.
       *
       * This is a convenience method for using the templated get method.
       * @param key keyword to search for.
       * @return uint32 value of the key
       */
      uint32_t get_uint32(const std::string &key) const;

      /**
       * @brief Return a int32 representation a parameter value stored in the header.
       *
       * This is a convenience method for using the templated get method.
       * @param key keyword to search for.
       * @return int32 value of the key
       */
      int32_t get_int32(const std::string &key) const;

      /**
       * @brief Return a uint64 representation a parameter value stored in the header.
       *
       * This is a convenience method for using the templated get method.
       * @param key keyword to search for.
       * @return uint32 value of the key
       */
      uint64_t get_uint64(const std::string &key) const;

      /**
       * @brief Return a float representation a parameter value stored in the header.
       *
       * This is a convenience method for using the templated get method.
       * @param key keyword to search for.
       * @return uint32 value of the key
       */
      float get_float(const std::string &key) const;

      /**
       * @brief Return a double representation a parameter value stored in the header.
       *
       * This is a convenience method for using the templated get method.
       * @param key keyword to search for.
       * @return uint32 value of the key
       */
      double get_double(const std::string &key) const;

      /**
       * @brief Set a key/value pair in the AsciiHeader parameter list
       *
       * @param key key to set
       * @param val value to set
       */
      void set_val(const std::string &key, const std::string &val);

      /**
       * @brief Set the padding (number of spaces) to use when generating the raw AsciiHeader.
       *
       * @param to_pad maxmimum number of characters to pad.
       */
      void set_key_padding(uint32_t to_pad);

      /**
       * @brief Get the padding (number of spaces) that will be used when generating the raw AsciiHEader.
       *
       * @return uint32_t maximum number of characters to pad
       */
      uint32_t get_key_padding() const;

      /**
       * @brief Compute the number of bits per time sample for the data stream described by the header
       *
       * @param header parameters that describe the data stream
       * @return uint64_t number of bytes per time sample
       */
      unsigned compute_bits_per_sample() const;

      /**
       * @brief Compute the number of bytes per second for the data stream described by the header
       *
       * @param header parameters that describe the data stream
       * @return double number of bytes per second
       */
      double compute_bytes_per_second() const;

    private:

      /**
        * @brief Load the header key/val parameter in the line
        *
        * @param line space deliminted key value pair
        */
      void load_from_line(const std::string &line);

      /**
       * @brief Throw a runtime exception if the string is empty of consists only of whitespace characters
       *
       * @param str string to inspect for emptiness
       */
      void check_not_empty(const std::string &str) const;

      //! list of the key/value pair parameters
      std::vector<std::pair<std::string, std::string > > params;

      //! size of the header in bytes
      size_t header_size{0};

      //! size of the space padding to insert between the key/value pair when generating a raw string view of the parameters
      uint32_t key_padding{20};
  };

} // common
} // pst
} // ska

#endif // SKA_PST_SMRB_UTIL_AsciiHeader_h
