project('bcc', 'cpp',
        version : '0.1',
        default_options : ['cpp_std=c++17'])

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_dep')

lexer   = static_library('lexer', 'src/lexer.cpp')
logger  = static_library('logger', 'src/logger.cpp')
parser  = static_library('parser', 'src/parser.cpp')
builder = static_library('Builder', 'src/Builder.cpp')
ir          = static_library('IR', 'src/IR.cpp')
irbuilder   = static_library('IRBuilder', 'src/IRBuilder.cpp')
class_graph = static_library('class_graph', 'src/class_graph.cpp')
translate   = static_library('translate', 'src/translate.cpp')
helper      = static_library('helper', 'src/helper.cpp')

front_deps = declare_dependency(link_with : 
  [lexer, logger, parser, builder])
end_deps = declare_dependency(link_with : [
                              ir, irbuilder, class_graph, translate, helper
                              ])

testing_deps = declare_dependency(
                include_directories : [
                  include_directories('src')
                ])

executable('bcc', 'src/main.cpp', dependencies : front_deps)
executable('test_parser', 'test/parsing.cpp',
           dependencies : [front_deps, testing_deps, gtest_dep])
executable('test_helper', 'test/helper.cpp',
           dependencies : [front_deps, end_deps, testing_deps, gtest_dep])
executable('test_catamorphism', 'test/catamorphism.cpp',
           dependencies : [front_deps, end_deps, testing_deps, gtest_dep])
executable('test_IR', 'test/IR.cpp',
           dependencies : [front_deps, end_deps, testing_deps, gtest_dep])
executable('test_translation', 'test/translation.cpp',
           dependencies : [front_deps, end_deps, testing_deps, gtest_dep])
run_target('all_tests', command : 'test/run_all.sh')
