import websocket
import time
import build.gameSession_pb2 as gameSessionProto

def create_game_session_request():
    request = gameSessionProto.Request()
    create_game_session_request = gameSessionProto.CreateGameSessionRequest()
    title = input("Enter game session title: ")
    create_game_session_request.title = title

    mode = input("Enter game mode (PVP or PVE): ")
    if (mode == "PVP"):
        create_game_session_request.mode = gameSessionProto.GameMode.PVP
    elif (mode == "PVE"):
        create_game_session_request.mode = gameSessionProto.GameMode.PVE
    else:
        print("Invalid game mode. Defaulting to PVE")
        create_game_session_request.mode = gameSessionProto.GameMode.PVE

    request.create_game_session_request.CopyFrom(create_game_session_request)
    return request.SerializeToString()

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
    ws.send(create_game_session_request())

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:8080",
                                on_message = on_message,
                                on_error = on_error,
                                on_close = on_close,
                                on_open = on_open)
    ws.run_forever()
