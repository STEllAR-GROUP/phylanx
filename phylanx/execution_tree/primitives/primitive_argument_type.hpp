// Copyright (c) 2017-2019 Hartmut Kaiser
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(PHYLANX_PRIMITIVES_ARGUMENT_TYPE_JUL_23_2018_0149PM)
#define PHYLANX_PRIMITIVES_ARGUMENT_TYPE_JUL_23_2018_0149PM

#include <phylanx/config.hpp>
#include <phylanx/ast/node.hpp>
#include <phylanx/util/hashed_string.hpp>
#include <phylanx/util/variant.hpp>
#include <phylanx/ir/dictionary.hpp>
#include <phylanx/ir/node_data.hpp>
#include <phylanx/ir/ranges.hpp>
#include <phylanx/util/variant.hpp>

#include <hpx/include/lcos.hpp>
#include <hpx/include/runtime.hpp>
#include <hpx/runtime/serialization/serialization_fwd.hpp>
#include <hpx/runtime/serialization/serialization_fwd.hpp>
#include <hpx/util/assert.hpp>
#include <hpx/util/internal_allocator.hpp>

#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <blaze/Math.h>
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
#include <blaze_tensor/Math.h>
#endif

namespace phylanx { namespace execution_tree
{
    ///////////////////////////////////////////////////////////////////////////
    namespace primitives
    {
        class primitive_component;
    }

    ///////////////////////////////////////////////////////////////////////////
    class primitive;
    struct primitive_argument_type;

    ///////////////////////////////////////////////////////////////////////////
    struct topology
    {
        topology() = default;

        topology(std::string name)
          : name_(std::move(name))
        {}

        topology(std::vector<topology> names)
          : children_(std::move(names))
        {}

        topology(std::vector<topology> names, std::string name)
          : children_(std::move(names)), name_(std::move(name))
        {}

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);

    public:
        std::vector<topology> children_;
        std::string name_;
    };

    ///////////////////////////////////////////////////////////////////////////
    // Generate Newick tree (string) format from given tree topology
    PHYLANX_EXPORT std::string newick_tree(
        std::string const& name, topology const& t);

    // Generate Dot tree (string) format from given tree topology
    PHYLANX_EXPORT std::string dot_tree(
        std::string const& name, topology const& t);

    PHYLANX_EXPORT bool is_primitive_operand(
        primitive_argument_type const& val);

    ///////////////////////////////////////////////////////////////////////////
    class variable_frame
    {
        using allocator_type = hpx::util::internal_allocator<
            std::pair<util::hashed_string const, primitive_argument_type>>;
        using variables_map_type = std::map<util::hashed_string,
            primitive_argument_type, std::less<>, allocator_type>;

    public:
        variable_frame() = default;

        variable_frame(std::shared_ptr<variable_frame> nextframe) noexcept
          : nextframe_(std::move(nextframe))
        {
        }

        inline std::size_t size() const noexcept
        {
            return variables_.size();
        }
        inline bool empty() const noexcept
        {
            return variables_.empty();
        }

        inline primitive_argument_type* get_var(
            util::hashed_string const& name) noexcept;
        inline primitive_argument_type const* get_var(
            util::hashed_string const& name) const noexcept;
        inline primitive_argument_type& set_var(
            util::hashed_string const& name, primitive_argument_type&& var);

    private:
        friend class hpx::serialization::access;
        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);

