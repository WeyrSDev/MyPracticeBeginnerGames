#include "Game.hpp"

Game::Game():
resources(),
windowTitle("Puzza (WIP:2015) by Hapax (github.com/Hapaxia)"),
graphics(resources),
timestep(),
window(sf::VideoMode(800, 600), windowTitle, sf::Style::Default),
scores(window),
ball(),
player(),
opponent(),
sound(),
background(window.getView().getSize()),
message(),
messageBox()
{
	scores.setFont(resources.getFont("main"));
	message.setFont(resources.getFont("main"));
	message.setCharacterSize(45u);
	message.setPosition(sf::Vector2f(window.getSize() / 2u));
	messageBox.setFillColor(pl::Sfml::colorFromColorAndAlpha(pl::Colors::Black, 0.75));
	window.setMouseCursorVisible(false);
	ball.setPosition(sf::Vector2f(window.getSize() / 2u));
	opponent.setAcceleration(100.f);
	opponent.setDeceleration(250.f);
	opponent.setMaximumSpeed(1.25f);
	timestep.setStep(1.0 / 500.0); // decrease timestep size to a 500th of a second to catch collisions at higher ball speeds
	sound.setBuffer(resources.getSoundBuffer("paddle"));
	background.setTexture(&resources.getTexture("background tile"));
	background.setTextureRect({ sf::Vector2i{ 0, 0 }, sf::Vector2i(window.getSize()) / 1 });
}

void Game::run()
{
	while (window.isOpen())
	{
		// process events
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (DEV::eventIsDefaultCloseWindow(event))
				window.close();
			else if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::F12) // forces program error
					throw "Forced exception";
				else if (event.key.code == sf::Keyboard::F1) // stops the ball
					ball.setSpeed(0.f);
				else if (event.key.code == sf::Keyboard::F2) // resets and restarts the ball in the default position, direction and speed
					resetBall();
				else if (event.key.code == sf::Keyboard::F5) // resets and restarts the game (ball and scores)
					reset();
				else if (event.key.code == sf::Keyboard::F6) // resets the game (ball and scores). ball is stopped
				{
					reset();
					ball.setSpeed(0.f);
				}
				else if (event.key.code == sf::Keyboard::Space)
				{
					switch (state)
					{
					case State::Ready:
						resetBall();
					case State::Paused:
						state = State::Running;
						break;
					case State::Over:
						state = State::Ready;
						reset();
						break;
					case State::Running:
					default:
						state = State::Paused;
						break;
					}
				}
				else if (event.key.code == sf::Keyboard::Q)
				{
					state = State::Over;
				}
			}
		}

		// main updates
		timestep.addFrame();
		while (timestep.isUpdateRequired())
			update();

		// update window title
		window.setTitle(windowTitle +
			" | Ball Speed: " + pl::stringFrom(static_cast<int>(ball.getSpeed())) +
			" | Ball Spin: " + pl::stringFrom(ball.getSpin())
			);

		// update message
		switch (state)
		{
		case State::Ready:
			message.setString("Press Space to start");
			break;
		case State::Paused:
			message.setString("Paused - Press Space to continue");
			break;
		case State::Over:
			message.setString("Game over! Press Space to reset");
			break;
		default:
			message.setString("");
		}
		const sf::Vector2f messageBoxPadding(4.f, 2.f);
		message.setOrigin(pl::Anchor::Local::getCenter(message));
		messageBox.setPosition(sf::Vector2f(window.getSize() / 2u) - messageBoxPadding);
		messageBox.setSize(pl::Anchor::Local::getBottomRight(message) - pl::Anchor::Local::getTopLeft(message) + messageBoxPadding * 2.f);
		messageBox.setOrigin(pl::Anchor::Local::getCenter(messageBox));

		// update display
		window.clear();
		window.draw(background);
		window.draw(graphics);
		window.draw(scores);
		if (state != State::Running)
		{
			window.draw(messageBox);
			window.draw(message);
		}
		window.display();
	}
}

void Game::reset()
{
	resetBall();
	player.resetScore();
	opponent.resetScore();
}

void Game::resetBall()
{
	ball.setPosition(window.getView().getSize() / 2.f);
	ball.setDirection(115.f);
	ball.setSpeed(200.f);
	ball.setSpin(0.f);
}

void Game::update()
{
	updatePlayerPaddle();
	updateOpponentPaddle();
	if (state == State::Running)
		updateBall();
	updateScores();

	// update ball graphic
	graphics.updateBall(ball, timestep.getStepAsFloat());
}

void Game::updatePlayerPaddle()
{
	// input
	const float mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window)).y;
	player.setTargetPosition((mouse / window.getView().getSize().y) * 2 - 1);

	// update
	player.update(timestep.getStepAsFloat());

	// update graphic
	graphics.updatePlayer(window.getView(), player, timestep.getStepAsFloat());
}

void Game::updateOpponentPaddle()
{
	// input (AI)
	const float radiansFromDegreesMultiplier = 0.0174532925f; // pi / 180 approx.
	const float directionInRadians = ball.getDirection() * radiansFromDegreesMultiplier;
	//const float opponentTarget = ball.getPosition().y; // target is ball's y position
	//const float opponentTarget = ball.getPosition().y + (-cos(directionInRadians) * ball.getSpeed() * 0.1f); // target is ahead of ball's y position by ball's speed
	//const float opponentTarget = ball.getPosition().y + (-cos(directionInRadians) * ball.getSpeed() * ((opponentGraphic.getPosition().x - ballGraphic.getPosition().x) / window.getView().getSize().x)); // target is ahead of ball's y position by ball's speed and distance from paddle

	// target is ahead of ball's y position by ball's speed and distance from paddle but makes error based on spin
	const float opponentTarget = ball.getPosition().y +
		(-cos(directionInRadians) * (ball.getSpeed() / (5.f * abs(ball.getSpin() + 1.f)) *
		(graphics.getBallDistanceLeftOfOpponent() / window.getView().getSize().x)));
	opponent.setTargetPosition((opponentTarget - (window.getView().getSize().y / 2)) / graphics.getPaddleReachFromCenter());

	// update
	opponent.update(timestep.getStepAsFloat());

	// update graphic
	graphics.updateOpponent(window.getView(), opponent, timestep.getStepAsFloat());
}

