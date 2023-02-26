import websocket
import re
import build.gameSession_pb2 as gameSessionProto

def on_message(ws, message):
    response = gameSessionProto.Response()
    response.ParseFromString(message.encode('utf-8'))
    print(response)

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print(f"### closed {close_status_code} {close_msg} ###")

def on_open(ws):
    print("connected")
    request = gameSessionProto.Request()
    create_game_session_request = gameSessionProto.CreateGameSessionRequest()
    create_game_session_request.title = "test game"
    create_game_session_request.mode = gameSessionProto.GameMode.PVE
    create_game_session_request.number_of_rounds = 3
    request.create_game_session_request.CopyFrom(create_game_session_request)
    serialized = request.SerializeToString()
    ws.send(serialized)

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:8080",
                                on_message = on_message,
                                on_error = on_error,
                                on_close = on_close,
                                on_open = on_open)
    ws.run_forever()
