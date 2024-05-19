#include "metro.hpp"
#include "raylib.h"
#include "raymath.h"

Metro::Metro(const Config& cfg)
    : mapWidth(cfg.mapWidth), mapHeight(cfg.mapHeight), cellWidth(20),
      cellHeight(20), stationDistance(120.f),
      lineStations(cfg.numStaionsEachline), numLines(cfg.numLines) {}

bool Metro::insideMap(int x, int y) const {
    if (static_cast<float>(x) < static_cast<float>(mapWidth - 100) / 2.f &&
        static_cast<float>(x) > -static_cast<float>(mapWidth - 100) / 2.f &&
        static_cast<float>(y) < static_cast<float>(mapHeight) / 2.f &&
        static_cast<float>(y) > -static_cast<float>(mapHeight) / 2.f) {
        return true;
    } else {
        return false;
    }
}

int Metro::addStation(Vector2 _pos) {
    auto ret = static_cast<int>(stations.size());
    stations[static_cast<int>(stations.size())] = Station{_pos, {}};
    return ret;
}

void Metro::addEdge(int from, int to, int lineId) {
    assert(from >= 0 && from < stations.size() && to >= 0 &&
           to < stations.size());
    stations[from].lines.insert(lineId);
    stations[to].lines.insert(lineId);
}

void Metro::build() {
    for (int id = 0; id < numLines; ++id) {
        // For each line
        int start = 0;
        if (stations.empty()) {
            start = addStation(Vector2Zero());
        } else {
            std::vector<std::pair<int, Station>> temp;
            for (const auto& pair : stations) {
                temp.push_back(pair);
            }
            std::sort(
                temp.begin(), temp.end(), [](const auto& a, const auto& b) {
                    return Vector2DistanceSqr(Vector2Zero(), a.second.pos) <
                           Vector2DistanceSqr(Vector2Zero(), b.second.pos);
                });

            for (const auto& [id, s] : temp) {
                if (s.lines.size() <= 3) {
                    start = id;
                    if (Random::get<bool>(0.3))
                        break;
                }
            }
        }
        auto l =
            Line{id, std::deque{start},
                 Vector2Normalize(Vector2{Random::get<float>(-1.f, 1.f),
                                          Random::get<float>(-1.f, 1.f)}),

                 Random::get<int>(lineStations - 3 < 1 ? 1 : lineStations - 3,
                                  lineStations + 3)};

        // bool has_tail_snap_and_apart = false;
        // bool has_head_snap_and_apart = false;
        int tail_snap_acc = 0;
        int head_snap_acc = 0;

        const int try_tolerance = 20;
        const int line_try_tolerance = 20;

        // While loop to build a line
        int line_n_try = 0;
        while (l.line.size() < l.desiredNumStations) {
            int n_try = 0;

            // Extend tail
            bool buildTail = false;
            while (!buildTail) {
                auto curId = l.line.back();
                auto& back = stations[curId];
                auto newPos =
                    Vector2Add(back.pos, getDesiredDir(l.dir, stationDistance));
                if (auto near = findNearStation(newPos, stationDistance / 2.f);
                    near) {
                    if (n_try < try_tolerance &&
                        (stations[*near].lines.size() >= 3 ||
                         head_snap_acc > 0 || tail_snap_acc >= 4)) {
                        // Retry
                        ++n_try;
                        continue;
                    }
                    l.line.push_back(*near);
                    addEdge(curId, *near, id);
                    ++tail_snap_acc;
                    buildTail = true;
                } else {
                    int x = static_cast<int>(newPos.x) / cellWidth * cellWidth;
                    int y =
                        static_cast<int>(newPos.y) / cellHeight * cellHeight;

                    if (insideMap(x, y)) {
                        auto next = addStation(Vector2{static_cast<float>(x),
                                                       static_cast<float>(y)});
                        l.line.push_back(next);
                        addEdge(curId, next, id);
                        buildTail = true;
                    } else {
                        // Retry
                        ++n_try;
                    }
                }
                if (n_try >= try_tolerance)
                    break;
            }

            if (l.line.size() >= l.desiredNumStations)
                break;

            // Extend head
            bool buildHead = false;
            while (!buildHead) {
                auto curId = l.line.front();
                auto& front = stations[curId];
                auto newPos =
                    Vector2Add(front.pos, getDesiredDir(Vector2Negate(l.dir),
                                                        stationDistance));
                if (auto near = findNearStation(newPos, stationDistance / 2.f);
                    near) {
                    if (n_try < try_tolerance &&
                        (stations[*near].lines.size() >= 3 ||
                         tail_snap_acc > 0 || head_snap_acc >= 4)) {
                        // Retry
                        ++n_try;
                        continue;
                    }
                    l.line.push_front(*near);
                    addEdge(curId, *near, id);
                    buildHead = true;
                } else {
                    int x = static_cast<int>(newPos.x) / cellWidth * cellWidth;
                    int y =
                        static_cast<int>(newPos.y) / cellHeight * cellHeight;

                    if (insideMap(x, y)) {
                        auto before = addStation(Vector2{
                            static_cast<float>(x), static_cast<float>(y)});
                        l.line.push_front(before);
                        addEdge(curId, before, id);
                        ++head_snap_acc;
                        buildHead = true;
                    } else {
                        // Retry
                        ++n_try;
                    }
                }
                if (n_try >= try_tolerance) {
                    ++line_n_try;
                    break;
                }
            }

            if (line_n_try >= line_try_tolerance) {
                l.desiredNumStations = l.line.size();
                break;
            }
        }

        // Successfully build a new subway line
        lines.push_back(l);
    }
}

