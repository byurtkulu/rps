///
#include "gtest/gtest.h"
#include "../src/gameSessionManager.h"

class GtestGameManager : public ::testing::Test {
protected:
    void SetUp() override {
        gameManager = new rps::GameSessionManager();
    }

    void TearDown() override {
        delete gameManager;
    }

    rps::GameSessionManager *gameManager;
};

TEST_F(GtestGameManager, test_NewConnectionMapping) {
   for (int i = 0; i < 100; i++) {
       gameManager->new_connection("test" + std::to_string(i));
   }
   ASSERT_TRUE(gameManager->new_connection("test" + std::to_string(100)));
   ASSERT_FALSE(gameManager->new_connection("test" + std::to_string(0)));
   ASSERT_FALSE(gameManager->new_connection("test" + std::to_string(25)));
   ASSERT_FALSE(gameManager->new_connection("test" + std::to_string(50)));
}

TEST_F(GtestGameManager, test_CreteGameSession) {
    protobuf_session::Request req;
    req.mutable_create_game_session_request()->set_title("test");
    req.mutable_create_game_session_request()->set_mode(protobuf_session::GameMode::PVE);

    auto r = gameManager->handle("client", req.SerializeAsString());

    protobuf_session::Response resp;
    ASSERT_TRUE(resp.ParseFromString(r));
    ASSERT_TRUE(resp.response_case() == protobuf_session::Response::kCreateGameSessionResponse);
    ASSERT_TRUE(resp.create_game_session_response().has_game_session());
    ASSERT_EQ(resp.create_game_session_response().game_session().mode(), protobuf_session::GameMode::PVE);
    ASSERT_EQ(resp.create_game_session_response().game_session().title(), "test");
    ASSERT_EQ(resp.create_game_session_response().game_session().id(), 1);
}

TEST_F(GtestGameManager, test_JoinGameSessionNotFound) {
    protobuf_session::Request req;
    req.mutable_join_game_session_request()->set_game_session_id(1);
    req.mutable_join_game_session_request()->set_player_name("test");

    auto r = gameManager->handle("client", req.SerializeAsString());

    protobuf_session::Response resp;
    ASSERT_TRUE(resp.ParseFromString(r));
    ASSERT_TRUE(resp.response_case() == protobuf_session::Response::kError);
    ASSERT_EQ(resp.error().error_code(), protobuf_session::ErrorCode::GAME_SESSION_NOT_FOUND);
}

TEST_F(GtestGameManager, test_JoinGameSession) {
    protobuf_session::Request reqCreate;
    reqCreate.mutable_create_game_session_request()->set_title("test");
    reqCreate.mutable_create_game_session_request()->set_mode(protobuf_session::GameMode::PVE);

    auto rCreate = gameManager->handle("client", reqCreate.SerializeAsString());

    protobuf_session::Response respCreate;
    ASSERT_TRUE(respCreate.ParseFromString(rCreate));

    protobuf_session::Request req;
    req.mutable_join_game_session_request()->set_game_session_id(respCreate.create_game_session_response().game_session().id());
    req.mutable_join_game_session_request()->set_player_name("testPlayer");

    auto r = gameManager->handle("client", req.SerializeAsString());

    protobuf_session::Response resp;
    ASSERT_TRUE(resp.ParseFromString(r));
    ASSERT_TRUE(resp.response_case() == protobuf_session::Response::kJoinGameSessionResponse);
    ASSERT_TRUE(resp.join_game_session_response().player_id() != 0);
}

TEST_F(GtestGameManager, test_RockPaperScissors) {
    protobuf_session::Request reqCreate;
    reqCreate.mutable_create_game_session_request()->set_title("test");
    reqCreate.mutable_create_game_session_request()->set_mode(protobuf_session::GameMode::PVE);

    auto rCreate = gameManager->handle("client", reqCreate.SerializeAsString());

    protobuf_session::Response respCreate;
    ASSERT_TRUE(respCreate.ParseFromString(rCreate));

    protobuf_session::Request reqJoin;
    reqJoin.mutable_join_game_session_request()->set_game_session_id(respCreate.create_game_session_response().game_session().id());
    reqJoin.mutable_join_game_session_request()->set_player_name("testPlayer");

    auto rJoin = gameManager->handle("client", reqJoin.SerializeAsString());

    protobuf_session::Response respJoin;
    ASSERT_TRUE(respJoin.ParseFromString(rJoin));


    protobuf_session::Request req;
    req.mutable_rock_paper_scissors_request()->set_game_session_id(respCreate.create_game_session_response().game_session().id());
    req.mutable_rock_paper_scissors_request()->set_player_id(respJoin.join_game_session_response().player_id());
    req.mutable_rock_paper_scissors_request()->set_choice(protobuf_session::GameChoice::ROCK);

    auto r = gameManager->handle("client", req.SerializeAsString());
    protobuf_session::Response resp;
    ASSERT_TRUE(resp.ParseFromString(r));
    ASSERT_EQ(resp.response_case(), protobuf_session::Response::kRockPaperScissorsResponse);
    ASSERT_TRUE(resp.rock_paper_scissors_response().has_game_session());
    ASSERT_TRUE(resp.rock_paper_scissors_response().game_session().has_player1());
    ASSERT_TRUE(resp.rock_paper_scissors_response().game_session().has_player2());
    ASSERT_TRUE(resp.rock_paper_scissors_response().game_session().player1().is_ai());
    ASSERT_FALSE(resp.rock_paper_scissors_response().game_session().player2().is_ai());
    ASSERT_TRUE(resp.rock_paper_scissors_response().game_session().player1().name() == "Bob");
    ASSERT_TRUE(resp.rock_paper_scissors_response().game_session().player2().name() == "testPlayer");
    ASSERT_TRUE(resp.rock_paper_scissors_response().game_session().player2().choice() == protobuf_session::GameChoice::ROCK);

    // Winner decision test
    auto bobs_coice = resp.rock_paper_scissors_response().game_session().player1().choice();
    auto game_session = resp.rock_paper_scissors_response().game_session();
    if (bobs_coice == protobuf_session::GameChoice::ROCK) {
        ASSERT_TRUE(game_session.is_draw());
    } else if (bobs_coice == protobuf_session::GameChoice::PAPER) {
        ASSERT_EQ(game_session.winner_id(), game_session.player1().id());
    } else if (bobs_coice == protobuf_session::GameChoice::SCISSORS) {
        ASSERT_EQ(game_session.winner_id(), game_session.player2().id());
    }
}




