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

#include <string>
#include <vector>

#ifndef SKA_PST_COMMON_UTIL_ValidationContext_h
#define SKA_PST_COMMON_UTIL_ValidationContext_h

namespace ska::pst::common
{
  /**
   * @brief The pst_validation_error intended to be constructed and thrown from a ValidationContext.
   *
   */
  class pst_validation_error : public std::logic_error
  {
    public:

      /**
       * @brief Construct a new pst validation error object
       *
       * @param msg error messsage describing the pst validation error
       */
      explicit pst_validation_error(const std::string& msg) : logic_error(msg) {};

      /**
       * @brief Construct a new pst validation error object
       *
       * @param msg error messsage describing the pst validation error
       */
      explicit pst_validation_error(const char* msg) : logic_error(msg) {};

      /**
       * @brief Destroy the pst validation error object
       *
       */
      ~pst_validation_error() override = default;
  };

  /**
   * @brief structure that describes a validation error and it's source.
   *
   */
  using validation_error_record_t = struct validation_error_record
  {
    //! the name of the field/key that failed validation
    std::string field_name;

    //! the value of the field that was invalid
    std::string value;

    //! the message that describes the error
    std::string message;

  };

  /**
   * @brief Provides a context to push validation errors
   *
   */
  class ValidationContext {

    public:

      /**
       * @brief Construct a new validation context
       *
       */
      ValidationContext() : errors(std::vector<validation_error_record_t>()) {}

      /**
       * @brief Destroy the Timer object
       *
       */
      ~ValidationContext() = default;

      /**
       * @brief Copy constructor for ValidationContext
       */
      ValidationContext(const ValidationContext& other) {
        copy_errors(other);
      }

      /**
       * @brief Check if empty
       *
       */
      auto is_empty() const noexcept -> bool {
        return errors.empty();
      }

      /**
       * @brief Add validation error based on a value
       *
       * @param field_name the name of the field/key that failed validation
       * @param value the value of the field that was invalid
       * @param message the message that describes the error, this could
       *   be decribing that the field is required, numeric, needs to
       *   meet a specific regular expression.
       */
      template <typename T>
      void add_validation_error(const std::string& field_name, T value, const std::string& message);

      /**
       * @brief Add required field validation error.
       *
       * This is a convenience method used when there is a required field  missing.
       *
       * @param field_name the name the field/key that failed validation
       */
      void add_missing_field_error(const std::string& field_name) {
        add_validation_error<std::string>(field_name, "<none>", "required value missing");
      }

      /**
       * @brief Add validation error based on a regular expression failing.
       *
       * @param field_name the name the field/key that failed validation
       * @param value the value of the field that was invalid
       * @param pattern the regular expression the field value should have matched to
       */
      void add_value_regex_error(const std::string& field_name, const std::string& value, const std::string& pattern) {
        add_validation_error<std::string>(field_name, value, "failed regex validation of \"" + pattern + "\"");
      }

      /**
       * @brief copy validation errors from a different context.
       *
       * This allows the use of a sub-validation context where the application can create another
       * context, add validation to it, check if that is empty or not before proceeding.
       *
       * @param other the other validation context to copy errors from.
       */
      void copy_errors(const ValidationContext& other) {
        errors.insert(errors.end(), other.errors.begin(), other.errors.end());
      }

      /**
       * @brief throw a validation error if not empty.
       *
       * If the context is empty this will do nothing. However, if there is at least
       * one validation error then this will throw a @see ska::pst::common::pst_validation_error.
       *
       * @throw ska::pst::common::pst_validation_error if there are valid error records
       */
      void throw_error_if_not_empty();

    private:

      //! precision of floating point values when converting to strings
      static constexpr uint32_t value_precision = 20;

      std::vector<validation_error_record_t> errors;

  };

} // namespace ska::pst::common

#endif // SKA_PST_COMMON_UTIL_ValidationContext_h
