#include "common.hpp"
#include <string.h>
#include <queue>

const float TILE_SIZE = 16;
float tileSize;
MvImage tileset, ball;
bool editor = false, completed = false, restart = false, pong = false, pongFail = false, pongWin = false;

vec2 camera;
uint32_t levelWidth, levelHeight, levelIndex = 2;
std::string levels[] = {
    "19 ~128 11e9 e2T6E|e9 9e|e ;400a;",
    "19 ~14 6e14 2e|3e14 2e|E2T11 9#11 9#210 ^18 eE9e9 e7E2Te9 11e;800a;",
    "99 ~700 39#7 47#7 39#7 47#7 39#7 47#7 39#7 47#7 39#7 47#7 39#7 47#7 #6 PE2Tr21 6#7 47#7 #9 2E34 47#7 #45 47#7 #45 47#7 #32 6#7 55# 37#7 47#2T4E2# 37#7 52#E2# 37#7 52#E2# 37#7 52#E2# 37#7 52#E2# 37#34 25#E2# 37#33 h25#E2# 96#E2#96 eEe#96 e|e#96 e|e;2823ab3176a;2s",
};
std::string llevel, message;
float* lpower = nullptr;
int* lprop = nullptr;
vec2 ballPos = vec2(-1, -1), ballVel, pongPos;

std::string createLevel(int width, int height) { return std::to_string(width - 1) + " ~" + std::to_string(width * (height - 1)) + " ;" + std::to_string(width * height * 2) + "a;"; }

void loadLevel(std::string level) {
  level = decodeRLE(level);
  message = level.substr(level.find_last_of(";") + 1);
  level = level.substr(0, level.find_last_of(";"));
  std::string props = level.substr(level.find(';') + 1);
  level = level.substr(0, level.find(';'));
  levelWidth = level.find_first_of("~") + 1;
  levelHeight = level.length() / levelWidth;
  llevel = level;
  if (lpower) delete[] lpower;
  if (lprop) delete[] lprop;
  lpower = new float[levelWidth * levelHeight];
  lprop = new int[levelWidth * levelHeight];
  for (int i = 0; i < llevel.size(); i++) {
    if (llevel[i] == '~') llevel[i] = ' ';
    lpower[i] = 0;
    lprop[i] = (props[i * 2] - 'a') << 4 | (props[i * 2 + 1] - 'a');
  }
}

char getTile(vec2 pos) {
  if (pos.x < 0 || pos.y < 0 || pos.x >= levelWidth || pos.y >= levelHeight) return '#';
  return llevel[pos.x + pos.y * levelWidth];
}

void setTile(vec2 pos, char tile) {
  if (pos.x < 0 || pos.y < 0 || pos.x >= levelWidth || pos.y >= levelHeight) return;
  llevel[pos.x + pos.y * levelWidth] = tile;
}

int getProp(vec2 pos) {
  if (pos.x < 0 || pos.y < 0 || pos.x >= levelWidth || pos.y >= levelHeight) return 0;
  return lprop[(int)(pos.x + pos.y * levelWidth)];
}

void setProp(vec2 pos, int value) {
  if (pos.x < 0 || pos.y < 0 || pos.x >= levelWidth || pos.y >= levelHeight) return;
  lprop[(int)(pos.x + pos.y * levelWidth)] = value;
}

float getPower(vec2 pos) {
  if (pos.x < 0 || pos.y < 0 || pos.x >= levelWidth || pos.y >= levelHeight) return 0;
  return lpower[(int)(pos.x + pos.y * levelWidth)];
}

void setPower(vec2 pos, float value) {
  if (pos.x < 0 || pos.y < 0 || pos.x >= levelWidth || pos.y >= levelHeight) return;
  lpower[(int)(pos.x + pos.y * levelWidth)] = value;
}

