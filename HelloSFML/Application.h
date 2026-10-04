#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

class Application
{
public:
    Application();

    bool IsRunning() const;
    void ProcessEvents();
    void Render();
    void Update();

private:
    // One HandleEvent overload per event type; anything without one is ignored.

    // Window events
    void HandleEvent(const sf::Event::Closed&);
    void HandleEvent(const sf::Event::Resized& resized);
    void HandleEvent(const sf::Event::FocusGained&);
    void HandleEvent(const sf::Event::FocusLost&);

    // Keyboard events
    void HandleEvent(const sf::Event::KeyPressed& key);
    void HandleEvent(const sf::Event::KeyReleased& key);

    // Mouse events
    void HandleEvent(const sf::Event::MouseButtonPressed& mouse);
    void HandleEvent(const sf::Event::MouseButtonReleased& mouse);
    void HandleEvent(const sf::Event::MouseMoved& mouse);
    void HandleEvent(const sf::Event::MouseWheelScrolled& wheel);

    // Fallback for every event type that has no overload above
    template <typename T>
    void HandleEvent(const T&) {}

    sf::RenderWindow window_;
    sf::CircleShape shape_;
    sf::RectangleShape rectangle_;
    sf::Texture texture_;
    sf::RectangleShape player_;
    sf::RectangleShape enemy_;
    sf::Clock clock_;   // measures time between Update() calls
    sf::Font font_;     // must be declared before text_ and outlive it
    sf::Text text_;

    sf::SoundBuffer buffer_;   // holds the sample data; must outlive sound_
    sf::Sound sound_;          // plays buffer_

    sf::View view_;            // the "camera" used for the game world
    float zoom_ = 1.f;         // 1 = normal, <1 = zoomed in, >1 = zoomed out

    sf::Texture backgroundTexture_; // must be declared before background_ and outlive it
    sf::Sprite background_;

};
