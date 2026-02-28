#include "ui.h"
#include "quantize.h"

// ============================================================================
// UI HELPER FUNCTIONS
// ============================================================================

static void drawVelBar(int x, int vel) {
    NT_drawShapeI(kNT_rectangle, x, UI_VEL_BAR_TOP, x + UI_VEL_BAR_WIDTH, UI_VEL_BAR_BOTTOM, UI_BRIGHTNESS_DIM);
    if (vel > 0) {
        int h = (vel * UI_VEL_BAR_HEIGHT) / 127;
        if (h < 1) h = 1;
        NT_drawShapeI(kNT_rectangle, x, UI_VEL_BAR_BOTTOM - h, x + UI_VEL_BAR_WIDTH, UI_VEL_BAR_BOTTOM, UI_BRIGHTNESS_MAX);
    }
}

static void drawTrackBox(MidiLooperAlgorithm* alg, const int16_t* v,
                         int t, int x, int boxTop, int boxBottom, int textY,
                         int recTrack) {
    MidiLooper_DTC* dtc = alg->dtc;
    TrackParams tp = TrackParams::fromAlgorithm(v, t);
    int len = tp.length();
    bool trackEnabled = tp.enabled();

    int rawStep = 1;
    if (trackEnabled) {
        rawStep = clampParam(alg->trackStates[t].step, 1, len);
    }

    int boxFill = trackEnabled ? UI_BRIGHTNESS_DIM : 0;

    NT_drawShapeI(kNT_rectangle, x - 1, boxTop, x + UI_TRACK_BOX_WIDTH, boxBottom, boxFill);

    if (t == recTrack) {
        NT_drawShapeI(kNT_box, x - 1, boxTop, x + UI_TRACK_BOX_WIDTH, boxBottom, UI_BRIGHTNESS_MAX);
    }

    int textBrightness = trackEnabled ? UI_BRIGHTNESS_MAX : 2;

    char label[8];
    label[0] = 'T';
    label[1] = '1' + t;
    label[2] = '\0';
    NT_drawText(x, textY, label, textBrightness, kNT_textLeft, kNT_textNormal);

    if (trackEnabled && len > 1 && transportIsRunning(dtc->transportState)) {
        int lineX = x + (rawStep - 1) * (UI_TRACK_BOX_WIDTH - 2) / (len - 1);
        NT_drawShapeI(kNT_line, lineX, boxTop + 1, lineX, boxBottom - 1, UI_BRIGHTNESS_MAX);
    }
}

// ============================================================================
// MAIN DRAW FUNCTION
// ============================================================================

