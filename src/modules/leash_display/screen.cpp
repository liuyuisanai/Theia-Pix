#include "screen.hpp"

#include <stdio.h>
#include <string.h>
#include <display.h>
#include <uORB/topics/leash_display.h>

#include "block.hpp"
#include "images/images.h"
#include "font.hpp"
#include "errormessages.hpp"

struct TextInfo {
    const char *text;
    Font *font;
};

enum DrawMode
{
    DrawMode_CenterXY,
    DrawMode_LeftTop,
};

static void drawText(int mode, struct TextInfo *text, int rowCount, int yc, int xc, int *yStart = nullptr)
{
    int height = 0;
    int width = 0;

    // calculate total height
    height = 0;
    for (int i = 0; i < rowCount; i++)
    {
        if (text[i].text != nullptr)
        {
            height += text[i].font->getTextHeight(text[i].text) + 2;
        }
    }

    int y = 0;

    switch (mode)
    {
        case DrawMode_CenterXY:
            y = yc - height / 2;
            break;

        case DrawMode_LeftTop:
            y = yc;
            break;
    }


    if (yStart != nullptr)
    {
        *yStart = y;
    }

    for (int i = 0; i < rowCount; i++)
    {
        if (text[i].text != nullptr)
        {
            int x = 0;

            switch (mode)
            {
                case DrawMode_CenterXY:
                    width = text[i].font->getTextWidth(text[i].text);
                    x = 64 - width / 2;
                    break;

                case DrawMode_LeftTop:
                    x = xc;
                    break;
            }


            TextBlock blockText(text[i].text, x, y, text[i].font);
            blockText.draw();
            y += text[i].font->getTextHeight(text[i].text) + 2;
        }
    }
}

void Screen::init()
{
    up_display_mcu_setup();
    display_init();
}


void Screen::showLogo()
{
    BitmapBlock blockLogo(0, 0, IMAGE_SCREENS_LOGO);
    blockLogo.draw();
}

