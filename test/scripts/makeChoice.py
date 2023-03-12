import websocket
import build.gameSession_pb2 as gameSessionProto

def create_rock_paper_scissors_request(ws):
    request = gameSessionProto.Request()
    rock_paper_scissors_request = gameSessionProto.RockPaperScissorsRequest()
    is_exit = input("Type \"exit\" to disconnect or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return
    game_session_id = input("Enter game session id: ")
    player_id = input("Enter player id: ")
    choice = input("Enter choice: (r/p/s) ")
    rock_paper_scissors_request.game_session_id = int(game_session_id)
    choice_enum = gameSessionProto.GameChoice.UNKNOWN_CHOICE
    if choice == "r":
        choice_enum = gameSessionProto.GameChoice.ROCK
    elif choice == "p":
        choice_enum = gameSessionProto.GameChoice.PAPER
    elif choice == "s":
        choice_enum = gameSessionProto.GameChoice.SCISSORS
    else:
        print("Invalid choice")
        return
    rock_paper_scissors_request.choice = choice_enum
    rock_paper_scissors_request.player_id = int(player_id)
    request.rock_paper_scissors_request.CopyFrom(rock_paper_scissors_request)
    return request.SerializeToString()

def on_message(ws, message):
    response = gameSessionProto.Response()
    response.ParseFromString(message)
    print(response)
    ws.send(create_rock_paper_scissors_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print(f"### closed {close_status_code} {close_msg} ###")

def on_open(ws):
    print("connected")
    ws.send(create_rock_paper_scissors_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("ws://localhost:8080",
                                on_message = on_message,
                                on_error = on_error,
                                on_close = on_close,
                                on_open = on_open)
    ws.run_forever()
