//
// Created by Bahadır Yurtkulu on 25/02/2023.
//

#pragma once

#include <iostream>
#include <string>
#include <mutex>
#include <shared_mutex>
#include <random>
#include "../build/gameSession.pb.h"
#include "../build/gamePlay.pb.h"
#include "gameSession.h"
#include "../util/util.h"

namespace rps {
    class GameSessionManager {
        using GameSessionID = uint64_t;
        using PlayerID = uint64_t;
    public:
        GameSessionManager() = default;

        std::string handle(std::string&& message) {
            protobuf_session::Request request;
            request.ParseFromString(message);

            switch (request.request_case()) {
                case protobuf_session::Request::kCreateGameSessionRequest: {
                    std::cout << "Create game session request" << std::endl;
                    return create_game_session(request.create_game_session_request());
                }
                case protobuf_session::Request::kGetGameSessionDataRequest: {
                    std::cout << "Get game session data request" << std::endl;
                    return get_game_session_data(request.get_game_session_data_request());
                }
                case protobuf_session::Request::kJoinGameSessionRequest: {
                    std::cout << "Join game session request" << std::endl;
                    return join_game_session(request.join_game_session_request());
                }
                case protobuf_session::Request::kWaitTillGameReadyRequest: {
                    std::cout << "Wait till game ready request" << std::endl;
                    return wait_till_game_ready(request.wait_till_game_ready_request());
                }
                case protobuf_session::Request::kRockPaperScissorsRequest: {
                    std::cout << "Rock Paper Scissors request" << std::endl;
                    return rock_paper_scissors(request.rock_paper_scissors_request());
                }
                case protobuf_session::Request::kWaitTillGameFinishRequest: {
                    std::cout << "Wait till game finish request" << std::endl;
                    return wait_till_game_finish(request.wait_till_game_finish_request());
                }

                case protobuf_session::Request::REQUEST_NOT_SET: {
                    std::cout << "Request not set" << std::endl;
                    return error_serialized(protobuf_session::ErrorCode::REQUEST_NOT_SET);
                }
                default: {
                    std::cout << "Unknown request" << std::endl;
                    return error_serialized(protobuf_session::ErrorCode::UNKNOWN_REQUEST);
                }
            }
        }

    private:
        // handlers
        std::string create_game_session(const protobuf_session::CreateGameSessionRequest &request) {
            std::unique_lock lock(game_sessions_mutex_);
            GameSession::GameSessionData data;
            data.set_id(game_session_counter_.next());
            data.set_title(request.title());
            data.set_mode(request.mode());
            data.set_state(protobuf_session::GameSessionState::WAITING_FOR_PLAYERS);

            if (data.mode() == protobuf_session::GameMode::PVE) {
                protobuf_session::Player* ai = data.mutable_player1();
                ai->set_id(player_counter_.next());
                ai->set_name("bob");
                ai->set_is_ai(true);
            }

            auto serialized_response = response_to_create_game_session(data);
            game_sessions_.insert({game_session_counter_.current(), std::make_unique<GameSession>(std::move(data))});
            return serialized_response;
        }

        std::string get_game_session_data(const protobuf_session::GetGameSessionDataRequest& request) {
            std::shared_lock lock(game_sessions_mutex_);
            std::cout << "Get game session data request: \n" << request.DebugString() << std::endl;
            auto itr = game_sessions_.find(request.game_session_id());
            if (itr == game_sessions_.end()) {
                std::cout << "Game session not found" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
            }
            return response_to_get_game_session_data(itr->second->data());
        }

        std::string join_game_session(const protobuf_session::JoinGameSessionRequest& request) {
            std::unique_lock lock(game_sessions_mutex_);
            std::cout << "Join game session request: \n" << request.DebugString() << std::endl;
            auto itr = game_sessions_.find(request.game_session_id());
            if (itr == game_sessions_.end()) {
                std::cout << "Game session not found" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
            }

            protobuf_session::GameSession& data = itr->second->mutable_data();

            if (data.state() != protobuf_session::GameSessionState::WAITING_FOR_PLAYERS) {
                std::cout << "Game session is not waiting for players" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_WAITING_FOR_PLAYERS);
            }

