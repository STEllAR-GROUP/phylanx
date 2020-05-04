//  Copyright (c) 2020 Christopher Taylor
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
#include "phylanx/plugins/algorithms/impl/lda_trainer.hpp"
#include <blaze/Blaze.h>

int main() {

    blaze::DynamicMatrix<double> wd_mat(1000, 100);

    phylanx::execution_tree::primitives::impl::lda_trainer trainer{};
    trainer(wd_mat, 3, 20);

    return 1;
}
