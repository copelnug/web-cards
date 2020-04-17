
dep_thread = dependency('threads')
dep_boost_beast = dependency('boost', modules: ['coroutine', 'system'])

serverlib = library('server',
	'FileRepository.cpp',
	'GlobalLobby.cpp',
	'Lobby.cpp',
	'LobbyEnfer.cpp',
	'Server.cpp',
	'UserRepository.cpp',
	'WebsocketSession.cpp',
	dependencies: [dep_cards, dep_boost_beast, dep_thread]
)
dep_server = declare_dependency(include_directories: '.', link_with : serverlib)

executable('server',
	'main.cpp',
	dependencies: [dep_server, dep_cards, dep_boost_beast, dep_thread]
)