std::optional<int> Metro::findNearStation(const Vector2& _pos,
                                          float range) const {
    int nearestStation = 0;
    int nearestDistSqr = INT_MAX;
    bool changed = false;
    for (const auto& [id, s] : stations) {
        auto newDistSqr = Vector2DistanceSqr(_pos, s.pos);
        if (newDistSqr < nearestDistSqr) {
            nearestDistSqr = newDistSqr;
            nearestStation = id;
            changed = true;
        }
    }
    if (changed && nearestDistSqr <= range * range) {
        return nearestStation;
    }
    return std::nullopt;
}

void Metro::draw(Camera2D cam) const {
    // Draw map
    DrawRectangleV(
        Vector2{-mapWidth / 2.f, -mapHeight / 2.f},
        Vector2{static_cast<float>(mapWidth), static_cast<float>(mapHeight)},
        Color{242, 230, 206, 255});

    // Draw lines
    for (const auto& l : lines) {
        auto offset = Vector2Zero();
        if (l.id % 2 == 0)
            offset = Vector2{static_cast<float>(l.id * 1.5),
                             static_cast<float>(l.id * 1.5)};
        else
            offset = Vector2{-static_cast<float>(l.id * 1.5),
                             -static_cast<float>(l.id * 1.5)};
        int cnt = 0;
        for (auto it = l.line.begin(); it != l.line.end() - 1; ++it) {
            auto next_it = std::next(it);
            auto a = Vector2Add(offset, stations.at(*it).pos);
            auto b = Vector2Add(offset, stations.at(*next_it).pos);

            if (abs(a.x - b.x) < EPS || abs(a.y - b.y) < EPS) {
                DrawLineEx(a, b, 4.f, PALETTE[l.id]);
            } else {
                auto middle = a;
                if (abs(a.x - b.x) < abs(a.y - b.y)) {
                    int m = 0;
                    if (b.y < a.y)
                        m = -1;
                    else
                        m = 1;
                    if (cnt % 2 == 0)
                        middle = Vector2{b.x, m * abs(a.x - b.x) + a.y};
                    else
                        middle = Vector2{a.x, -m * abs(a.x - b.x) + b.y};
                    DrawLineEx(a, middle, 4.f, PALETTE[l.id]);
                    DrawLineEx(middle, b, 4.f, PALETTE[l.id]);
                } else {
                    int m = 0;
                    if (b.x < a.x)
                        m = -1;
                    else
                        m = 1;
                    if (cnt % 2 == 1)
                        middle = Vector2{m * abs(b.y - a.y) + a.x, b.y};
                    else
                        middle = Vector2{-m * abs(b.y - a.y) + b.x, a.y};
                    DrawLineEx(a, middle, 4.f, PALETTE[l.id]);
                    DrawLineEx(middle, b, 4.f, PALETTE[l.id]);
                }
            }
            ++cnt;
        }
    }

    // Draw stations
    for (const auto& [_, s] : stations) {
        // DrawCircleV(s.pos, 10.f, BLACK);
        // DrawCircleV(s.pos, 7.f, RAYWHITE);
        if (s.lines.size() == 1) {
            auto lineId = *s.lines.begin();
            auto offset = Vector2Zero();
            if (lineId % 2 == 0)
                offset = Vector2{static_cast<float>(lineId * 1.5),
                                 static_cast<float>(lineId * 1.5)};
            else
                offset = Vector2{-static_cast<float>(lineId * 1.5),
                                 -static_cast<float>(lineId * 1.5)};
            auto actualPos = Vector2Add(offset, s.pos);
            DrawCircleV(actualPos, 9.f, BLACK);
            DrawCircleV(actualPos, 6.f, RAYWHITE);
        } else {
            float maxOffset = lines.size() - 1;
            DrawCircleV(Vector2Subtract(s.pos, Vector2{maxOffset, maxOffset}),
                        9.f, BLACK);
            DrawCircleV(Vector2Add(s.pos, Vector2{maxOffset, maxOffset}), 9.f,
                        BLACK);
            DrawLineEx(Vector2Subtract(s.pos, Vector2{maxOffset, maxOffset}),
                       Vector2Add(s.pos, Vector2{maxOffset, maxOffset}), 18.f,
                       BLACK);

            DrawCircleV(Vector2Subtract(s.pos, Vector2{maxOffset, maxOffset}),
                        6.f, RAYWHITE);
            DrawCircleV(Vector2Add(s.pos, Vector2{maxOffset, maxOffset}), 6.f,
                        RAYWHITE);
            DrawLineEx(Vector2Subtract(s.pos, Vector2{maxOffset, maxOffset}),
                       Vector2Add(s.pos, Vector2{maxOffset, maxOffset}), 12.f,
                       RAYWHITE);
        }
    }
}

Vector2 getDesiredDir(const Vector2& dir, float len) {
    auto x = Vector2Normalize(Vector2{dir.x, 0.f});
    auto y = Vector2Normalize(Vector2{0.f, dir.y});

    if (Random::get<bool>(0.5)) {
        // Go straight
        if (Random::get<bool>(std::abs(dir.x))) {
            // Go horizontal
            return Vector2Scale(x, len);
        } else {
            // Go vertical
            return Vector2Scale(y, len);
        }
    } else {
        return Vector2Add(Vector2Scale(dir, len),
                          Vector2{Random::get<float>(-len / 8.f, len / 8.f),
                                  Random::get<float>(-len / 8.f, len / 8.f)});
    }
}