void Screen::showMain(int mode, const char *presetName, int leashBattery, int airdogBattery,
                      int airdogMode, int followMode, int landMode)
{
    DOG_PRINT("[display]{main} current mode %d\n", mode);
    int width = 0;
    int cx = 0;
    TextBlock blockpresetName(presetName, 4, 4, &Font::LucideGrandeSmall);
    blockpresetName.draw();

    display_draw_line(0, 19, 127, 19);

    BitmapBlock blockBattery(103, 6, IMAGE_SCREENS_BATTERY);
    blockBattery.draw();

    BitmapBlock blockBatterySegment(105, 8, IMAGE_SCREENS_BATTERY_SEGMENT);
    if (leashBattery >=20)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=40)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=60)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }
    if (leashBattery >=80)
    {
        blockBatterySegment.draw();
        blockBatterySegment.x += 4;
    }

    int airdogImageId = -1;

    switch (airdogMode)
    {
        case AIRDOGMODE_NONE:
            break;

        case AIRDOGMODE_PLAY:
            airdogImageId = IMAGE_SCREENS_PLAY;
            break;

        case AIRDOGMODE_PAUSE:
            airdogImageId = IMAGE_SCREENS_PAUSE;
            break;

    }

    int followImageId = -1;
    int landImageId = -1;

    switch (followMode)
    {
        case FOLLOW_PATH:
            followImageId = IMAGE_SCREENS_PATH;
            break;

        case FOLLOW_ABS:
            followImageId = IMAGE_SCREENS_ABS;
            break;
    }


    switch (landMode)
    {
        case LAND_HOME:
            landImageId = IMAGE_SCREENS_HOME;
            break;

        case LAND_SPOT:
            landImageId = IMAGE_SCREENS_SPOT;
            break;
    }


    if (mode == MAINSCREEN_INFO || mode == MAINSCREEN_INFO_SUB)
    {
        // draw lines
        display_draw_line(0, 63, 127, 63);
        display_draw_line(0, 19, 0, 63);

        display_draw_line(64, 19, 64, 63);
        display_draw_line(64, 41, 113, 41);

        if (mode == MAINSCREEN_INFO_SUB)
        {
            //display_draw_line(31, 63, 31, 19);
            //display_draw_line(0, 41, 64, 41);
            display_fill_rectangle(0, 19, 64, 63);
        }

        // draw aidog land and follow modes
        BitmapBlock blockLndFol(113, 19, IMAGE_SCREENS_LND_FOL);
        blockLndFol.draw();

        width = imageInfo[followImageId].w;
        cx = (113 + 64 - width) / 2 + 1;

        BitmapBlock blockFollow(cx, 26, followImageId);
        blockFollow.draw();

        width = imageInfo[landImageId].w;
        cx = (113 + 64 - width) / 2 + 1;

        BitmapBlock blockLand(cx, 48, landImageId);
        blockLand.draw();

        // draw airdog battery level
        char buf[10];
        sprintf(buf, "%d", airdogBattery);
        TextBlock blockAirdogBattery(buf, 4, 30, &Font::LucideGrandeBig);
        blockAirdogBattery.draw();

        width = Font::LucideGrandeBig.getTextWidth(buf);

        if (airdogBattery < 100)
        {
            if (airdogImageId == -1)
            {
                TextBlock blockProcent("%", 5 + width, 35, &Font::LucideGrandeMed);
                blockProcent.draw();
            }
            else if (airdogImageId != -1)
            {
                BitmapBlock blockAirdogMode(7 + width, 33, airdogImageId);
                blockAirdogMode.draw();
            }
        }
    }
    else if (mode == MAINSCREEN_READY_TO_TAKEOFF)
    {
        const int rowCount = 2;
        struct TextInfo text[rowCount];

        memset(text, 0, sizeof(text));

        text[0].text = "Press    to";
        text[0].font = &Font::LucideGrandeSmall;
        text[1].text = "takeoff";
        text[1].font = &Font::LucideGrandeSmall;

        int y = 0;
        int yc = 19 + (64 - 20) / 2;
        drawText(DrawMode_CenterXY, text, rowCount, yc, 0, &y);

        // draw play pause button
        int x = Font::LucideGrandeSmall.getTextWidth(text[0].text);
        x = 64 - x / 2; // center
        x += Font::LucideGrandeSmall.getTextWidth("Press");
        x += 4;
        BitmapBlock blockPlayPause(x, y + 2, IMAGE_SCREENS_PLAY_PAUSE);
        blockPlayPause.draw();
    }
    else
    {
        const int rowCount = 2;
        struct TextInfo text[rowCount];

        memset(text, 0, sizeof(text));

        switch (mode)
        {
            case MAINSCREEN_LANDING:
                text[0].text = "Landing...";
                text[0].font = &Font::LucideGrandeSmall;
                break;

            case MAINSCREEN_TAKING_OFF:
                text[0].text = "Taking off...";
                text[0].font = &Font::LucideGrandeSmall;
                break;

            case MAINSCREEN_CONFIRM_TAKEOFF:
                text[0].text = "Press OK to";
                text[0].font = &Font::LucideGrandeSmall;
                text[1].text = "confirm takeoff";
                text[1].font = &Font::LucideGrandeSmall;
                break;

            case MAINSCREEN_GOING_HOME:
                text[0].text = "Going Home...";
                text[0].font = &Font::LucideGrandeSmall;
                break;
        }

        int yc = 19 + (64 - 20) / 2;
        drawText(DrawMode_CenterXY, text, rowCount, yc, 0);
    }


    /*
    // follow
    TextBlock blockFollow("Path", 66, 24, &Font::LucideGrande16);
    blockFollow.draw();
    */


    /*
    // land
    TextBlock blockLand("HOME", 66, 46, &Font::LucideGrandeSmall);
    blockLand.draw();
    */
}

