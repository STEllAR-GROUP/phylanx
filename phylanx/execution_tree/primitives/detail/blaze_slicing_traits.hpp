//         ///////////////////////////////////////////////////////////////////////
//         template <typename T>
//         struct scalar_type
//         {
//             using type = T;
//         };
//
//         template <typename T>
//         struct scalar_type<blaze::DynamicVector<T>>
//         {
//             using type = T;
//         };
//
//         template <typename T, bool TF, typename RT>
//         struct scalar_type<blaze::CustomVector<T, true, true, TF, RT>>
//         {
//             using type = T;
//         };
//
//         template <typename T>
//         struct scalar_type<blaze::DynamicMatrix<T>>
//         {
//             using type = blaze::DynamicVector<T>;
//         };
//
//         template <typename T, bool SO, typename RT>
//         struct scalar_type<blaze::CustomMatrix<T, true, true, SO, RT>>
//         {
//             using type = blaze::DynamicVector<T>;
//         };
//
//         ///////////////////////////////////////////////////////////////////////
//         template <typename T>
//         struct vector_type
//         {
//             using type = T;
//         };
//
//         template <typename T>
//         struct vector_type<blaze::DynamicVector<T>>
//         {
//             using type = blaze::DynamicVector<T>;
//         };
//
//         template <typename T, bool TF, typename RT>
//         struct vector_type<blaze::CustomVector<T, true, true, TF, RT>>
//         {
//             using type = blaze::DynamicVector<T>;
//         };
//
//         template <typename T>
//         struct vector_type<blaze::DynamicMatrix<T>>
//         {
//             using type = blaze::DynamicMatrix<T>;
//         };
//
//         template <typename T, bool SO, typename RT>
//         struct vector_type<blaze::CustomMatrix<T, true, true, SO, RT>>
//         {
//             using type = blaze::DynamicMatrix<T>;
//         };
//
//         ///////////////////////////////////////////////////////////////////////
//         template <typename T>
//         std::size_t get_size(blaze::DynamicVector<T> const& data)
//         {
//             return data.size();
//         }
//
//         template <typename T, bool TF, typename RT>
//         std::size_t get_size(
//             blaze::CustomVector<T, true, true, TF, RT> const& data)
//         {
//             return data.size();
//         }
//
//         template <typename T>
//         std::size_t get_size(blaze::DynamicMatrix<T> const& data)
//         {
//             return data.rows();
//         }
//
//         template <typename T, bool SO, typename RT>
//         std::size_t get_size(
//             blaze::CustomMatrix<T, true, true, SO, RT> const& data)
//         {
//             return data.rows();
//         }
//
//         ///////////////////////////////////////////////////////////////////////
//         template <typename T>
//         T& get_element(blaze::DynamicVector<T>& data, std::size_t index)
//         {
//             return data[index];
//         }
//
//         template <typename T, bool TF, typename RT>
//         T& get_element(
//             blaze::CustomVector<T, true, true, TF, RT>& data, std::size_t index)
//         {
//             return data[index];
//         }
//
//         template <typename T>
//         auto get_element(blaze::DynamicMatrix<T>& data, std::size_t index)
//         -> decltype(blaze::row(data, index))
//         {
//             return blaze::row(data, index);
//         }
//
//         template <typename T, bool SO, typename RT>
//         auto get_element(blaze::CustomMatrix<T, true, true, SO, RT>& data,
//                 std::size_t index)
//         -> decltype(blaze::row(data, index))
//         {
//             return blaze::row(data, index);
//         }
//
//         ///////////////////////////////////////////////////////////////////////
//         template <typename T, bool TF, typename RT>
//         auto get_subvector(blaze::CustomVector<T, true, true, TF, RT>& data,
//                 std::size_t start, std::size_t count)
//         ->  decltype(blaze::subvector(data, start, count))
//         {
//             return blaze::subvector(data, start, count);
//         }
//
//         template <typename T>
//         auto get_subvector(blaze::DynamicVector<T>& data,
//                 std::size_t start, std::size_t count)
//         ->  decltype(blaze::subvector(data, start, count))
//         {
//             return blaze::subvector(data, start, count);
//         }
//
//         template <typename T, bool SO, typename RT>
//         auto get_subvector(blaze::CustomMatrix<T, true, true, SO, RT>& data,
//                 std::size_t start, std::size_t count)
//         ->  decltype(blaze::submatrix(data, start, 0, count, data.columns()))
//         {
//             return blaze::submatrix(data, start, 0, count, data.columns());
//         }
//
//         template <typename T>
//         auto get_subvector(blaze::DynamicMatrix<T>& data,
//                 std::size_t start, std::size_t count)
//         ->  decltype(blaze::submatrix(data, start, 0, count, data.columns()))
//         {
//             return blaze::submatrix(data, start, 0, count, data.columns());
//         }
//
//         ///////////////////////////////////////////////////////////////////////
//         template <typename T, bool TF, typename RT, typename IndexData>
//         auto get_elements(blaze::CustomVector<T, true, true, TF, RT>& data,
//                 IndexData const* index, std::size_t index_size)
//         ->  decltype(blaze::elements(data, index, index_size))
//         {
//             return blaze::elements(data, index, index_size);
//         }
//
//         template <typename T, typename IndexData>
//         auto get_elements(blaze::DynamicVector<T>& data,
//                 IndexData const* index, std::size_t index_size)
//         ->  decltype(blaze::elements(data, index, index_size))
//         {
//             return blaze::elements(data, index, index_size);
//         }
//
//         template <typename T, bool SO, typename RT, typename IndexData>
//         auto get_elements(blaze::CustomMatrix<T, true, true, SO, RT>& data,
//                 IndexData const* index, std::size_t index_size)
//         ->  decltype(blaze::rows(data, index, index_size))
//         {
//             return blaze::rows(data, index, index_size);
//         }
//
//         template <typename T, typename IndexData>
//         auto get_elements(blaze::DynamicMatrix<T>& data,
//                 IndexData const* index, std::size_t index_size)
//         ->  decltype(blaze::rows(data, index, index_size))
//         {
//             return blaze::rows(data, index, index_size);
//         }