bool drawUI(MidiLooperAlgorithm* alg) {
    MidiLooper_DTC* dtc = alg->dtc;
    const int16_t* v = alg->v;

    int recTrack = clampParam(v[kParamRecTrack], 0, alg->numTracks - 1);

    // Transport indicator (play triangle or stop square)
    if (transportIsRunning(dtc->transportState)) {
        // Draw filled right-pointing play triangle
        for (int i = 0; i <= 8; i++) {
            int halfW = (i <= 4) ? i : (8 - i);  // 0,1,2,3,4,3,2,1,0
            int rightX = UI_LEFT_MARGIN + halfW * 2;
            NT_drawShapeI(kNT_line, UI_LEFT_MARGIN, UI_VEL_BAR_TOP + i, rightX, UI_VEL_BAR_TOP + i, UI_BRIGHTNESS_MAX);
        }
    } else {
        // Draw dim stop square
        NT_drawShapeI(kNT_rectangle, UI_LEFT_MARGIN, UI_VEL_BAR_TOP, UI_STOP_RIGHT, UI_STOP_BOTTOM, UI_BRIGHTNESS_DIM);
    }

    // Record indicator (circle)
    if (dtc->recordState == REC_LIVE || dtc->recordState == REC_STEP) {
        // Draw filled record circle
        for (int y = UI_REC_CENTER_Y - UI_REC_RADIUS; y <= UI_REC_CENTER_Y + UI_REC_RADIUS; y++) {
            int dy = y - UI_REC_CENTER_Y;
            int xOffset = 0;
            // Calculate x offset using integer approximation
            int r2 = UI_REC_RADIUS * UI_REC_RADIUS;
            int dy2 = dy * dy;
            for (int x = UI_REC_RADIUS; x >= 0; x--) {
                if (x * x + dy2 <= r2) {
                    xOffset = x;
                    break;
                }
            }
            NT_drawShapeI(kNT_line, UI_REC_CENTER_X - xOffset, y, UI_REC_CENTER_X + xOffset, y, UI_BRIGHTNESS_MAX);
        }
    } else {
        // Draw dim circle outline manually (kNT_circle renders incorrectly on hardware)
        int r2 = UI_REC_RADIUS * UI_REC_RADIUS;
        for (int y = UI_REC_CENTER_Y - UI_REC_RADIUS; y <= UI_REC_CENTER_Y + UI_REC_RADIUS; y++) {
            int dy = y - UI_REC_CENTER_Y;
            int dy2 = dy * dy;
            int xOffset = 0;
            for (int x = UI_REC_RADIUS; x >= 0; x--) {
                if (x * x + dy2 <= r2) {
                    xOffset = x;
                    break;
                }
            }
            NT_drawShapeI(kNT_point, UI_REC_CENTER_X - xOffset, y, 0, 0, UI_BRIGHTNESS_DIM);
            NT_drawShapeI(kNT_point, UI_REC_CENTER_X + xOffset, y, 0, 0, UI_BRIGHTNESS_DIM);
        }
    }

    // Step recording position number (to the right of play/record icons)
    if (dtc->recordState == REC_STEP && dtc->stepRecPos > 0) {
        char buf[8];
        NT_intToString(buf, dtc->stepRecPos);
        NT_drawText(UI_REC_CENTER_X + UI_REC_RADIUS + 5, UI_VEL_BAR_TOP + 8, buf, UI_BRIGHTNESS_MAX, kNT_textLeft, kNT_textNormal);
    }

    // 4-beat metronome indicator
    {
        int activeBeat = -1; // -1 means no beat highlighted
        if (transportIsRunning(dtc->transportState)) {
            int loopLen;
            int recQuantize = getCachedQuantize(v, recTrack, &alg->trackStates[recTrack].cache, loopLen);
            if (recQuantize > 0) {
                activeBeat = ((alg->trackStates[recTrack].clockCount - 1) / recQuantize) % 4;
            }
        }

        for (int i = 0; i < 4; i++) {
            int sqX = UI_LEFT_MARGIN + i * UI_STEP_SPACING;
            if (i == activeBeat) {
                NT_drawShapeI(kNT_rectangle, sqX, UI_STEP_Y_TOP, sqX + UI_STEP_WIDTH, UI_STEP_Y_BOTTOM, UI_BRIGHTNESS_MAX);
            } else {
                NT_drawShapeI(kNT_box, sqX, UI_STEP_Y_TOP, sqX + UI_STEP_WIDTH, UI_STEP_Y_BOTTOM, UI_BRIGHTNESS_DIM);
            }
        }
    }

    // Input velocity meter
    NT_drawText(UI_INPUT_LABEL_X, UI_LABEL_Y, "I:", 15, kNT_textLeft, kNT_textNormal);
    drawVelBar(UI_INPUT_BAR_X, dtc->inputVel);

    // Output velocity meters
    NT_drawText(UI_OUTPUT_LABEL_X, UI_LABEL_Y, "O:", 15, kNT_textLeft, kNT_textNormal);
    for (int t = 0; t < alg->numTracks; t++) {
        drawVelBar(UI_OUTPUT_BAR_X + t * UI_OUTPUT_BAR_SPACE, alg->trackStates[t].activeVel);
    }

    // Track info boxes - Row 1 (tracks 1-4)
    for (int t = 0; t < alg->numTracks && t < 4; t++) {
        int x = UI_LEFT_MARGIN + t * UI_TRACK_WIDTH;
        drawTrackBox(alg, v, t, x, UI_TRACK_ROW1_TOP, UI_TRACK_ROW1_BOTTOM, UI_TRACK_ROW1_TEXT_Y, recTrack);
    }
    // Row 2 (tracks 5-8)
    for (int t = 4; t < alg->numTracks; t++) {
        int x = UI_LEFT_MARGIN + (t - 4) * UI_TRACK_WIDTH;
        drawTrackBox(alg, v, t, x, UI_TRACK_ROW2_TOP, UI_TRACK_ROW2_BOTTOM, UI_TRACK_ROW2_TEXT_Y, recTrack);
    }

    return false;  // Show standard parameter line at top
}
