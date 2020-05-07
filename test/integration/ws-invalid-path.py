#!/usr/bin/env python3

# Need python3-websockets

import asyncio
import websockets

async def hello(uri):
    async with websockets.connect(uri) as websocket:
        pass

asyncio.get_event_loop().run_until_complete(
    hello('ws://localhost:8080/game/not-a-game'))