void drawQuater(vec2 screen, vec2 pos, vec2 atlas, vec2 off, const char* accepted) {  // Thanks Kofybreak (Youtube) for idea to split tiles into quaters
  vec2 delta = off * 2.f - 1.f;
  char tile = getTile(pos);
  if (!strchr(accepted, getTile(pos + vec2(delta.x, 0)))) {
    atlas.x += (!strchr(accepted, getTile(pos + vec2(0, delta.y))) ? 4 : 1);
  } else if (!strchr(accepted, getTile(pos + vec2(0, delta.y)))) {
    atlas.x += 3;
  } else {
    atlas.x += (!strchr(accepted, getTile(pos + delta)) ? 2 : 0);
  }
  Mova::drawImage(tileset, screen + off * tileSize / 2.f, vec2(tileSize / 2.f), FLIP_NONE, atlas * TILE_SIZE + off * TILE_SIZE / 2.f, vec2(TILE_SIZE / 2.f));
}

void drawPatch(vec2 screen, vec2 pos, vec2 atlas, const char* accepted) {
  drawQuater(screen, pos, atlas, vec2(0, 0), accepted);
  drawQuater(screen, pos, atlas, vec2(1, 0), accepted);
  drawQuater(screen, pos, atlas, vec2(0, 1), accepted);
  drawQuater(screen, pos, atlas, vec2(1, 1), accepted);
}

void drawTile(vec2 pos) {
  char tile = getTile(pos);
  vec2 screen = pos * tileSize - camera;
  float pixel = tileSize / TILE_SIZE;
  if (tile == '#' || tile == '~') {
    drawPatch(screen, pos, vec2(0, 0), "#~");
  } else if (tile == 'E' || tile == 'e') {
    drawPatch(screen, pos, vec2(0, 1), "EeT|");
  } else if (tile == 'T') {
    if (getTile(pos + vec2(1, 0)) != 'T') return;
    Mova::drawImage(tileset, screen, vec2(2, 1) * tileSize, FLIP_NONE, vec2(0, 4) * TILE_SIZE, vec2(2, 1) * TILE_SIZE);
    char buf[6];
    sprintf(buf, "%02d:%02d", (int)getPower(pos), (int)(getPower(pos) * 100) % 100);
    Mova::drawText(screen + vec2(5, 5) * pixel, buf, green);
  } else if (tile == 'r') {
    Mova::drawImage(tileset, screen, vec2(tileSize), FLIP_NONE, vec2(2, 4) * TILE_SIZE, vec2(TILE_SIZE));
    if (getPower(pos) == 0) Mova::drawText(screen + vec2(5, 5) * pixel, std::to_string(getProp(pos)), green);
  } else if (tile == '|') {
    if (getTile(pos + vec2(0, 1)) != '|') return;
    if (completed && getPower(pos) < 0.5 || getPower(pos) > 1) setPower(pos, 0.5);
    float height = abs(getPower(pos) * 4 - 2);
    Mova::drawImage(tileset, screen, vec2(1, height) * tileSize, FLIP_NONE, vec2(4, 6 - height) * TILE_SIZE, vec2(1, height) * TILE_SIZE);
  } else if (tile == '^') {
    Mova::drawImage(tileset, screen, vec2(tileSize), FLIP_NONE, vec2(getPower(pos) > 0, 5) * TILE_SIZE, vec2(TILE_SIZE));
  } else if (tile == 'P') {
    float height = 0;
    pongPos = pos;
    if (getPower(pos) > 0) {
      if (!pong) ballPos = pos * tileSize;
      else height = ballPos.y - pos.y * tileSize - tileSize / 4;
    }
    if (pong) {
      ballPos += ballVel * Mova::deltaTime() * tileSize * 4.f;
      float left = ballPos.x / tileSize, right = ballPos.x / tileSize + 0.5f, top = ballPos.y / tileSize, bottom = ballPos.y / tileSize + 0.5f;
      if (getTile(vec2((int)left, (int)top)) == '#' && getTile(vec2((int)left, (int)bottom)) == '#') pongWin = true;
      if (getTile(vec2((int)right, (int)top)) == '#' && getTile(vec2((int)right, (int)bottom)) == '#') pongFail = true;
      if (getTile(vec2((int)left, (int)top)) == '#' && getTile(vec2((int)right, (int)top)) == '#') ballVel.y = 1;
      if (getTile(vec2((int)left, (int)bottom)) == '#' && getTile(vec2((int)right, (int)bottom)) == '#') ballVel.y = -1;
      float ballHeight = ballPos.y - pos.y * tileSize;
      if (left < pos.x + 0.75f && ballHeight + tileSize / 2 > height && ballHeight < height + tileSize) ballVel.x = 1;
    }
    Mova::drawLine(screen + vec2(tileSize / 2, 0), screen + vec2(tileSize / 2, tileSize * 5.5f), black);
    Mova::drawImage(tileset, screen + vec2(0, height), vec2(tileSize), FLIP_NONE, vec2(2, 5) * TILE_SIZE, vec2(TILE_SIZE));
  } else if(tile == 'h') {
    Mova::drawImage(tileset, screen, vec2(tileSize), FLIP_NONE, vec2(0, 6) * TILE_SIZE, vec2(TILE_SIZE));
  }
}

