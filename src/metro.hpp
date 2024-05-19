#ifndef METRO_HPP
#define METRO_HPP

#include <deque>
#include <effolkronium/random.hpp>
#include <map>
#include <optional>
#include <raylib.h>
#include <raymath.h>
#include <set>

using Random = effolkronium::random_static;

constexpr static const float EPS = 0.00001f;

Vector2 getDesiredDir(const Vector2& dir, float len);

struct Config {
    int mapWidth = 1000;
    int mapHeight = 1000;
    int numLines = 7;
    int numStaionsEachline = 9;
};

struct Station {
    Vector2 pos;
    std::set<int> lines;
};

struct Line {
    int id;
    std::deque<int> line;
    Vector2 dir;
    int desiredNumStations;
};

Vector2 getDesiredDir(const Vector2& dir, float len);

class Metro {
  public:
    Metro(const Config& cfg);
    void build();
    void draw(Camera2D cam) const;

  private:
    int mapWidth;
    int mapHeight;

    int cellWidth;
    int cellHeight;

    float stationDistance;
    int numLines;
    int lineStations;

    std::map<int, Station> stations;
    std::vector<Line> lines;

  private:
    bool insideMap(int x, int y) const;
    int addStation(Vector2 _pos);
    void addEdge(int from, int to, int lineId);
    std::optional<int> findNearStation(const Vector2& _pos, float range) const;
};

const Color PALETTE[] = {Color{229, 20, 0, 255},   Color{96, 169, 23, 255},
                         Color{227, 200, 0, 255},  Color{0, 80, 239, 255},
                         Color{250, 104, 0, 255},  Color{170, 0, 255, 255},
                         Color{130, 90, 44, 255},  Color{162, 0, 37, 255},
                         Color{164, 196, 0, 255},  Color{27, 161, 226, 255},
                         Color{240, 163, 10, 255}, Color{100, 118, 135, 255},
                         Color{0, 138, 0, 255},    Color{216, 0, 115, 255},
                         Color{160, 82, 45, 255},  Color{0, 171, 169, 255},
                         Color{106, 0, 255, 255},  Color{109, 135, 100, 255},
                         Color{118, 96, 138, 255}, Color{244, 114, 208, 255}};

#endif // METRO_HPP