    private:
        variables_map_type variables_;
        std::shared_ptr<variable_frame> nextframe_;
    };

    ///////////////////////////////////////////////////////////////////////////
    enum eval_mode
    {
        eval_default = 0x00,                // always evaluate everything
        eval_dont_wrap_functions = 0x01,    // don't wrap partially bound functions
        eval_dont_evaluate_partials = 0x02, // don't evaluate partially bound functions
        eval_dont_evaluate_lambdas = 0x04,  // don't evaluate functions
        eval_slicing = 0x08                 // do perform slicing
    };

    struct eval_context
    {
        enum initmode { noinit = 0 };

        explicit eval_context(initmode) noexcept
          : mode_(eval_default)
          , variables_()
        {
        }

        explicit eval_context(eval_mode mode = eval_default)
          : mode_(mode)
          , variables_(std::allocate_shared<variable_frame>(alloc_))
        {}

        eval_context& set_mode(eval_mode mode) noexcept
        {
            mode_ = mode;
            return *this;
        }
        eval_context& add_mode(eval_mode mode) noexcept
        {
            mode_ = eval_mode(mode_ | mode);
            return *this;
        }
        eval_context& remove_mode(eval_mode mode) noexcept
        {
            mode_ = eval_mode(mode_ & ~mode);
            return *this;
        }

        primitive_argument_type* get_var(
            util::hashed_string const& name) noexcept
        {
            HPX_ASSERT(bool(variables_));
            return variables_->get_var(name);
        }
        primitive_argument_type const* get_var(
            util::hashed_string const& name) const noexcept
        {
            HPX_ASSERT(bool(variables_));
            return variables_->get_var(name);
        }

        inline primitive_argument_type& set_var(util::hashed_string const& name,
            primitive_argument_type&& var);

        eval_context& add_frame()
        {
            if (variables_ && !variables_->empty())
            {
                variables_ = std::allocate_shared<variable_frame>(
                    alloc_, std::move(variables_));
            }
            return *this;
        }

        explicit operator bool() const noexcept
        {
            return bool(variables_);
        }

    private:
        friend class hpx::serialization::access;
        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);

    public:
        eval_mode mode_;
        std::shared_ptr<variable_frame> variables_;

        PHYLANX_EXPORT static
            hpx::util::internal_allocator<variable_frame> alloc_;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline eval_context set_mode(eval_context const& ctx, eval_mode mode)
    {
        eval_context newctx = ctx;
        newctx.set_mode(mode);
        return newctx;
    }
    inline eval_context set_mode(eval_context && ctx, eval_mode mode) noexcept
    {
        eval_context newctx = std::move(ctx);
        newctx.set_mode(mode);
        return newctx;
    }

    inline eval_context add_mode(eval_context const& ctx, eval_mode mode)
    {
        eval_context newctx = ctx;
        newctx.add_mode(mode);
        return newctx;
    }
    inline eval_context add_mode(eval_context && ctx, eval_mode mode) noexcept
    {
        eval_context newctx = std::move(ctx);
        newctx.add_mode(mode);
        return newctx;
    }

    inline eval_context remove_mode(eval_context const& ctx, eval_mode mode)
    {
        eval_context newctx = ctx;
        newctx.remove_mode(mode);
        return newctx;
    }
    inline eval_context remove_mode(eval_context && ctx, eval_mode mode) noexcept
    {
        eval_context newctx = std::move(ctx);
        newctx.remove_mode(mode);
        return newctx;
    }

    inline eval_context add_frame(eval_context && ctx)
    {
        eval_context newctx = std::move(ctx);
        newctx.add_frame();
        return newctx;
    }

    ///////////////////////////////////////////////////////////////////////////
    class primitive
      : public hpx::components::client_base<primitive,
            primitives::primitive_component>
    {
    private:
        using base_type = hpx::components::client_base<primitive,
            primitives::primitive_component>;

    public:
        primitive() = default;

        primitive(hpx::id_type && id)
          : base_type(std::move(id))
        {
        }
        primitive(hpx::future<hpx::id_type> && fid)
          : base_type(std::move(fid))
        {
        }

        PHYLANX_EXPORT primitive(hpx::future<hpx::id_type>&& fid,
            std::string const& name, bool register_with_agas = true);

        primitive(primitive const&) = default;
        primitive(primitive &&) = default;

        primitive& operator=(primitive const&) = default;
        primitive& operator=(primitive &&) = default;

        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            eval_context ctx = eval_context{}) const;
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            primitive_argument_type && arg,
            eval_context ctx = eval_context{}) const;
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            primitive_arguments_type && args,
            eval_context ctx = eval_context{}) const;
        PHYLANX_EXPORT hpx::future<primitive_argument_type> eval(
            primitive_arguments_type const& args,
            eval_context ctx = eval_context{}) const;

        PHYLANX_EXPORT primitive_argument_type eval(hpx::launch::sync_policy,
            eval_context ctx = eval_context{}) const;
        PHYLANX_EXPORT primitive_argument_type eval(hpx::launch::sync_policy,
            primitive_argument_type && arg,
            eval_context ctx = eval_context{}) const;
        PHYLANX_EXPORT primitive_argument_type eval(hpx::launch::sync_policy,
            primitive_arguments_type&& args,
            eval_context ctx = eval_context{}) const;
        PHYLANX_EXPORT primitive_argument_type eval(hpx::launch::sync_policy,
            primitive_arguments_type const& args,
            eval_context ctx = eval_context{}) const;

        PHYLANX_EXPORT hpx::future<void> store(primitive_argument_type&&,
            primitive_arguments_type&&, eval_context ctx = eval_context{});
        PHYLANX_EXPORT hpx::future<void> store(primitive_arguments_type&&,
            primitive_arguments_type&&, eval_context ctx = eval_context{});
        PHYLANX_EXPORT void store(hpx::launch::sync_policy,
            primitive_argument_type&&, primitive_arguments_type&&,
            eval_context ctx = eval_context{});
        PHYLANX_EXPORT void store(hpx::launch::sync_policy,
            primitive_arguments_type&&, primitive_arguments_type&&,
            eval_context ctx = eval_context{});

        PHYLANX_EXPORT hpx::future<topology> expression_topology(
            std::set<std::string>&& functions) const;
        PHYLANX_EXPORT topology expression_topology(hpx::launch::sync_policy,
            std::set<std::string>&& functions) const;

        PHYLANX_EXPORT hpx::future<topology> expression_topology(
            std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const;
        PHYLANX_EXPORT topology expression_topology(hpx::launch::sync_policy,
            std::set<std::string>&& functions,
            std::set<std::string>&& resolve_children) const;

        PHYLANX_EXPORT bool bind(
            primitive_arguments_type&& args, eval_context ctx) const;
        PHYLANX_EXPORT bool bind(
            primitive_arguments_type const& args, eval_context ctx) const;

    public:
        static bool enable_tracing;
    };

    ///////////////////////////////////////////////////////////////////////////
    using argument_value_type =
        phylanx::util::variant<
            ast::nil
          , ir::node_data<std::uint8_t>
          , ir::node_data<std::int64_t>
          , std::string
          , ir::node_data<double>
          , primitive
          , std::vector<ast::expression>
          , ir::range
          , phylanx::ir::dictionary
        >;

    PHYLANX_EXPORT primitive_argument_type extract_copy_value(
        primitive_argument_type const& val,
        std::string const& name = "",
        std::string const& codename = "<unknown>");

    struct primitive_argument_type : argument_value_type
    {
        enum variant_index
        {
            nil_index = 0,
            bool_index = 1,
            int64_index = 2,
            string_index = 3,
            float64_index = 4,
            primitive_index = 5,
            expression_index = 6,
            list_index = 7,
            dictionary_index = 8
        };

        primitive_argument_type() = default;

        primitive_argument_type(ast::nil val)
          : argument_value_type{val}
        {}

        explicit primitive_argument_type(bool val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(std::uint8_t val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(
                blaze::DynamicVector<std::uint8_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicVector<std::uint8_t>&& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{std::move(val)}}
        {}
        explicit primitive_argument_type(
                blaze::DynamicMatrix<std::uint8_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicMatrix<std::uint8_t>&& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{std::move(val)}}
        {}
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        explicit primitive_argument_type(
                blaze::DynamicTensor<std::uint8_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicTensor<std::uint8_t>&& val)
          : argument_value_type{phylanx::ir::node_data<std::uint8_t>{std::move(val)}}
        {}
#endif

        primitive_argument_type(phylanx::ir::node_data<std::uint8_t> const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(phylanx::ir::node_data<std::uint8_t>&& val)
          : argument_value_type{std::move(val)}
        {}

        explicit primitive_argument_type(std::int64_t val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{val}}
        {
        }
        explicit primitive_argument_type(
                blaze::DynamicVector<std::int64_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{val}}
        {
        }
        explicit primitive_argument_type(
                blaze::DynamicVector<std::int64_t>&& val)
          : argument_value_type{
                phylanx::ir::node_data<std::int64_t>{std::move(val)}}
        {
        }
        explicit primitive_argument_type(
                blaze::DynamicMatrix<std::int64_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{val}}
        {
        }
        explicit primitive_argument_type(
                blaze::DynamicMatrix<std::int64_t>&& val)
          : argument_value_type{
                phylanx::ir::node_data<std::int64_t>{std::move(val)}}
        {
        }
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        explicit primitive_argument_type(
                blaze::DynamicTensor<std::int64_t> const& val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicTensor<std::int64_t>&& val)
          : argument_value_type{phylanx::ir::node_data<std::int64_t>{std::move(val)}}
        {}
#endif

        primitive_argument_type(phylanx::ir::node_data<std::int64_t> const& val)
          : argument_value_type{val}
        {
        }
        primitive_argument_type(phylanx::ir::node_data<std::int64_t>&& val)
          : argument_value_type{std::move(val)}
        {
        }

        primitive_argument_type(char const* val)
          : argument_value_type{std::string(val)}
        {}
        primitive_argument_type(std::string const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(std::string && val)
          : argument_value_type{std::move(val)}
        {}

        explicit primitive_argument_type(double val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicVector<double> const& val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicVector<double>&& val)
          : argument_value_type{phylanx::ir::node_data<double>{std::move(val)}}
        {}
        explicit primitive_argument_type(blaze::DynamicMatrix<double> const& val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicMatrix<double>&& val)
          : argument_value_type{phylanx::ir::node_data<double>{std::move(val)}}
        {}
#if defined(PHYLANX_HAVE_BLAZE_TENSOR)
        explicit primitive_argument_type(blaze::DynamicTensor<double> const& val)
          : argument_value_type{phylanx::ir::node_data<double>{val}}
        {}
        explicit primitive_argument_type(blaze::DynamicTensor<double>&& val)
          : argument_value_type{phylanx::ir::node_data<double>{std::move(val)}}
        {}
#endif

        primitive_argument_type(phylanx::ir::node_data<double> const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(phylanx::ir::node_data<double>&& val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(primitive const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(primitive && val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(std::vector<ast::expression> const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(std::vector<ast::expression>&& val)
          : argument_value_type{std::move(val)}
        {}

        explicit primitive_argument_type(
            primitive_arguments_type const& val)
          : argument_value_type{ir::range{val}}
        {}

        explicit primitive_argument_type(
            primitive_arguments_type&& val)
          : argument_value_type{ir::range{std::move(val)}}
        {}

        primitive_argument_type(ir::range const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(ir::range&& val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(ir::dictionary const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(ir::dictionary&& val)
          : argument_value_type{std::move(val)}
        {}

        primitive_argument_type(argument_value_type const& val)
          : argument_value_type{val}
        {}
        primitive_argument_type(argument_value_type&& val)
          : argument_value_type{std::move(val)}
        {}

        inline primitive_argument_type run(
            eval_context ctx = eval_context{}) const;

        PHYLANX_EXPORT primitive_argument_type operator()(
            eval_context ctx = eval_context{}) const;

        PHYLANX_EXPORT primitive_argument_type operator()(
            primitive_arguments_type const& args,
            eval_context ctx = eval_context{}) const;

        PHYLANX_EXPORT primitive_argument_type operator()(
            primitive_arguments_type&& args,
            eval_context ctx = eval_context{}) const;

        template <typename T1, typename... Ts>
        typename std::enable_if<
           !std::is_same<eval_context, typename std::decay<T1>::type>::value,
            primitive_argument_type
        >::type
        operator()(T1&& t1, Ts&&... ts) const
        {
            primitive_arguments_type args;
            args.reserve(sizeof...(Ts));

            int const sequencer_[] = {
                (args.emplace_back(
                    extract_copy_value(primitive_argument_type{
                        std::forward<T1>(t1)
                    })), 0)
              , (args.emplace_back(
                    extract_copy_value(primitive_argument_type{
                        std::forward<Ts>(ts)
                    })), 0)...
            };
            (void)sequencer_;

            return (*this)(std::move(args));
        }

        template <typename ... Ts>
        primitive_argument_type operator()(eval_context ctx, Ts &&... ts) const
        {
            primitive_arguments_type args;
            args.reserve(sizeof...(Ts));

            int const sequencer_[] = {
                0, (args.emplace_back(
                        extract_copy_value(primitive_argument_type{
                            std::forward<Ts>(ts)
                        })), 0)...
            };
            (void)sequencer_;

            return (*this)(std::move(args), std::move(ctx));
        }

        // a primitive_argument_type instance is valid if its not an implicit nil
        explicit operator bool() const noexcept
        {
            return variant().index() != 0;
        }

        // a primitive_argument_type could be an explicit nil
        bool is_explicit_nil() const noexcept
        {
            return variant().index() == 0 &&
                util::get<0>(variant()).explicit_nil;
        }

        bool is_implicit_nil() const noexcept
        {
            return variant().index() == 0 &&
                !util::get<0>(variant()).explicit_nil;
        }

        // workaround for problem in implementation of MSVC14.12
        // variant::visit
        argument_value_type& variant() noexcept { return *this; }
        argument_value_type const& variant() const noexcept { return *this; }

    private:
        friend class hpx::serialization::access;

        PHYLANX_EXPORT void serialize(hpx::serialization::output_archive& ar,
            unsigned);
        PHYLANX_EXPORT void serialize(hpx::serialization::input_archive& ar,
            unsigned);
    };

    // a argument is valid of its not nil{}
    inline bool valid(primitive_argument_type const& val) noexcept
    {
        return bool(val);
    }
    inline bool valid(primitive_argument_type && val) noexcept
    {
        return bool(val);
    }

    inline bool is_explicit_nil(primitive_argument_type const& val) noexcept
    {
        return val.is_explicit_nil();
    }
    inline bool is_explicit_nil(primitive_argument_type && val) noexcept
    {
        return val.is_explicit_nil();
    }

    inline bool is_implicit_nil(primitive_argument_type const& val) noexcept
    {
        return val.is_implicit_nil();
    }
    inline bool is_implicit_nil(primitive_argument_type && val) noexcept
    {
        return val.is_implicit_nil();
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool operator==(primitive_argument_type const& lhs,
        primitive_argument_type const& rhs)
    {
        return lhs.variant() == rhs.variant();
    }
    inline bool operator!=(primitive_argument_type const& lhs,
        primitive_argument_type const& rhs)
    {
        return !(lhs == rhs);
    }

    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& os,
        primitive_argument_type const&);

    PHYLANX_EXPORT std::string to_string(primitive_argument_type const&);

    PHYLANX_EXPORT std::ostream& operator<<(std::ostream& os,
        primitive const&);

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type& eval_context::set_var(
        util::hashed_string const& name, primitive_argument_type&& var)
    {
        return variables_->set_var(name, std::move(var));
    }

    ///////////////////////////////////////////////////////////////////////////
    primitive_argument_type* variable_frame::get_var(
        util::hashed_string const& name) noexcept
    {
        auto it = variables_.find(name);
        if (it == variables_.end())
        {
            if (!nextframe_)
            {
                return nullptr;
            }
            return nextframe_->get_var(name);
        }
        return &it->second;
    }

    primitive_argument_type const* variable_frame::get_var(
        util::hashed_string const& name) const noexcept
    {
        auto it = variables_.find(name);
        if (it == variables_.end())
        {
            if (!nextframe_)
            {
                return nullptr;
            }
            return nextframe_->get_var(name);
        }
        return &it->second;
    }

    primitive_argument_type& variable_frame::set_var(
        util::hashed_string const& name, primitive_argument_type&& var)
    {
        // variables are always created in the currently top-most environment
        auto it = variables_.find(name);
        if (it == variables_.end())
        {
            using value_type = variables_map_type::value_type;
            it = variables_.insert(value_type(name, std::move(var))).first;
        }
        else
        {
            it->second = std::move(var);
        }
        return it->second;
    }
}}

#endif