void Screen::showMenu(int buttons, int type, int value, const char *presetName)
{
    int imageId = -1;
    int width = 0;
    const char *text = nullptr;
    const char *valueText = nullptr;
    const char *label = nullptr;
    char buf[10];

    switch (type)
    {
        case MENUTYPE_SETTINGS:
            imageId = IMAGE_SCREENS_ICONS_SETTINGS;
            text = "Settings";
            break;

        case MENUTYPE_ACTIVITIES:
            imageId = IMAGE_SCREENS_ICONS_ACTIVITIES;
            text = "Activities";
            break;

        case MENUTYPE_SNOWBOARD:
            imageId = IMAGE_SCREENS_ICONS_SNOWBOARD;
            //text = "Snowboard";
            // TODO: use snowboard
            text = presetName;
            break;

        case MENUTYPE_PAIRING:
            imageId = IMAGE_SCREENS_ICONS_PARING;
            text = "Pairing";
            break;

        case MENUTYPE_CALIBRATION:
            imageId = IMAGE_SCREENS_ICONS_CALIBRATION;
            text = "Calibration";
            break;

        case MENUTYPE_CALIBRATION_AIRDOG:
            imageId = IMAGE_SCREENS_ICONS_CALIBRATION;
            text = "Airdog calib";
            break;

        case MENUTYPE_COMPASS:
            imageId = IMAGE_SCREENS_ICONS_COMPASS;
            text = "Compass";
            break;

        case MENUTYPE_ACCELS:
            imageId = IMAGE_SCREENS_ICONS_ACCELS;
            text = "Accels";
            break;

        case MENUTYPE_GYRO:
            imageId = IMAGE_SCREENS_ICONS_GYRO;
            text = "Gyro";
            break;

        case MENUTYPE_SELECT:
            label = presetName;
            text = "Select";
            break;

        case MENUTYPE_CUSTOMIZE:
            label = presetName;
            text = "Customize";
            break;

        case MENUTYPE_ALTITUDE:
            text = "Altitude";
            sprintf(buf, "%d m", value);
            valueText = buf;
            break;

        case MENUTYPE_FOLLOW:
            text = "Follow";
            switch (value)
            {
                case FOLLOW_PATH:
                    valueText = "PATH";
                    break;

                case FOLLOW_ABS:
                    valueText = "ABS";
                    break;
            }
            break;

        case MENUTYPE_LANDING:
            text = "Landing";

            switch (value)
            {
                case LAND_HOME:
                    valueText = "HOME";
                    break;

                case LAND_SPOT:
                    valueText = "SPOT";
                    break;
            }
            break;

        case MENUTYPE_SAVE:
            label = presetName;
            text = "SAVE";
            break;

        case MENUTYPE_CANCEL:
            label = presetName;
            text = "CANCEL";
            break;
    }

    int yc = 32; // y center

    if (label != nullptr)
    {
        TextBlock blockpresetName(label, 4, 4, &Font::LucideGrandeSmall);
        blockpresetName.draw();

        display_draw_line(0, 19, 127, 19);

        yc = 19 + (64 - 20) / 2;
    }


    if (MENUBUTTON_LEFT & buttons)
    {
        ImageInfo info = imageInfo[IMAGE_SCREENS_LEFT];
        BitmapBlock blockFollow(0, yc - info.h / 2, IMAGE_SCREENS_LEFT);
        blockFollow.draw();
    }

    if (MENUBUTTON_RIGHT & buttons)
    {
        ImageInfo info = imageInfo[IMAGE_SCREENS_RIGHT];
        BitmapBlock blockFollow(128 - info.w, yc - info.h / 2, IMAGE_SCREENS_RIGHT);
        blockFollow.draw();
    }

    // show image and text
    if (imageId != -1 && text != nullptr)
    {
        ImageInfo info = imageInfo[imageId];
        BitmapBlock blockImage(64 - info.w / 2, 42 - info.h, imageId);
        blockImage.draw();

        width = Font::LucideGrandeSmall.getTextWidth(text);
        TextBlock blockText(text, 64 - width / 2, 44, &Font::LucideGrandeSmall);
        blockText.draw();
    }
    // show only text if we have a label
    else if (label != nullptr)
    {
        Font &font = strlen(text) > 8 ? Font::LucideGrandeSmall : Font::LucideGrandeMed;
        // center text if we have label
        int y = yc - font.getTextHeight(text) / 2;

        width = font.getTextWidth(text);
        TextBlock blockText(text, 64 - width / 2, y, &font);
        blockText.draw();
    }
    // show text && value text
    else if (text != nullptr && valueText != nullptr)
    {
        width = Font::LucideGrandeSmall.getTextWidth(text);
        TextBlock blockText(text, 64 - width / 2, 10, &Font::LucideGrandeSmall);
        blockText.draw();

        width = Font::LucideGrandeBig.getTextWidth(valueText);
        TextBlock blockValueText(valueText, 64 - width / 2, yc - 2, &Font::LucideGrandeBig);
        blockValueText.draw();
    }



    (void)value;
}

