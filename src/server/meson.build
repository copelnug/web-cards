
dep_thread = dependency('threads')
dep_boost_beast = dependency('boost', modules: ['coroutine', 'system'])

executable(
	'server',
	'main.cpp',
	dependencies: [dep_boost_beast, dep_thread]
)