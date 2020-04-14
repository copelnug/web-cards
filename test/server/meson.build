
testServer = executable('test-server',
	'testLobbyEnfer.cpp',
	dependencies: [dep_server, dep_boost_beast, dep_cards, dep_catch, dep_thread]
)
test('Server unittests', testServer)