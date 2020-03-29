project('web-cards', 'cpp', default_options : ['cpp_std=c++2a'])

# Asio will ensure the function keep working: https://github.com/boostorg/beast/issues/1262
add_global_arguments('-DBOOST_COROUTINES_NO_DEPRECATION_WARNING', language : 'cpp')

subdir('src/server')
subdir('test/catch')
subdir('test/server')