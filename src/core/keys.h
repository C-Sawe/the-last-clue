#pragma once
// Input events shared by the platform layer and the game — Caleb (CP3, issue #39)
// The game code never includes GLFW: the window layer translates GLFW keys and
// mouse buttons into these plain types, so all game logic can also run (and be
// tested) without a window.
namespace input {

enum class Key {
    Unknown, Enter, Escape, Backspace, Delete, Tab,
    Left, Right, Up, Down, Home, End, Space,
    R, E, B, D, F1,
};

enum class Button { Left, Right, Middle };

}  // namespace input
