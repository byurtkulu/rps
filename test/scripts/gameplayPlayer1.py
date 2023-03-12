import websocket
import time
import build.gameSession_pb2 as gameSessionProto
import google.protobuf as protobuf
from google.protobuf.message import DecodeError

def create_game_session_request(ws):
    print("create_game_session_request inputs:")

    is_exit = input("Type \"exit\" to disconnect, \"skip\" to skip this step or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return
    elif (is_exit.lower() == "skip"):
        return join_gamed_session_request(ws)


    request = gameSessionProto.Request()
    create_game_session_request = gameSessionProto.CreateGameSessionRequest()

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

    # create_game_session_request.title = "python-test"
    # create_game_session_request.mode = gameSessionProto.GameMode.PVP

    request.create_game_session_request.CopyFrom(create_game_session_request)
    return request.SerializeToString()

def join_gamed_session_request(ws):
    print("join_game_session_request inputs:")

    is_exit = input("Type \"exit\" to disconnect, \"skip\" to skip this step or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return
    elif (is_exit.lower() == "skip"):
        return wait_till_game_ready_request(ws)


    request = gameSessionProto.Request()
    join_game_session_request = gameSessionProto.JoinGameSessionRequest()
    game_session_id = input("Enter game session id you want to join: ")
    player_name = input("Enter your name: ")
    join_game_session_request.game_session_id = int(game_session_id)
    join_game_session_request.player_name = player_name
    request.join_game_session_request.CopyFrom(join_game_session_request)
    return request.SerializeToString()

def create_rock_paper_scissors_request(ws):
    print("create_rock_paper_scissors_request inputs:")
    is_exit = input("Type \"exit\" to disconnect, \"skip\" to skip this step or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return
    elif (is_exit.lower() == "skip"):
        return wait_till_game_finish_request(ws)

    request = gameSessionProto.Request()
    rock_paper_scissors_request = gameSessionProto.RockPaperScissorsRequest()
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

def wait_till_game_ready_request(ws):
    print("wait_till_game_ready_request inputs:")
    is_exit = input("Type \"exit\" to disconnect, \"skip\" to skip this step or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return
    elif (is_exit.lower() == "skip"):
        return create_rock_paper_scissors_request(ws)

    request = gameSessionProto.Request()
    wait_till_game_ready_request = gameSessionProto.WaitTillGameReadyRequest()
    game_session_id = input("Enter game session id you want to subscribe: ")
    wait_till_game_ready_request.game_session_id = int(game_session_id)
    request.wait_till_game_ready_request.CopyFrom(wait_till_game_ready_request)
    return request.SerializeToString()

def wait_till_game_finish_request(ws):
    print("wait_till_game_finish_request inputs:")
    is_exit = input("Type \"exit\" to disconnect, \"skip\" to skip this step or press enter to continue: ")
    if (is_exit.lower() == "exit"):
        ws.close()
        return
    elif (is_exit.lower() == "skip"):
        return
    request = gameSessionProto.Request()
    wait_till_game_finish_request = gameSessionProto.WaitTillGameFinishRequest()
    game_session_id = input("Enter game session id you want to subscribe: ")
    wait_till_game_finish_request.game_session_id = int(game_session_id)
    request.wait_till_game_finish_request.CopyFrom(wait_till_game_finish_request)
    return request.SerializeToString()


def process_message(response):
    if response.HasField("create_game_session_response"):
        print("Game session created")
        print(response.create_game_session_response)
        ws.send(join_gamed_session_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)
    elif response.HasField("join_game_session_response"):
        print("Game session joined")
        print(response.join_game_session_response)
        ws.send(wait_till_game_ready_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)
    elif response.HasField("wait_till_game_ready_response"):
        print("Game ready")
        print(response.wait_till_game_ready_response)
        ws.send(create_rock_paper_scissors_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)
    elif response.HasField("rock_paper_scissors_response"):
        print("Rock paper scissors response")
        print(response.rock_paper_scissors_response)
        ws.send(wait_till_game_finish_request(ws), opcode=websocket.ABNF.OPCODE_BINARY)
    elif response.HasField("wait_till_game_finish_response"):
        print("Game finished")
        print(response.wait_till_game_finish_response)
        ws.close()

def on_message(ws, message):
    try:
        response = gameSessionProto.Response()
        response.ParseFromString(message)
    except DecodeError as e:
        e.__class__.use_enum_values_for_proto3 = True
        raise e
    process_message(response)

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
