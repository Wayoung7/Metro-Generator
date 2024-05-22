#ifndef UI_HPP
#define UI_HPP

#include "metro.hpp"
#include <raygui.h>
#include <raylib.h>

enum UIAction {
    NullAction,
    GenerateAction,
    ExportImageAction,
};

class UI {
  public:
    inline UIAction handleUI(Config& cfg) {
        if (WindowBoxActive) {
            WindowBoxActive = !GuiWindowBox(layoutRecs[0], WindowBoxText);
            if (GuiButton(layoutRecs[1], GenerateButtonText))
                return GenerateAction;
            GuiLine(layoutRecs[2], "SETTINGS");
            if (GuiValueBox(layoutRecs[3], MapWidthValueBoxText, &cfg.mapWidth,
                            100, 10000, MapWidthValueBoxEditMode))
                MapWidthValueBoxEditMode = !MapWidthValueBoxEditMode;
            if (GuiValueBox(layoutRecs[4], MapHeightValueBoxText,
                            &cfg.mapHeight, 100, 10000,
                            MapHeightValueBoxEditMode))
                MapHeightValueBoxEditMode = !MapHeightValueBoxEditMode;
            if (GuiSpinner(layoutRecs[5], StationsSpinnerText, &cfg.numStaions,
                           5, 100, StationsSpinnerEditMode))
                StationsSpinnerEditMode = !StationsSpinnerEditMode;
            if (GuiSpinner(layoutRecs[6], LinesSpinnerText, &cfg.numLines, 1,
                           100, LinesSpinnerEditMode))
                LinesSpinnerEditMode = !LinesSpinnerEditMode;
            if (GuiButton((layoutRecs[7]), "Export Image")) {
                return ExportImageAction;
            }
        }
        return NullAction;
    }

  private:
    const char* WindowBoxText = "Metro Generator";
    const char* GenerateButtonText = "GENERATE";
    const char* LinesSpinnerText = "Lines";
    const char* StationsSpinnerText = "Stations";
    const char* MapWidthValueBoxText = "MapWidth";
    const char* MapHeightValueBoxText = "MapHeight";

    Vector2 anchor01 = Vector2{20, 20};

    bool WindowBoxActive = true;
    bool MapWidthValueBoxEditMode = false;
    bool MapHeightValueBoxEditMode = false;
    bool StationsSpinnerEditMode = false;
    bool LinesSpinnerEditMode = false;

    Rectangle layoutRecs[8] = {
        Rectangle{anchor01.x + 0, anchor01.y + 0, 240, 392},
        Rectangle{anchor01.x + 24, anchor01.y + 40, 192, 48},
        Rectangle{anchor01.x + 0, anchor01.y + 96, 240, 12},
        Rectangle{anchor01.x + 64, anchor01.y + 224, 152, 24},
        Rectangle{anchor01.x + 64, anchor01.y + 272, 152, 24},
        Rectangle{anchor01.x + 64, anchor01.y + 176, 152, 24},
        Rectangle{anchor01.x + 64, anchor01.y + 128, 152, 24},
        Rectangle{anchor01.x + 24, anchor01.y + 320, 192, 48},
    };
};

#endif // UI_HPP