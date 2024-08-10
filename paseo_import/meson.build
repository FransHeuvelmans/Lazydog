project('paseo_import', 'c', version: '0.2.0a', default_options : ['c_std=c17'])
add_project_arguments('-Wall', '-Wextra', '-Wno-poison-system-directories', '-Wno-declaration-after-statement', language : 'c')

srcs = [
    'main.c',
    'db_ops.c',
    'db_ops.h',
    'samples.c',
    'samples.h',
    'tsv.c',
    'tsv.h',
]

deps = [
    dependency('sqlite3')
]

executable('paseo_import', srcs, dependencies: deps, install: true)

test_deps =  [
    dependency('cmocka')
]
t2 = executable('db_test', 'db_ops_test.c', 'db_ops.h', 'db_ops.c', 'samples.c', 'samples.h', dependencies : deps + test_deps, c_args: '-fsanitize=undefined,address', link_args : '-fsanitize=undefined,address')
test('db_ops_test', t2)
t3 = executable('samples_test', 'samples_test.c', 'samples.h', 'samples.c', dependencies : deps + test_deps, c_args: '-fsanitize=undefined,address', link_args : '-fsanitize=undefined,address')
test('samples_test', t3)
t4 = executable('tsv_test', 'tsv_test.c', 'tsv.c', 'tsv.h', 'samples.h', 'samples.c', dependencies : deps + test_deps, c_args: '-fsanitize=undefined,address', link_args : '-fsanitize=undefined,address')
test('tsv_test', t4)