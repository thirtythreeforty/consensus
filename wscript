
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

import os.path

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

    CROSS_COMPILE_PREFIX = 'arm-none-eabi-'
    cxx = CROSS_COMPILE_PREFIX + 'g++'
    for p in ctx.env.TARGET_PLATFORMS:
        e = ctx.all_envs[p]
        ctx.env.CXX = cxx
        ctx.load('g++')

        e.CXX = cxx
        e.CXXFLAGS = list(ctx.env.CFLAGS)
        e.CXXFLAGS.remove('-std=c99')
        e.CXXFLAGS.extend(['-c', '-std=c++14', '-fPIE', '-fno-rtti', '-fno-unwind-tables', '-fno-exceptions', '-fno-threadsafe-statics', '-flto'])
        e.LINKFLAGS.extend(['-std=c++14', '-fPIE', '-fno-rtti', '-fno-unwind-tables', '-fno-exceptions', '-fno-threadsafe-statics', '-flto'])
        e.CXX_TGT_F = ['-o']
        e.LINK_CC = cxx

def build(ctx):
    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []

    for p in ctx.env.TARGET_PLATFORMS:
        ctx.set_env(ctx.all_envs[p])
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf='{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.cpp'), target=app_elf)

        if build_worker:
            worker_elf='{}/pebble-worker.elf'.format(ctx.env.BUILD_DIR)
            binaries.append({'platform': p, 'app_elf': app_elf, 'worker_elf': worker_elf})
            ctx.pbl_worker(source=ctx.path.ant_glob('src/**/*.cpp'), target=worker_elf)
        else:
            binaries.append({'platform': p, 'app_elf': app_elf})

    ctx.set_group('bundle')
    ctx.pbl_bundle(binaries=binaries, js=ctx.path.ant_glob('src/js/**/*.js'))
