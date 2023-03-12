import websocket
import time
import build.gameSession_pb2 as gameSessionProto
import google.protobuf as protobuf
from google.protobuf.message import DecodeError

def create_game_session_request(ws):
    request = gameSessionProto.Request()
    create_game_session_request = gameSessionProto.CreateGameSessionRequest()

    create_game_session_request.title = "python-test"
    create_game_session_request.mode = gameSessionProto.GameMode.PVP

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

def main():
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:8080",
                                on_message = on_message,
                                on_error = on_error,
                                on_close = on_close,
                                on_open = on_open)
    ws.run_forever()
