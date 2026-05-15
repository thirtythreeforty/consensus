from waflib.TaskGen import after_method, before_method, feature

@after_method('process_source')
@feature('cxx')
def fix_pebble_h_dependencies(task_gen):
    # This bit of apparent nonsense is mostly copied from Pebble's SDK.  It
    # injects an additional dependency on the `resource_ids.auto.h` file into
    # all the *.c files that include `pebble.h`.  I'm not entirely sure why
    # they don't simply #include the resource header in pebble.h, but we need
    # to replicate this functionality for C++ files.
    import types
    from waflib.Tools import cxx, c_preproc

    def wrap_c_preproc_scan(task):
        def get_node_from_abspath(ctx, path):
            return ctx.root.make_node(path)

        (nodes, names) = c_preproc.scan(task)
        if 'pebble.h' in names:
            nodes.append(get_node_from_abspath(task.generator.bld, task.env.RESOURCE_ID_HEADER))
        return (nodes, names)

    for task in task_gen.tasks:
        if type(task) == cxx.cxx:
            task.scan = types.MethodType(wrap_c_preproc_scan, task)

@feature('c', 'cxx')
@before_method('apply_incpaths')
def prepend_local_override_includes(task_gen):
    """Put local waftools/include/ before SDK include/ so we can override SDK headers
    like pebble_warn_unsupported_functions.h without modifying the SDK."""
    includes = task_gen.to_list(getattr(task_gen, 'includes', []))
    task_gen.includes = ['src/include'] + includes

def configure(ctx):
    CROSS_COMPILE_PREFIX = 'arm-none-eabi-'
    cxx = CROSS_COMPILE_PREFIX + 'g++'
    for p in ctx.env.TARGET_PLATFORMS:
        e = ctx.all_envs[p]
        ctx.env.CXX = cxx
        ctx.load('g++')

        e.CXX = cxx
        e.CXXFLAGS = list(e.CFLAGS)
        e.CXXFLAGS.remove('-std=c99')
        e.CXXFLAGS.extend(['-c', '-std=c++20', '-fPIE', '-fno-rtti', '-fno-use-cxa-atexit', '-fno-unwind-tables', '-fno-exceptions', '-fno-threadsafe-statics', '-flto'])
        e.LINKFLAGS.extend(['-std=c++20', '-fPIE', '-fno-rtti', '-fno-use-cxa-atexit', '-fno-unwind-tables', '-fno-exceptions', '-fno-threadsafe-statics', '--specs=nano.specs', '--specs=nosys.specs', '-flto'])
        e.CXX_TGT_F = ['-o']
        e.LINK_CC = cxx
