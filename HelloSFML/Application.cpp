#include "Application.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <vector>

Application::Application() :
    window_(sf::VideoMode({600, 800}), "SFML Window"),
    shape_(100.f),
    text_(font_),   // sf::Text needs a font at construction; it is loaded below
    sound_(buffer_), // sf::Sound needs a buffer at construction; it is filled below
    background_(backgroundTexture_) // sf::Sprite needs a texture at construction; it is loaded below
{
    window_.setKeyRepeatEnabled(false);
    view_ = window_.getDefaultView(); // start with the normal, unzoomed view

    // Background: ASSET_DIR (set in CMakeLists.txt) is the project folder, so the
    // image is found no matter which working directory the exe is started from
    if (!backgroundTexture_.loadFromFile(std::string(ASSET_DIR) + "/background.jpg"))
        std::cerr << "Failed to load background.jpg\n";
    background_.setTexture(backgroundTexture_, true); // true = reset the sprite's rect to the new texture size
    // Tile the image over a large area of the world, so it scrolls with the camera
    backgroundTexture_.setRepeated(true);
    background_.setTextureRect(sf::IntRect({0, 0}, {10000, 10000}));
    background_.setPosition({-5000.f, -5000.f});

    shape_.setFillColor(sf::Color::Green);
    rectangle_.setSize(sf::Vector2f(200,100));
    rectangle_.setPosition({100,150});
    rectangle_.setFillColor(sf::Color::Red);
    rectangle_.setOutlineThickness(5);
    rectangle_.setOutlineColor(sf::Color(255,165,0));
    window_.setFramerateLimit(60);

    texture_.loadFromFile("Player.png");
    player_.setTexture(&texture_);
    player_.setPosition({250,250});
    player_.setSize({64,64});
    // Pivot at the centre so rotation and scaling happen around the middle, not the top-left
    player_.setOrigin(player_.getSize() / 2.f);

    enemy_.setFillColor(sf::Color::Red);
    enemy_.setPosition({400,250});
    enemy_.setSize({64,64});
    enemy_.setOrigin(enemy_.getSize() / 2.f);

    // Font: using the Windows system font; copy a .ttf next to the exe to ship your own
    if (!font_.openFromFile("C:/Windows/Fonts/arial.ttf"))
        std::cerr << "Failed to load font\n";
    text_.setString("Arrows: player   WASD: enemy   Esc: quit");
    text_.setCharacterSize(18);
    text_.setFillColor(sf::Color::White);
    text_.setPosition({10.f, 10.f});

    // Collision sound: no audio file needed, so generate a short fading 220 Hz beep.
    // To use a file instead: buffer_.loadFromFile("hit.wav");
    constexpr unsigned sampleRate = 44100;
    constexpr float duration = 0.15f; // seconds
    constexpr float frequency = 220.f;
    constexpr float pi = 3.14159265f;
    std::vector<std::int16_t> samples(static_cast<std::size_t>(sampleRate * duration));
    for (std::size_t i = 0; i < samples.size(); ++i)
    {
        const float t = static_cast<float>(i) / sampleRate;
        const float fade = 1.f - static_cast<float>(i) / samples.size();
        samples[i] = static_cast<std::int16_t>(12000.f * fade * std::sin(2.f * pi * frequency * t));
    }
    if (!buffer_.loadFromSamples(samples.data(), samples.size(), 1, sampleRate, {sf::SoundChannel::Mono}))
        std::cerr << "Failed to create collision sound\n";
    sound_.setBuffer(buffer_);
}

bool Application::IsRunning() const
{
    return window_.isOpen();
}

void Application::ProcessEvents()
{
    // Event visitor: dispatches each event to the matching HandleEvent overload
    while (const std::optional event = window_.pollEvent())
        event->visit([this](const auto& e) { HandleEvent(e); });
}

void Application::Render()
{
    window_.clear();

    // window_.draw(shape_);      // disabled
    // window_.draw(rectangle_);  // disabled

    // World: drawn through the zoomable, following view (background included,
    // so you can see the camera move)
    window_.setView(view_);
    window_.draw(background_);
    window_.draw(player_);
    window_.draw(enemy_);

    // UI: drawn with the default view so the text stays fixed on screen
    window_.setView(window_.getDefaultView());
    window_.draw(text_);
    window_.display();
}

// Window events
void Application::HandleEvent(const sf::Event::Closed&)
{
    window_.close();
}

void Application::HandleEvent(const sf::Event::Resized& resized)
{
    std::cout << "Size: " << resized.size.x << "x" << resized.size.y << '\n';
}

void Application::HandleEvent(const sf::Event::FocusGained&)
{
    std::cout << "Focus gained\n";
}

void Application::HandleEvent(const sf::Event::FocusLost&)
{
    std::cout << "Focus lost\n";
}

// Keyboard events
void Application::HandleEvent(const sf::Event::KeyPressed& key)
{
    if (key.code == sf::Keyboard::Key::Escape)
        window_.close();
    else if (key.code == sf::Keyboard::Key::W)
        std::cout << "W pressed\n";
}

