#include "common.hpp"
#include <emscripten.h>
#include <deque>

void levelEditor(MvFont& timerFont) {
  static std::string paletteMessage;
  const char* brushes = "#EeTr|^Phg";
  static char brush = '#';
  if (Mova::getCharPressed() && strchr(brushes, Mova::getCharPressed())) brush = Mova::getCharPressed();
  vec2 mouseTile = glm::floor(vec2(Mova::getMousePos() + camera) / tileSize);
  if (Mova::isMouseButtonHeld(MOUSE_LEFT)) {
    if (Mova::isKeyHeld(MvKey::ControlLeft)) {
      std::deque<vec2> q;
      q.push_back(mouseTile);
      char target = getTile(mouseTile);
      if (target == brush) return;
      std::vector<vec2> dirs = {vec2(1, 0), vec2(-1, 0), vec2(0, 1), vec2(0, -1)};

      while (!q.empty()) {
        vec2 tile = q.front();
        q.pop_front();
        if (tile.x < 0 || tile.x >= levelWidth || tile.y < 0 || tile.y >= levelHeight) continue;
        setTile(tile, brush);
        setProp(tile, 0);
        setPower(tile, 0);
        for (vec2 dir : dirs) {
          if (getTile(tile + dir) == target && std::find(q.begin(), q.end(), tile + dir) == q.end()) q.push_back(tile + dir);
        }
      }
      return;
    }
    setTile(mouseTile, brush);
    setProp(mouseTile, 0);
    setPower(mouseTile, 0);
    if (strchr("T", brush)) {
      setTile(mouseTile + vec2(1, 0), brush);
      setPower(mouseTile + vec2(1, 0), 0);
    }
    if (strchr("|", brush)) setTile(mouseTile + vec2(0, 1), brush);
  } else if (Mova::isMouseButtonHeld(MOUSE_RIGHT)) {
    if (Mova::isKeyHeld(MvKey::ShiftLeft)) {
      brush = getTile(mouseTile);
      return;
    }
    if (strchr("T", getTile(mouseTile)) && getTile(mouseTile + vec2(1, 0)) == getTile(mouseTile)) setTile(mouseTile + vec2(1, 0), ' ');
    if (strchr("T", getTile(mouseTile)) && getTile(mouseTile + vec2(-1, 0)) == getTile(mouseTile)) setTile(mouseTile + vec2(-1, 0), ' ');
    if (strchr("|", getTile(mouseTile)) && getTile(mouseTile + vec2(0, 1)) == getTile(mouseTile)) setTile(mouseTile + vec2(0, 1), ' ');
    if (strchr("|", getTile(mouseTile)) && getTile(mouseTile + vec2(0, -1)) == getTile(mouseTile)) setTile(mouseTile + vec2(0, -1), ' ');
    setTile(mouseTile, ' ');
  }
  if (Mova::isKeyPressed(MvKey::Tab) || paletteMessage == "reset") {
    int width = EM_ASM_INT(return parseInt(prompt("Level Width:")););
    int height = EM_ASM_INT(return parseInt(prompt("Level Height:")););
    if (!width || !height) return;
    loadLevel(createLevel(width, height));
  }

  if (Mova::isKeyHeld(MvKey::ShiftRight) && Mova::isKeyPressed(MvKey::Enter) || paletteMessage == "load") {
    // clang-format off
    std::string level = std::string((char*)EM_ASM_PTR({
      var text = prompt("Level Data:");
      var lengthBytes = lengthBytesUTF8(text) + 1;
      var stringOnWasmHeap = _malloc(lengthBytes);
      stringToUTF8(text, stringOnWasmHeap, lengthBytes);
      return stringOnWasmHeap;
    }));
    // clang-format on
    level.erase(level.begin());
    level.pop_back();
    loadLevel(level);
  } else if (Mova::isKeyPressed(MvKey::Enter) || paletteMessage == "save") {
    std::string save = '\\' + llevel + ';';
    save[levelWidth] = '~';
    save.reserve(levelWidth * levelHeight * 2);
    for (int i = 0; i < levelWidth * levelHeight; i++) {
      save += (lprop[i] >> 4) + 'a';
      save += (lprop[i] & 0xf) + 'a';
    }
    save += ';' + message;
    save += '\\';
    save = encodeRLE(save);
    EM_ASM(navigator.clipboard.writeText(UTF8ToString($0)); alert("Level code is copied to clipboard!"), save.c_str());
  }
  if (Mova::isKeyPressed(MvKey::F2) || paletteMessage == "message") {
    message = std::string((char*)EM_ASM_PTR({
      var text = prompt("New sign message (Use \\\\n to put newline character):").replace("\\\\n", "\\n");
      var lengthBytes = lengthBytesUTF8(text) + 1;
      var stringOnWasmHeap = _malloc(lengthBytes);
      stringToUTF8(text, stringOnWasmHeap, lengthBytes);
      return stringOnWasmHeap;
    }));
  }
  if (strchr("r", getTile(mouseTile))) {
    setProp(mouseTile, getProp(mouseTile) + Mova::getScrollY());
  }
  paletteMessage = "";
  if (Mova::isKeyPressed(MvKey::F1)) {
    paletteMessage = std::string((char*)EM_ASM_PTR({
      var text = prompt("Enter command (load - load level (Shift+Enter), save - save level (Enter), reset - reset level (Tab), message(F2) - set sign message for this level):");
      var lengthBytes = lengthBytesUTF8(text) + 1;
      var stringOnWasmHeap = _malloc(lengthBytes);
      stringToUTF8(text, stringOnWasmHeap, lengthBytes);
      return stringOnWasmHeap;
    }));
    std::transform(paletteMessage.begin(), paletteMessage.end(), paletteMessage.begin(), ::tolower);
  }
}
