//  Copyright (c) 2019 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <phylanx/phylanx.hpp>

#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include <bindings/binding_helpers.hpp>
#include <bindings/type_casters.hpp>
#include <bindings/variable.hpp>

#include <hpx/util/assert.hpp>
#include <hpx/util/format.hpp>

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include <utility>

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    variable::variable(primitive value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint)
      : dtype_(std::move(dtype))
      , value_(std::move(value))
      , name_(name.is_none() ?
              hpx::util::format("variable_{}", ++variable_count) :
              name.cast<std::string>())
      , constraint_(std::move(constraint))
    {
    }

    variable::variable(pybind11::array value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint)
      : dtype_(dtype.is_none() ? value.dtype() : std::move(dtype))
      , value_(create_variable(
          pybind11::cast<primitive_argument_type>(std::move(value))))
      , name_(name.is_none() ?
              hpx::util::format("variable_{}", ++variable_count) :
              name.cast<std::string>())
      , constraint_(std::move(constraint))
  {}

    variable::variable(std::string value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint)
      : dtype_(dtype.is_none() ? pybind11::dtype("S") : std::move(dtype))
      , value_(create_variable(primitive_argument_type(std::move(value))))
      , name_(name.is_none() ?
              hpx::util::format("variable_{}", ++variable_count) :
              name.cast<std::string>())
      , constraint_(std::move(constraint))
    {}

    variable::variable(primitive_argument_type value, pybind11::object dtype,
            pybind11::object name, pybind11::object constraint)
      : dtype_(std::move(dtype))
      , value_(is_primitive_operand(value) ?
              primitive_operand(std::move(value)) :
              create_variable(std::move(value)))
      , name_(name.is_none() ?
              hpx::util::format("variable_{}", ++variable_count) :
              name.cast<std::string>())
      , constraint_(std::move(constraint))
    {}

    ///////////////////////////////////////////////////////////////////////////
    primitive variable::create_variable(primitive_argument_type&& value)
    {
        return create_primitive_component(
            hpx::find_here(), "variable", std::move(value));
    }

    std::size_t variable::variable_count = 0;

    ///////////////////////////////////////////////////////////////////////////
    pybind11::dtype variable::dtype() const
    {
        if (dtype_.is_none())
        {
            return bindings::extract_dtype(value_);
        }
        return dtype_;
    }

    ///////////////////////////////////////////////////////////////////////////
    namespace detail
    {
        template <typename T>
        pybind11::handle convert_array(primitive_argument_type&& result)
        {
            pybind11::array_t<T> arr = pybind11::cast(std::move(result),
                pybind11::return_value_policy::move);
            return arr.release();
        }
    }

    pybind11::handle variable::handle_return_f(
        primitive_argument_type&& result, pybind11::ssize_t itemsize) const
    {
        if (itemsize == 4)
        {
            return detail::convert_array<float>(std::move(result));
        }

        HPX_ASSERT(itemsize == 8);
        return detail::convert_array<double>(std::move(result));
    }

    pybind11::handle variable::handle_return_i(
        primitive_argument_type&& result, pybind11::ssize_t itemsize) const
    {
        if (itemsize == 2)
        {
            return detail::convert_array<std::int16_t>(std::move(result));
        }
        else if (itemsize == 4)
        {
            return detail::convert_array<std::int32_t>(std::move(result));
        }

        HPX_ASSERT(itemsize == 8);
        return detail::convert_array<std::int64_t>(std::move(result));
    }

    pybind11::handle variable::handle_return_u(
        primitive_argument_type&& result, pybind11::ssize_t itemsize) const
    {
        if (itemsize == 2)
        {
            return detail::convert_array<std::uint16_t>(std::move(result));
        }
        else if (itemsize == 4)
        {
            return detail::convert_array<std::uint32_t>(std::move(result));
        }

        HPX_ASSERT(itemsize == 8);
        return detail::convert_array<std::uint64_t>(std::move(result));
    }

    pybind11::handle variable::handle_return_b(
        primitive_argument_type&& result) const
    {
        pybind11::bool_ b = pybind11::cast(
            std::move(result), pybind11::return_value_policy::move);
        return b.release();
    }

    pybind11::handle variable::handle_return_S(
        primitive_argument_type&& result) const
    {
        pybind11::str s = pybind11::cast(
            std::move(result), pybind11::return_value_policy::move);
        return s.release();
    }

    ///////////////////////////////////////////////////////////////////////////
    pybind11::handle variable::eval(pybind11::args args) const
    {
        phylanx::execution_tree::primitive_arguments_type keep_alive;
        keep_alive.reserve(args.size());
        phylanx::execution_tree::primitive_arguments_type fargs;
        fargs.reserve(args.size());

        {
            pybind11::gil_scoped_acquire acquire;
            for (auto const& item : args)
            {
                using phylanx::execution_tree::primitive_argument_type;

                primitive_argument_type value =
                    item.cast<primitive_argument_type>();

                keep_alive.emplace_back(std::move(value));
                fargs.emplace_back(extract_ref_value(keep_alive.back()));
            }
        }

        primitive_argument_type result =
            value_operand_sync(value_, std::move(fargs));

        // re-acquire GIL
        pybind11::gil_scoped_acquire acquire;

        // access dtype of result, if necessary
        if (!dtype_.is_none())
        {
            switch (dtype_.kind())
            {
            case 'b':   // boolean
                if (!is_boolean_operand_strict(result))
                {
                    return handle_return_b(std::move(result));
                }
                break;

            case 'i':   // signed integer
                if (!is_integer_operand_strict(result) || dtype_.itemsize() != 8)
                {
                    return handle_return_i(std::move(result), dtype_.itemsize());
                }
                break;

            case 'u':   // unsigned integer
                return handle_return_u(std::move(result), dtype_.itemsize());

            case 'f':   // floating-point
                if (!is_numeric_operand_strict(result) || dtype_.itemsize() != 8)
                {
                    return handle_return_f(std::move(result), dtype_.itemsize());
                }
                break;

            case 'O':   // object
                break;

            case 'S':   // (byte-)string
                if (!is_string_operand_strict(result))
                {
                    return handle_return_S(std::move(result));
                }
                break;

            case 'c': HPX_FALLTHROUGH;  // complex floating-point
            case 'm': HPX_FALLTHROUGH;  // timedelta
            case 'M': HPX_FALLTHROUGH;  // datetime
            case 'U': HPX_FALLTHROUGH;  // Unicode
            case 'V':                   // void
                {
                    HPX_THROW_EXCEPTION(hpx::bad_parameter,
                        "variable::eval",
                        hpx::util::format("unsupported dtype: {}", dtype_.kind()));
                }
                break;
            }
        }

        pybind11::object h = pybind11::cast(std::move(result),
            pybind11::return_value_policy::move);
        return h.release();
    }
}}
