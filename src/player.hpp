#include "common.hpp"

MvImage playerRed;
float finishTimer;
bool messageShown = false;
struct Player {
  MvImage* image;
  Flip flip = FLIP_NONE;

  int frames, frameWidth;
  float frame = 0;

  vec2 pos = vec2(0), size, speed = vec2(0);
  int jumps = 0;
  vec2 door;

  Player() = default;
  Player(MvImage* image, int frames) : image(image), frames(frames), door(-1, -1) {
    frameWidth = image->width / frames;
    completed = false;
    restart = false;
    pong = false;
    ballPos = vec2(-1, -1);
    finishTimer = 0;
    pos.y = levelHeight * tileSize;
  }

  bool collides(vec2 delta) {
    if (editor) return false;
    float pixelBorder = tileSize / TILE_SIZE * 3;
    vec2 from = pos + pixelBorder, to = pos + size - pixelBorder;
    for (int x = from.x / tileSize + 1; x < to.x / tileSize + 1; x++) {
      for (int y = from.y / tileSize + 1; y < to.y / tileSize + 1; y++) {
        vec2 pos = vec2(x - 1, y - 1);
        float yOff = (Player::pos.y + size.y) / tileSize - pos.y;
        char tile = getTile(pos);
        if (tile == '#' || tile == 'g') return true;
        if (tile == '^' && yOff > (getPower(pos) > 0 ? 0.375 : 0.5625)) {
          if (getPower(pos) > 0 && delta.y > 0) speed.y = -20 * tileSize;
          return true;
        }
        if (tile == '|') {
          if (getPower(pos) < 0.625 && getPower(pos) > 0.5) {
            completed = true;
            door = pos;
          } else if (!completed) return true;
        }
        if (Mova::isKeyHeld(MvKey::E)) {
          if (tile == 'r') setTile(pos, ' ');
          if (tile == 'h' || tile == 'f') messageShown = true;
        }
      }
    }
    return false;
  }

  void update() {
    while (collides(vec2(0, 1))) pos.y--;
    size = vec2(tileSize, tileSize * image->height / frameWidth);
    if (editor) speed = MGame::keyboardJoy() * tileSize * 8.f;
    else {
      speed.x = MGame::keyboardX() * tileSize * 8;
      speed.y += tileSize * 15 * Mova::deltaTime();
      if (Mova::isKeyPressed(MvKey::Space) || Mova::isKeyPressed(MvKey::W) || Mova::isKeyPressed(MvKey::ArrowUp)) {
        if (jumps) {
          jumps--;
          speed.y = tileSize * -10;
        }
      }
    }
    if (Mova::isKeyPressed(MvKey::R) && Mova::isKeyHeld(MvKey::ControlLeft)) restart = true;
    if (completed && door.x != -1) speed = vec2(tileSize * 4, 0);
    if (Mova::isKeyHeld(MvKey::ShiftLeft)) {
      frame = frames - 1;
    } else if (abs(speed.x) >= 1) {
      flip = speed.x < 0 ? FLIP_HORIZONTAL : FLIP_NONE;
      frame += Mova::deltaTime() * 8;
      if (frame >= frames - 1) {
        frame = 1;
      }
    } else {
      frame = 0;
    }

    vec2 step = glm::sign(speed) * 0.5f;
    for (float i = 0; i < abs(speed.x * Mova::deltaTime()); i += 0.5) {
      pos.x += step.x;
      if (collides(vec2(step.x, 0))) {
        pos.x -= step.x;
        speed.x = 0;
        break;
      }
    }
    for (float i = 0; i < abs(speed.y * Mova::deltaTime()); i += 0.5) {
      pos.y += step.y;
      if (collides(vec2(0, step.y))) {
        pos.y -= step.y;
        if (speed.y > 0) jumps = 2;
        speed.y = 0;
        break;
      }
    }

    if (ballPos.x != -1 && !pong) {
      if (pos.x > ballPos.x && pos.x + size.x < ballPos.x + tileSize * 7) {
        if (pos.y > ballPos.y && pos.y < ballPos.y + tileSize * 5) {
          pong = true;
          for (int i = 0; i < 5; i++) {
            setTile(pongPos + vec2(-1, i), '#');
            setTile(pongPos + vec2(8, i), '#');
          }
          ballPos.x += tileSize;
          ballVel = vec2(1, 1);
        }
      }
    }
    if (pong) {
      if (ballPos.x + tileSize / 2 > pos.x && ballPos.x < pos.x + size.x) {
        if (ballPos.y > pos.y && ballPos.y + tileSize / 2 < pos.y + size.y) {
          if (ballVel.x != (flip == FLIP_HORIZONTAL ? -1 : 1)) {
            ballVel.x *= -1;
          }
        }
      }
    }
    if (pongFail) {
      for (int i = 0; i < 5; i++) {
        setTile(pongPos + vec2(-1, i), ' ');
        setTile(pongPos + vec2(8, i), ' ');
      }
      pos.x += tileSize * 7;
      ballPos = vec2(-1, -1);
      ballVel = vec2(1, 1);
      pong = false;
      pongFail = false;
    }
    if (pongWin) {
      for (int i = 0; i < 5; i++) {
        setTile(pongPos + vec2(-1, i), ' ');
        setTile(pongPos + vec2(8, i), ' ');
      }
      setTile(pongPos, ' ');
      ballPos = vec2(-1, -1);
      ballVel = vec2(1, 1);
      pong = false;
      pongWin = false;
    }

    camera = pos + size / 2.f - Mova::getViewportSize() / 2.f;
    camera = glm::max(vec2(tileSize * -2), camera);
    camera = glm::min(vec2(levelWidth + 2, levelHeight + 2) * tileSize - Mova::getViewportSize(), camera);
  }

  void draw() {
    float in = 0;
    if (completed) {
      finishTimer += Mova::deltaTime();
      if (door.x != -1) in = std::min(std::max((pos.x + size.x) / tileSize - door.x - 1 + 2 / TILE_SIZE, 0.f), 1.f);
    }
    Mova::drawImage(*image, pos - camera, size - vec2(in, 0) * tileSize, flip, vec2(((int)frame) * frameWidth, 0), vec2(frameWidth - in * TILE_SIZE, image->height));
  }
};

Player player;