void Game::updateBall()
{
	// input
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))
		ball.changeSpeed(3.f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))
		ball.changeSpeed(-3.f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))
		ball.setDirection(ball.getDirection() + 0.3f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))
		ball.setDirection(ball.getDirection() - 0.3f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Period))
		ball.changeSpin(0.3f);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Comma))
		ball.changeSpin(-0.3f);

	// update
	ball.update(timestep.getStepAsFloat());

	// collision with top or bottom
	if ((ball.getPosition().y < ball.getRadius()) || (ball.getPosition().y > window.getView().getSize().y - ball.getRadius()))
	{
		playSound("wall");
		ball.setPosition({ ball.getPosition().x, pl::clamp(ball.getPosition().y, ball.getRadius(), window.getView().getSize().y - ball.getRadius()) });
		ball.setDirection(ball.getDirection() - ball.getSpin() * 0.5f);
		ball.flipDirectionVertically();
		ball.setSpin(ball.getSpin() * 0.1f);
	}

	// collision with sides
	if (ball.getPosition().x < ball.getRadius())
	{
		playSound("opponent score");
		opponent.increaseScore();
		resetBall();
	}
	else if (ball.getPosition().x > window.getView().getSize().x - ball.getRadius())
	{
		playSound("player score");
		player.increaseScore();
		resetBall();
	}

	// create collision box for ball since we don't want to collide with the entire sprite
	const sf::FloatRect ballCollisionBox
	{
		ball.getPosition().x - ball.getRadius(), ball.getPosition().y - ball.getRadius(), ball.getRadius() * 2.f, ball.getRadius() * 2.f
	};

	// collision with player paddle
	if (graphics.playerCollidesWith(ballCollisionBox))
	{

		if ((graphics.isInLineWithPlayer(ball.getPosition().y)) &&
			(graphics.isToTheRightOfPlayer(ball.getPosition().x)) &&
			(ball.getDirection() > 180.f))
		{
			playSound("paddle");
			ball.setPosition({ graphics.getPlayerFront() + ball.getRadius(), ball.getPosition().y });
			ball.changeSpeed(25.f);
			ball.setSpin(ball.getSpin() * 0.25f);
			ball.setDirection(ball.getDirection() - ball.getSpin());
			ball.flipDirectionHorizontally();
			ball.changeSpin(-5.f * player.getSpeed() * ball.getSpeed() * timestep.getStepAsFloat());
			graphics.startBallAnimHit();
			graphics.startPlayerAnimHit();
		}
		else if (graphics.isToTheRightOfPlayerBack(ball.getPosition().x))
		{
			if (graphics.isHigherThanPlayer(ball.getPosition().y))
			{
				ball.setPosition({ ball.getPosition().x, graphics.getPlayerTop() - ball.getRadius() });
				if (pl::inRange(ball.getDirection(), pl::Range<float>{90.f, 270.f}))
					ball.flipDirectionVertically();
			}
			else if (graphics.isLowerThanPlayer(ball.getPosition().y))
			{
				ball.setPosition({ ball.getPosition().x, graphics.getPlayerBottom() + ball.getRadius() });
				if (!pl::inRange(ball.getDirection(), pl::Range<float>{90.f, 270.f}))
					ball.flipDirectionVertically();
			}
		}
	}

	// collision with opponent paddle
	if (graphics.opponentCollidesWith(ballCollisionBox))
	{
		if ((graphics.isInLineWithOpponent(ball.getPosition().y)) &&
			(graphics.isToTheLeftOfOpponent(ball.getPosition().x)) &&
			(ball.getDirection() < 180.f))
		{
			playSound("paddle");
			ball.setPosition({ graphics.getOpponentFront() - ball.getRadius(), ball.getPosition().y });
			ball.changeSpeed(25.f);
			ball.setSpin(ball.getSpin() * 0.25f);
			ball.setDirection(ball.getDirection() - ball.getSpin());
			ball.flipDirectionHorizontally();
			ball.changeSpin(5.f * opponent.getSpeed() * ball.getSpeed() * timestep.getStepAsFloat());
			graphics.startBallAnimHit();
			graphics.startOpponentAnimHit();
		}
		else if (graphics.isToTheLeftOfOpponentBack(ball.getPosition().x))
		{
			if (graphics.isHigherThanOpponent(ball.getPosition().y))
			{
				ball.setPosition({ ball.getPosition().x, graphics.getOpponentTop() - ball.getRadius() });
				if (pl::inRange(ball.getDirection(), pl::Range<float>{90.f, 270.f}))
					ball.flipDirectionVertically();
			}
			else if (graphics.isLowerThanOpponent(ball.getPosition().y))
			{
				ball.setPosition({ ball.getPosition().x, graphics.getOpponentBottom() + ball.getRadius() });
				if (!pl::inRange(ball.getDirection(), pl::Range<float>{90.f, 270.f}))
					ball.flipDirectionVertically();
			}
		}
	}
}

void Game::updateScores()
{
	scores.updatePlayer(player.getScore());
	scores.updateOpponent(opponent.getScore());
}

void Game::playSound(const std::string& soundName)
{
	sound.setBuffer(resources.getSoundBuffer(soundName));
	sound.play();
}
