# mpt-base/examples/core/meson.build
# tests for MPT core library

test('COBS encoding',
  executable('coding', 'coding.c',
    c_args : test_defines,
    dependencies: core),
  args: 'cobs')

test('identifier sizes',
  executable('ident', 'ident.c',
    c_args : test_defines,
    dependencies: core),
  args: [ '11', '12', '13', '1020', '1021', '4080' ])

test('config environment import',
  executable('environ', 'environ.c',
    c_args : test_defines,
    dependencies: core),
  args: 'mpt_*')
