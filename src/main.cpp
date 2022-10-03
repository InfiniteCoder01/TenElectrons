#include "common.hpp"
#include "level.hpp"
#include "player.hpp"
#include "editor.hpp"

int main(int argc, const char** argv) {
  MvWindow window = MvWindow("10 Electrons");
  MvFont timerFont = MvFont("Assets/DsDigii.ttf");
  MvFont simpleFont = MvFont("Assets/font.ttf");
  ball = MvImage("Assets/ball.png");
  tileset = MvImage("Assets/tileset.png");
  playerRed = MvImage("Assets/playerRed.png");
  tileSize = 64;
  Mova::setFont(timerFont, tileSize * 0.6f);
  levels[4] = createLevel(25, 20);

  while (true) {
    loadLevel(levels[levelIndex]);
    player = Player(&playerRed, 5);
    if (levelIndex == 4) {
      Mova::setFont(simpleFont, tileSize * 0.6f);
      message =
          "Congratulations for beating the game!\n"
          "Did you found an easter egg in morse code?\n"
          "If so, go to link https://infinitecoder01.github.io/TenElectrons/html/XX,\n"
          "replacing XX with secret code (upper case)!\n"
          "Made by InfiniteCoder for\n"
          "Ludum Dare 51!\n"
          "I've used custom rendering framework\n"
          "called \"Mova\",\n"
          "you can find it on my github (InfiniteCoder01)!";
    }
    while (finishTimer < 2 && !restart) {
      if (Mova::isKeyPressed(MvKey::Escape)) editor = !editor;
      if (editor) levelEditor(timerFont);
      updateLevel();
      player.update();

      Mova::clear(levelIndex == 4 ? Color(135, 206, 235) : darkgray);
      for (int x = camera.x / tileSize - 1; x < (camera.x + Mova::getViewportWidth()) / tileSize; x++) {
        for (int y = camera.y / tileSize - 1; y < (camera.y + Mova::getViewportHeight()) / tileSize; y++) drawTile(vec2(x, y));
      }
      for (int x = camera.x / tileSize - 1; x < (camera.x + Mova::getViewportWidth()) / tileSize; x++) {
        for (int y = camera.y / tileSize - 1; y < (camera.y + Mova::getViewportHeight()) / tileSize; y++) drawTile2(vec2(x, y));
      }
      player.draw();
      if (pong) Mova::drawImage(ball, ballPos - camera, vec2(tileSize / 2));
      if (messageShown) {
        std::string _message = message;
        int height = 10;
        while (true) {
          size_t newline = _message.find('\n');
          std::string line = _message;
          if (newline != std::string::npos) {
            line = _message.substr(0, newline);
            _message = _message.substr(newline + 1);
          }
          Mova::drawText(Mova::getViewportWidth() / 2 - Mova::textWidth(line) / 2, height, line, levelIndex == 4 ? Color(0, 8, 48) : green);
          height += Mova::textHeight(line);
          if (newline == std::string::npos) break;
          messageShown = false;
        }
      }

      Mova::nextFrame();
    }
    if (finishTimer >= 2) {
      levelIndex++;
    }
  }
  return 0;
}
