#include "common.hpp"
#include "level.hpp"
#include "player.hpp"
#include "editor.hpp"

int main(int argc, const char** argv) {
  MvWindow window = MvWindow("10 Electrons");
  MvFont timerFont = MvFont("Assets/DsDigii.ttf");
  ball = MvImage("Assets/ball.png");
  tileset = MvImage("Assets/tileset.png");
  playerRed = MvImage("Assets/playerRed.png");
  tileSize = 64;
  Mova::setFont(timerFont, tileSize * 0.6f);

  while (true) {
    loadLevel(levels[levelIndex]);
    player = Player(&playerRed, 5);
    while (finishTimer < 2 && !restart) {
      if (Mova::isKeyPressed(MvKey::Escape)) editor = !editor;
      if (editor) levelEditor(timerFont);
      updateLevel();
      player.update();

      Mova::clear(darkgray);
      for (int x = camera.x / tileSize - 1; x < (camera.x + Mova::getViewportWidth()) / tileSize; x++) {
        for (int y = camera.y / tileSize - 1; y < (camera.y + Mova::getViewportHeight()) / tileSize; y++) drawTile(vec2(x, y));
      }
      for (int x = camera.x / tileSize - 1; x < (camera.x + Mova::getViewportWidth()) / tileSize; x++) {
        for (int y = camera.y / tileSize - 1; y < (camera.y + Mova::getViewportHeight()) / tileSize; y++) drawTile2(vec2(x, y));
      }
      player.draw();
      if(pong) Mova::drawImage(ball, ballPos - camera, vec2(tileSize / 2));

      Mova::nextFrame();
    }
    if (finishTimer >= 2) {
      levelIndex++;
    }
  }
  return 0;
}
