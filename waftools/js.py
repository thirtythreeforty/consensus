import os
import itertools

from waflib.Configure import conf


@conf
def compile_js(ctx, js_path=os.path.join("src", "js")):
    js_nodes = (ctx.path.ant_glob(js_path + '/**/*.js') +
                ctx.path.ant_glob(js_path + '/**/*.json'))

    def js_task(task):
        in_js_nodes = task.inputs
        out_js_nodes = task.outputs
        # Cleanup js and comments
        uglifyjs = os.path.join('node_modules', 'uglify-js', 'bin', 'uglifyjs')
        options = '--bare-returns --reserve-domprops -m toplevel=true -c warnings=false,hoist_funs=true -r $,require,exports '

        for node_in, node_out in itertools.izip(in_js_nodes, out_js_nodes):
            ret = ctx.exec_command('%s %s %s > %s' % (uglifyjs, node_in.abspath(), options, node_out.abspath()))
            if ret != 0:
                ctx.fatal('uglifyjs failed')

    js_targets = [ctx.path.get_bld().make_node(node.path_from(ctx.path)) for node in js_nodes]

    ctx(rule=js_task,
        source=js_nodes,
        target=js_targets)

    return js_targets, next(i for i in js_targets if "src/js/app.js" in i.abspath())
