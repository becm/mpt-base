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
  args: [ '2', '28', '29', '60', '61', '124', '125', '252', '253', '4080' ])

test('config environment import',
  executable('environ', 'environ.c',
    c_args : test_defines,
    dependencies: core),
  args: 'mpt_*')

test('config file import',
  executable('config', 'config.c',
    c_args : test_defines,
    dependencies: core))

parse = executable('parse', 'parse.c',
    c_args : test_defines,
    dependencies: core)
# see MPT_NAMEFLAG(Flags) in parse.h for name restriction configuration
test('parser test', parse,
  args : [ files('online.txt'), '{*} =;!# `', 'Ef' ])
test('layout parser', parse,
  args : [ files('layout.txt'), '[*] = ', 'Esnw' ])
test('layout parser (alternative settings)', parse,
  args : [ files('layout.txt'), '[*] = !' ])
test('section parser', parse,
  args : [ files('subsect.lay'), '{*} =;!#', 'E' ])
test('config parser', parse,
  args : [ files('config.txt'), '[ ] = #', 'Esc' ])
