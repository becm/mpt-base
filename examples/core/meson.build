# mpt-base/examples/core/meson.build
# tests for MPT core library

cdir = meson.current_source_dir()

test('COBS encoding',
  executable('coding', cdir + '/coding.c',
    c_args : test_defines,
    dependencies: core),
  args: 'cobs')

test('identifier sizes',
  executable('ident', cdir + '/ident.c',
    c_args : test_defines,
    dependencies: core),
  args: [ '11', '12', '13', '1020', '1021', '4080' ])

test('config environment import',
  executable('environ', cdir + '/environ.c',
    c_args : test_defines,
    dependencies: core),
  args: 'mpt_*')