void drawTile2(vec2 pos) {
  char tile = getTile(pos);
  vec2 screen = pos * tileSize - camera;
  float pixel = tileSize / TILE_SIZE;
  if (tile == 'E') {
    drawPatch(screen, pos, vec2(0, 2 + (getPower(pos) > 0)), "ETr|P");
  } else if (tile == '|') {
    if (getTile(pos + vec2(0, 1)) != '|') return;
    Mova::drawImage(tileset, screen, vec2(1, 2) * tileSize, FLIP_NONE, vec2(3, 4) * TILE_SIZE, vec2(1, 2) * TILE_SIZE);
  }
}

void updateLevel() {
  std::queue<vec2> q;
  std::vector<vec2> dirs = {vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1)};
  std::vector<vec2> dirs2 = {vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1), vec2(1, 1), vec2(1, -1), vec2(2, 0)};
  for (int x = 0; x < levelWidth; x++) {
    for (int y = 0; y < levelHeight; y++) {
      if (strchr("Er|^P", getTile(vec2(x, y)))) setPower(vec2(x, y), 0);
      if (strchr("T", getTile(vec2(x, y)))) q.push(vec2(x, y));
    }
  }
  while (!q.empty()) {
    vec2 pos = q.front();
    q.pop();
    if (getTile(pos) == 'T') {
      if (!completed || getPower(pos) < 5) setPower(pos, getPower(pos) - Mova::deltaTime());
      if (getPower(pos) < 0) setPower(pos, 10);
      float power = getPower(pos) <= 1 ? getPower(pos) : 0;
      for (vec2& dir : dirs) {
        if (getTile(pos + dir) == 'E') {
          if (getPower(pos + dir) >= power) continue;
          setPower(pos + dir, power);
          q.push(pos + dir);
        }
      }
    } else if (getTile(pos) == 'E') {
      for (vec2& dir : dirs) {
        if (!strchr("Er|^P", getTile(pos + dir))) continue;
        if (getPower(pos + dir) >= getPower(pos)) continue;
        setPower(pos + dir, getPower(pos));
        q.push(pos + dir);
      }
    } else if (getTile(pos) == 'r') {
      if (getPower(pos) <= 0) continue;
      if (getTile(pos + vec2(-1, 0)) != 'T') continue;
      setPower(pos + vec2(-1, 0), getProp(pos));
      setPower(pos + vec2(-2, 0), getProp(pos));
    } else if (getTile(pos) == '|') {
      if (getTile(pos + vec2(0, 1)) == '|') setPower(pos + vec2(0, 1), getPower(pos));
      else setPower(pos + vec2(0, -1), getPower(pos));
    }
  }
}
