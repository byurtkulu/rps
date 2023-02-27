import websocket
import time
import build.gameSession_pb2 as gameSessionProto
import google.protobuf as protobuf
from google.protobuf.message import DecodeError

def create_game_session_request(ws):
    request = gameSessionProto.Request()
    create_game_session_request = gameSessionProto.CreateGameSessionRequest()

    is_exit = input("Type \"exit\" to disconnect or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return

    title = input("Enter game session title: ")
    create_game_session_request.title = title
    mode = input("Enter game mode (PVP or PVE): ")
    if (mode.lower() == "pvp"):
        create_game_session_request.mode = gameSessionProto.GameMode.PVP
    elif (mode.lower() == "pve"):
        create_game_session_request.mode = gameSessionProto.GameMode.PVE
    else:
        print("Invalid game mode. Defaulting to PVE")
        create_game_session_request.mode = gameSessionProto.GameMode.PVE

    # create_game_session_request.title = "test"
    # create_game_session_request.mode = gameSessionProto.GameMode.PVP

    request.create_game_session_request.CopyFrom(create_game_session_request)
    return request.SerializeToString()

def on_message(ws, message):
    try:
        response = gameSessionProto.Response()
        response.ParseFromString(message)
    except DecodeError as e:
        e.__class__.use_enum_values_for_proto3 = True
        raise e

    print(response)
    ws.send(create_game_session_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print(f"### closed {close_status_code} {close_msg} ###")

def on_open(ws):
    print("connected")
    ws.send(create_game_session_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:8080",
                                on_message = on_message,
                                on_error = on_error,
                                on_close = on_close,
                                on_open = on_open)
    ws.run_forever()
