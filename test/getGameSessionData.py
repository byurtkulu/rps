import websocket
import build.gameSession_pb2 as gameSessionProto

def create_game_session_request():
    request = gameSessionProto.Request()
    game_session_id = input("Enter game session id you want to get data from: ")
    request.get_game_session_data_request.game_session_id = int(game_session_id)
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