            protobuf_session::Player* player;
            if (not data.has_player1()) {
                player = data.mutable_player1();
                player->set_id(player_counter_.next());
                player->set_name(request.player_name());
                player->set_is_ai(false);
            } else if (not data.has_player2()) {
                player = data.mutable_player2();
                player->set_id(player_counter_.next());
                player->set_name(request.player_name());
                player->set_is_ai(false);
            } else {
                std::cout << "Game session is full!" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_FULL);
            }

            if (data.has_player1() and data.has_player2()) {
                data.set_state(protobuf_session::GameSessionState::READY);
            }

            return response_to_join_game_session(data, player->id());
        }

        std::string wait_till_game_ready(const protobuf_session::WaitTillGameReadyRequest& request) {
            static constexpr int MAX_WAITING_TIME = 120*1000; // 2 minutes
            static constexpr int SLEEP_FOR = 1500; // 1.5 seconds
            uint32_t waiting_time = 0;
            auto state = protobuf_session::GameSessionState::UNKNOWN_STATE;
            while (state != protobuf_session::GameSessionState::READY) {
                std::this_thread::sleep_for(std::chrono::milliseconds (SLEEP_FOR));
                std::shared_lock lock(game_sessions_mutex_); // this blocks any write operation
                auto itr = game_sessions_.find(request.game_session_id());
                if (itr == game_sessions_.end()) {
                    std::cout << "Game session not found" << std::endl;
                    return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
                }
                waiting_time += SLEEP_FOR;
                if (waiting_time > MAX_WAITING_TIME) {
                    std::cout << "Game session is not become ready in " << (MAX_WAITING_TIME/(1000)) << " seconds." << std::endl;
                    return error_serialized(protobuf_session::ErrorCode::TIMEOUT);
                }
                state = itr->second->data().state();
                if (state == protobuf_session::GameSessionState::READY || state == protobuf_session::GameSessionState::PLAYING) {
                    return response_to_respond_when_game_finished(itr->second->data());
                }
            }
            return error_serialized(protobuf_session::ErrorCode::UNKNOWN_ERROR);
        }

        std::string wait_till_game_finish(const protobuf_session::WaitTillGameFinishRequest& request) {
            static constexpr int MAX_WAITING_TIME = 120*1000; // 2 minutes
            static constexpr int SLEEP_FOR = 1500; // 1.5 seconds
            uint32_t waiting_time = 0;
            auto state = protobuf_session::GameSessionState::UNKNOWN_STATE;
            while (state != protobuf_session::GameSessionState::FINISHED) {
                std::this_thread::sleep_for(std::chrono::milliseconds (SLEEP_FOR));
                std::shared_lock lock(game_sessions_mutex_); // this blocks any write operation
                auto itr = game_sessions_.find(request.game_session_id());
                if (itr == game_sessions_.end()) {
                    std::cout << "Game session not found" << std::endl;
                    return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
                }
                waiting_time += SLEEP_FOR;
                if (waiting_time > MAX_WAITING_TIME) {
                    std::cout << "Game session is not become ready in " << (MAX_WAITING_TIME/(1000)) << " seconds." << std::endl;
                    return error_serialized(protobuf_session::ErrorCode::TIMEOUT);
                }
                state = itr->second->data().state();
                if (state == protobuf_session::GameSessionState::FINISHED) {
                    return response_to_respond_when_game_ready(itr->second->data());
                }
            }
            return error_serialized(protobuf_session::ErrorCode::UNKNOWN_ERROR);
        }

        std::string rock_paper_scissors(const protobuf_session::RockPaperScissorsRequest& request)  {
            std::cout << "Rock paper scissors request: \n" << request.DebugString() << std::endl;
            std::unique_lock lock(game_sessions_mutex_);
            auto itr = game_sessions_.find(request.game_session_id());
            if (itr == game_sessions_.end()) {
                std::cout << "Game session not found" << std::endl;
                return error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
            }

            protobuf_session::GameSession& data = itr->second->mutable_data();
            auto res = make_choice(data, request);
            if (not res.first) {
                return res.second;
            }

            if (data.player1().choice() != protobuf_session::GameChoice::UNKNOWN_CHOICE
                && data.player2().choice() != protobuf_session::GameChoice::UNKNOWN_CHOICE)
            {
                decide_winner(data);
                data.set_state(protobuf_session::GameSessionState::FINISHED);
            }
            return response_to_rock_paper_scissors(data);
        }

        // responses
        std::string response_to_create_game_session(const GameSession::GameSessionData& data) {
            std::cout << "*** create game session data ***\n" << data.DebugString() << std::endl;
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_create_game_session_response()->mutable_game_session()->CopyFrom(data);
            auto serialized = response_wrapper.SerializeAsString();
            return serialized;
        }

        std::string response_to_get_game_session_data(const GameSession::GameSessionData& data) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_get_game_session_data_response()->mutable_game_session()->CopyFrom(data);
            return response_wrapper.SerializeAsString();
        }

        std::string response_to_join_game_session(const GameSession::GameSessionData& data, uint64_t player_id) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_join_game_session_response()->mutable_game_session()->CopyFrom(data);
            response_wrapper.mutable_join_game_session_response()->set_player_id(player_id);
            return response_wrapper.SerializeAsString();
        }

        std::string  response_to_respond_when_game_ready(const GameSession::GameSessionData& data) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_wait_till_game_ready_response()->mutable_game_session()->CopyFrom(data);
            return response_wrapper.SerializeAsString();
        }

        std::string response_to_respond_when_game_finished(const GameSession::GameSessionData& data) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_wait_till_game_ready_response()->mutable_game_session()->CopyFrom(data);
            return response_wrapper.SerializeAsString();
        }

        std::string response_to_rock_paper_scissors(const GameSession::GameSessionData& data) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_rock_paper_scissors_response()->mutable_game_session()->CopyFrom(data);
            return response_wrapper.SerializeAsString();
        }

        // error response
        std::string error_serialized(protobuf_session::ErrorCode err) {
            protobuf_session::Response response_wrapper;
            response_wrapper.mutable_error()->set_error_code(err);
            return response_wrapper.SerializeAsString();
        }

        uint8_t random_choice() {
            std::random_device seed;
            std::mt19937 gen{seed()}; // seed the generator
            std::uniform_int_distribution dist{1, 3}; // set min and max
            return dist(gen); // generate number
        }

        // make choice - returns pair<is_valid, error_serialized_response>
        std::pair<bool, std::string> make_choice(protobuf_session::GameSession& data, const protobuf_session::RockPaperScissorsRequest& request) {
            if (data.state() != protobuf_session::GameSessionState::READY) {
                std::cout << "Game session is not ready" << std::endl;
                return {false, error_serialized(protobuf_session::ErrorCode::GAME_SESSION_NOT_READY)};
            }

            if (request.player_id() == 0) {
                std::cout << "Player id is not valid" << std::endl;
                return {false, error_serialized(protobuf_session::ErrorCode::PLAYER_ID_NOT_VALID)};
            }

            if (data.player1().id() == request.player_id() && not data.player1().is_ai()) {
                data.mutable_player1()->set_choice(request.choice());
                if (data.player2().is_ai()) {
                    data.mutable_player2()->set_choice(static_cast<protobuf_session::GameChoice>(random_choice()));
                }
            } else if (data.player2().id() == request.player_id() && not data.player2().is_ai()) {
                data.mutable_player2()->set_choice(request.choice());
                if (data.player1().is_ai()) {
                    data.mutable_player1()->set_choice(static_cast<protobuf_session::GameChoice>(random_choice()));
                }
            } else {
                std::cout << "Player not found" << std::endl;
                return {false, error_serialized(protobuf_session::ErrorCode::PLAYER_NOT_FOUND)};
            }

            return {true, ""};
        }

        void decide_winner(protobuf_session::GameSession& data) {
            auto player1_choice = data.player1().choice();
            auto player2_choice = data.player2().choice();

            int diff = player1_choice - player2_choice;
            if (diff == 0) {
                data.set_is_draw(true);
            } else if (diff == -2 || diff == 1) {
                data.set_winner_id(data.player1().id());
            } else if (diff == -1 || diff == 2) {
                data.set_winner_id(data.player2().id());
            }
        }



    private:
        std::unordered_map<GameSessionID, std::unique_ptr<GameSession>> game_sessions_;
        std::shared_mutex game_sessions_mutex_;
        util::Counter game_session_counter_;
        util::Counter player_counter_;
    };
} // namespace rps