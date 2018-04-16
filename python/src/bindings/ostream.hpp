//  Copyright (c) 2016-2018 Hartmut Kaiser
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_BINDING_HELPERS_OSTREAM_HPP)
#define PHYLANX_BINDING_HELPERS_OSTREAM_HPP

#include <phylanx/config.hpp>

#include <pybind11/pybind11.h>

#include <cstddef>
#include <iostream>
#include <utility>

namespace phylanx { namespace bindings
{
    class pythonbuf : public std::streambuf
    {
    private:
        using traits_type = std::streambuf::traits_type;

        char d_buffer[1024];
        pybind11::object pywrite;
        pybind11::object pyflush;

        int overflow(int c) override
        {
            if (!traits_type::eq_int_type(c, traits_type::eof()))
            {
                *pptr() = traits_type::to_char_type(c);
                pbump(1);
            }
            return sync() ? traits_type::not_eof(c) : traits_type::eof();
        }

        int sync() override
        {
            if (pbase() != pptr())
            {
                // acquire GIL to avoid multi-threading problems
                pybind11::gil_scoped_acquire acquire;

                // This subtraction cannot be negative, so dropping the sign
                pybind11::str line(
                    pbase(), static_cast<std::size_t>(pptr() - pbase()));

                pywrite(line);
                pyflush();

                setp(pbase(), epptr());
            }
            return 0;
        }

    public:
        pythonbuf(pybind11::object pyostream)
        {
            {
                // acquire GIL to avoid multi-threading problems
                pybind11::gil_scoped_acquire acquire;
                pybind11::object tmp = std::move(pyostream);
                pywrite = tmp.attr("write");
                pyflush = tmp.attr("flush");
            }

            setp(d_buffer, d_buffer + sizeof(d_buffer) - 1);
        }

        /// Sync before destroy
        ~pythonbuf()
        {
            sync();
        }
    };

    class scoped_ostream_redirect
    {
    protected:
        std::streambuf* old;
        std::ostream& costream;
        pythonbuf buffer;

        static pybind11::object import_stdout()
        {
            pybind11::gil_scoped_acquire acquire;
            return pybind11::module::import("sys").attr("stdout");
        }

    public:
        scoped_ostream_redirect(std::ostream& costream = std::cout,
                pybind11::object pyostream = import_stdout())
          : costream(costream)
          , buffer(std::move(pyostream))
        {
            old = costream.rdbuf(&buffer);
        }

        ~scoped_ostream_redirect()
        {
            costream.rdbuf(old);
        }

        scoped_ostream_redirect(const scoped_ostream_redirect&) = delete;
        scoped_ostream_redirect(scoped_ostream_redirect&& other) = default;
        scoped_ostream_redirect& operator=(
            const scoped_ostream_redirect&) = delete;
        scoped_ostream_redirect& operator=(scoped_ostream_redirect&&) = delete;
    };
}}

#endif

