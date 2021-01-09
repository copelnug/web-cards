
dep_thread = dependency('threads')
# dep_fmtlib = dependency('fmt') # TODO libfmt-dev
dep_fmtlib = []
dep_boost_beast = dependency('boost', modules: ['coroutine', 'system'], version: '>=1.71')

serverlib = library('server',
	'BasicLobby.cpp',
	'FileRepository.cpp',
	'GlobalLobby.cpp',
	'Helper.cpp',
	'Lobby.cpp',
	'LobbyEnfer.cpp',
	'Server.cpp',
	'UserRepository.cpp',
	'WebsocketSession.cpp',
	dependencies: [dep_cards, dep_fmtlib, dep_boost_beast, dep_thread],
	install: true
)
dep_server = declare_dependency(include_directories: '.', link_with : serverlib)

executable('server',
	'main.cpp',
	dependencies: [dep_server, dep_cards, dep_fmtlib, dep_boost_beast, dep_thread],
	install: true
)