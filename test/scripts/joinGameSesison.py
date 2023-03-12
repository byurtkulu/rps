import websocket
import build.gameSession_pb2 as gameSessionProto

def join_gamed_session_request(ws):
    request = gameSessionProto.Request()
    join_game_session_request = gameSessionProto.JoinGameSessionRequest()
    is_exit = input("Type \"exit\" to disconnect or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return
    game_session_id = input("Enter game session id you want to join: ")
    player_name = input("Enter your name: ")
    join_game_session_request.game_session_id = int(game_session_id)
    join_game_session_request.player_name = player_name
    request.join_game_session_request.CopyFrom(join_game_session_request)
    return request.SerializeToString()

def on_message(ws, message):
    response = gameSessionProto.Response()
    response.ParseFromString(message)
    print(response)
    ws.send(join_gamed_session_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print(f"### closed {close_status_code} {close_msg} ###")


def on_open(ws):
    print("connected")
    ws.send(join_gamed_session_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:8080",
                                on_message = on_message,
                                on_error = on_error,
                                on_close = on_close,
                                on_open = on_open)
    ws.run_forever()
