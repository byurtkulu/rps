import websocket
import build.gameSession_pb2 as gameSessionProto

def create_game_session_request(ws):
    request = gameSessionProto.Request()
    wait_till_game_ready_request = gameSessionProto.WaitTillGameReadyRequest()
    game_session_id = input("Enter game session id you want to subscribe: ")
    wait_till_game_ready_request.game_session_id = int(game_session_id)
    request.wait_till_game_ready_request.CopyFrom(wait_till_game_ready_request)
    return request.SerializeToString()

def on_message(ws, message):
    response = gameSessionProto.Response()
    response.ParseFromString(message)
    print(response)

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
