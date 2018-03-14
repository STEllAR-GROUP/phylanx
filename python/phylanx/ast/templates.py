oscop_global = \
"""<OpenScop>

# =============================================== Global
# Backend Language
C

# Context
$domain_t
$nrows $ncols 0 0 0 1
$context

# Parameter names are provided
$params_exist
# Parameter names
$params_names

# Number of statements
$num_statements
"""  # noqa: E122

oscop_statment = \
"""# =============================================== Statement $statement_num
# Number of relations describing the statement
$num_relations

# ----------------------------------------------  $statement_num.1 Domain
$domain_t
$domain

# ----------------------------------------------  $statement_num.2 Scattering
$scattering

# ----------------------------------------------  $statement_num.3 Access
$access

"""  # noqa: E122


def empty_oscop_statment():
    return dict(
        domain_t='',
        statement_num='',
        num_relations=0,
        domain='',
        scattering='',
        access='')


def empty_oscop_global():
    return dict(
        domain_t='',
        context='',
        nrows=0,
        ncols=0,
        params_exist='',
        params_names='',
        num_statements='')