void Screen::showInfo(int info, int error)
{
    const int rowCount = 4;
    struct TextInfo text[rowCount];
    char buffer[60]; // buffer for error message

    memset(text, 0, sizeof(text));

    switch (info)
    {
        case INFO_CONNECTING_TO_AIRDOG:
            text[0].text = "Connecting";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Turn on your";
            text[1].font = &Font::LucideGrandeSmall;
            text[2].text = "airdog";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_CONNECTION_LOST:
            text[0].text = "Connection Lost";
            text[0].font = &Font::LucideGrandeSmall;
            text[1].text = "Move closer to";
            text[1].font = &Font::LucideGrandeTiny;
            text[2].text = "airdog";
            text[2].font = &Font::LucideGrandeTiny;
            break;

        case INFO_TAKEOFF_FAILED:
            text[0].text = "Takeoff failed";
            text[0].font = &Font::LucideGrandeSmall;

            switch (error)
            {
                default:
                    snprintf(buffer, sizeof(buffer), "Error %d", error);
                    buffer[sizeof(buffer)-1] = 0;
                    break;
            }

            text[1].text = buffer;
            text[1].font = &Font::LucideGrandeTiny;
            text[2].text = "refer to user";
            text[2].font = &Font::LucideGrandeTiny;
            text[3].text = "manual";
            text[3].font = &Font::LucideGrandeTiny;
            break;

        case INFO_CALIBRATING_SENSORS:
            text[0].text = "Calibrate";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Sensors";
            text[1].font = &Font::LucideGrandeSmall;
            text[2].text = "Press OK";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_CALIBRATING_AIRDOG:
            text[0].text = "Calibrate";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Airdog";
            text[1].font = &Font::LucideGrandeSmall;
            text[2].text = "Press OK";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_PAIRING:
            text[0].text = "Pairing";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Long press pairing";
            text[1].font = &Font::LucideGrandeTiny;
            text[2].text = "button on airdog";
            text[2].font = &Font::LucideGrandeTiny;
            break;

        case INFO_NOT_PAIRED:
            text[0].text = "Not paired";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Press OK to pair";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_ACQUIRING_GPS_LEASH:
            text[0].text = "Acquiring GPS";
            text[0].font = &Font::LucideGrandeSmall;
            break;

        case INFO_ACQUIRING_GPS_AIRDOG:
            text[0].text = "Airdog";
            text[0].font = &Font::LucideGrandeSmall;
            text[1].text = "Acquiring GPS";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_CALIBRATING_HOLD_STILL:
            text[0].text = "Calibrating";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Hold still";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_SUCCESS:
            text[0].text = "SUCCESS";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Press OK";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_FAILED:
            text[0].text = "FAILED";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Press OK";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_CALIBRATING_DANCE:
            text[0].text = "Calibrating";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "Do The Dance";
            text[1].font = &Font::LucideGrandeSmall;
            break;

        case INFO_NEXT_SIDE_UP:
            text[0].text = "Next Side";
            text[0].font = &Font::LucideGrandeMed;
            text[1].text = "UP";
            text[1].font = &Font::LucideGrandeMed;
            text[2].text = "And hold still";
            text[2].font = &Font::LucideGrandeSmall;
            break;

        case INFO_COMMUNICATION_FAILED:
            text[0].text = "Communication";
            text[0].font = &Font::LucideGrandeSmall;
            text[1].text = "Error";
            text[1].font = &Font::LucideGrandeSmall;
            text[2].text = "Please update";
            text[2].font = &Font::LucideGrandeTiny;
            text[3].text = "your software";
            text[3].font = &Font::LucideGrandeTiny;
            break;

        case INFO_ERROR:
            text[0].text = "Error";
            text[0].font = &Font::LucideGrandeMed;

            const char *errorText = getErrorMessageText(error);

            if (errorText == nullptr)
            {
                snprintf(buffer, sizeof(buffer), "%d", error);
                buffer[sizeof(buffer)-1] = 0;
                text[1].text = buffer;
                text[1].font = &Font::LucideGrandeTiny;
                text[2].text = "refer to user";
                text[2].font = &Font::LucideGrandeTiny;
                text[3].text = "manual";
                text[3].font = &Font::LucideGrandeTiny;
            }
            else
            {
                strcpy(buffer, errorText);
                char *p = buffer;

                for (int i = 1; i <= 3; i++)
                {
                    text[i].text = p;
                    text[i].font = &Font::LucideGrandeTiny;

                    // search for line end
                    while (*p != 0 && *p != '\n')
                    {
                        p++;
                    }

                    if (*p == 0)
                    {
                        // message end
                        break;
                    }
                    else
                    {
                        // we found \n
                        *p = 0;
                        p++;
                    }
                }
            }

            break;
    }

    drawText(DrawMode_CenterXY, text, rowCount, 32, 0);
}

void Screen::showList(LeashDisplay_Lines lines, int lineCount)
{
    struct TextInfo text[lineCount];

    memset(text, 0, sizeof(text));

    for (int i = 0; i < lineCount; i++)
    {
        text[i].text = &lines[i][0];
        if (text[i].text[0] == 0)
        {
            // make " " string if string is empty
            // otherwise drawText will skip it
            text[i].text = " ";
        }
        text[i].font = &Font::LucideGrandeTiny;
    }

    drawText(DrawMode_LeftTop, text, lineCount, 5, 1);
}