void Application::HandleEvent(const sf::Event::KeyReleased& key)
{
    if (key.code == sf::Keyboard::Key::W)
        std::cout << "W released\n";
}

// Mouse events
void Application::HandleEvent(const sf::Event::MouseButtonPressed& mouse)
{
    if (mouse.button == sf::Mouse::Button::Left)
        std::cout << "Mouse Button Pressed at: " << mouse.position.x << ", " << mouse.position.y << '\n';
}

void Application::HandleEvent(const sf::Event::MouseButtonReleased& mouse)
{
    if (mouse.button == sf::Mouse::Button::Left)
        std::cout << "Mouse Button Released at: " << mouse.position.x << ", " << mouse.position.y << '\n';
}

void Application::HandleEvent(const sf::Event::MouseMoved& mouse)
{
    std::cout << "Mouse moved to: " << mouse.position.x << ", " << mouse.position.y << '\n';
}

void Application::HandleEvent(const sf::Event::MouseWheelScrolled& wheel)
{
    // World point currently under the mouse, before zooming
    const sf::Vector2f before = window_.mapPixelToCoords(wheel.position, view_);

    // Scroll up = zoom in, scroll down = zoom out
    zoom_ *= (wheel.delta > 0) ? 0.9f : 1.1f;
    zoom_ = std::clamp(zoom_, 0.2f, 5.f);

    // A bigger view size shows more of the world, so objects look smaller
    view_.setSize(window_.getDefaultView().getSize() * zoom_);

    // The same pixel now maps to a different world point; shift the view
    // by the difference so the point under the mouse stays put
    const sf::Vector2f after = window_.mapPixelToCoords(wheel.position, view_);
    view_.move(before - after);
}

void Application::Update(){
    // Delta time keeps speed independent of frame rate
    const float dt = clock_.restart().asSeconds();

    const float moveSpeed = 200.f;                // pixels per second
    const sf::Angle rotateSpeed = sf::degrees(120.f); // per second
    const float scaleSpeed = 1.f;                 // scale units per second

    using Key = sf::Keyboard::Key;

    // Player translation: arrow keys
    sf::Vector2f direction{0.f, 0.f};
    if (sf::Keyboard::isKeyPressed(Key::Left))  direction.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(Key::Right)) direction.x += 1.f;
    if (sf::Keyboard::isKeyPressed(Key::Up))    direction.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(Key::Down))  direction.y += 1.f;
    const sf::Vector2f lastPosition = player_.getPosition(); // saved before moving
    player_.move(direction * moveSpeed * dt);

    // Enemy translation: WASD
    sf::Vector2f enemyDirection{0.f, 0.f};
    if (sf::Keyboard::isKeyPressed(Key::A)) enemyDirection.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(Key::D)) enemyDirection.x += 1.f;
    if (sf::Keyboard::isKeyPressed(Key::W)) enemyDirection.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(Key::S)) enemyDirection.y += 1.f;
    const sf::Vector2f lastPositionEnemy = enemy_.getPosition();
    enemy_.move(enemyDirection * moveSpeed * dt);

    // Undo the player's move if it now overlaps the enemy
    if (player_.getGlobalBounds().findIntersection(enemy_.getGlobalBounds())) {
        player_.setPosition(lastPosition);
        // Don't restart the sound while it is still playing
        if (sound_.getStatus() != sf::Sound::Status::Playing)
            sound_.play();
    }

    // Undo the enemy's move if it now overlaps the enemy
    if (enemy_.getGlobalBounds().findIntersection(player_.getGlobalBounds())) {
        enemy_.setPosition(lastPositionEnemy);
        if (sound_.getStatus() != sf::Sound::Status::Playing)
            sound_.play();
    }

    // Camera: follow whoever actually moved this frame. If both moved, the
    // player wins. If neither moved, the camera stays where it is.
    const bool playerMoved = player_.getPosition() != lastPosition;
    const bool enemyMoved = enemy_.getPosition() != lastPositionEnemy;
    if (playerMoved)
        view_.setCenter(player_.getPosition());
    else if (enemyMoved)
        view_.setCenter(enemy_.getPosition());

    // Rotation: Q = counter-clockwise, E = clockwise
    if (sf::Keyboard::isKeyPressed(Key::Q)) player_.rotate(-rotateSpeed * dt);
    if (sf::Keyboard::isKeyPressed(Key::E)) player_.rotate(rotateSpeed * dt);

    // Scale: Z = shrink, X = grow (clamped so it never flips or vanishes)
    float scale = player_.getScale().x;
    if (sf::Keyboard::isKeyPressed(Key::Z)) scale -= scaleSpeed * dt;
    if (sf::Keyboard::isKeyPressed(Key::X)) scale += scaleSpeed * dt;
    scale = std::clamp(scale, 0.2f, 5.f);
    player_.setScale({scale, scale});
}