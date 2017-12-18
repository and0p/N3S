#pragma once

using namespace std;

enum N3SFunction {
	nes_p1_a, nes_p1_b, nes_p1_up, nes_p1_left, nes_p1_down, nes_p1_right, nes_p1_start, nes_p1_select,
	nes_p2_a, nes_p2_b, nes_p2_up, nes_p2_left, nes_p2_down, nes_p2_right, nes_p2_start, nes_p2_select,
	emu_pause, emu_reset, emu_savestate, emu_loadstate, emu_nextstate, emu_prevstate,
	tog_game, tog_editor, editor_alt,
	cam_left, cam_right, cam_up, cam_down, cam_pan_in, cam_pan_out,
	cam_move_left, cam_move_right, cam_move_up, cam_move_down, cam_reset,
	selection_add, selection_remove, selection_copy, selection_delete, selection_deselect,
	editor_moveleft, editor_moveright, editor_moveup, editor_movedown,
	voxeleditor_movein, voxeleditor_moveout, voxeleditor_setvoxel, voxeleditor_deletevoxel,
	voxeleditor_color0, voxeleditor_color1, voxeleditor_color2, voxeleditor_color3,
	voxeleditor_exit, voxeleditor_mirror, editor_copy, editor_paste, palette_copy, palette_paste,
	FUNCTION_COUNT
};

enum DeviceType { KEYBOARD, GAMEPAD };

enum JoyButton { DPAD_UP, DPAD_DOWN, DPAD_LEFT, DPAD_RIGHT, BTN_A, BTN_B, BTN_X, BTN_Y, BTN_L_BUMPER, BTN_R_BUMPER, BTN_SELECT, BTN_START, BTN_L_CLICK, BTN_R_CLICK, BUTTONCOUNT };
enum JoyAxis { AX_L_STICK_RIGHT, AX_L_STICK_LEFT, AX_L_STICK_UP, AX_L_STICK_DOWN, AX_R_STICK_RIGHT, AX_R_STICK_LEFT, AX_R_STICK_UP, AX_R_STICK_DOWN, AX_L_TRIGGER, AX_R_TRIGGER, AXISCOUNT };

const int inputCount = 238;

struct InputMapping {
	DeviceType type;
	int deviceNumber;
	bool analog;
	int enumNumber;
	string name;
	bool bindable;
};

struct FunctionMapping {
	N3SFunction function;
	string name;
	bool configurable;
	string defaultBinding1;
	string defaultBinding2;
};

const InputMapping inputMap[inputCount] = {
	{ KEYBOARD, 0, false, VK_ADD, "NUMPAD PLUS", true },
	{ KEYBOARD, 0, false, VK_ATTN, "ATTN", true },
	{ KEYBOARD, 0, false, VK_BACK, "BACKSPACE", true },
	{ KEYBOARD, 0, false, VK_CANCEL, "BREAK", true },
	{ KEYBOARD, 0, false, VK_CLEAR, "CLEAR", true },
	{ KEYBOARD, 0, false, VK_CRSEL, "CR SELECT", true },
	{ KEYBOARD, 0, false, VK_DECIMAL, "NUMPAD PERIOD", true },
	{ KEYBOARD, 0, false, VK_DIVIDE, "NUMPAD SLASH", true },
	{ KEYBOARD, 0, false, VK_EREOF, "ER EOF", true },
	{ KEYBOARD, 0, false, VK_ESCAPE, "ESCAPE", false },
	{ KEYBOARD, 0, false, VK_EXECUTE, "EXECUTE", true },
	{ KEYBOARD, 0, false, VK_EXSEL, "EX SEL", true },
	{ KEYBOARD, 0, false, VK_ICO_CLEAR, "ICON CLEAR", true },
	{ KEYBOARD, 0, false, VK_ICO_HELP, "ICON HELP", true },
	{ KEYBOARD, 0, false, 48, "0", true },
	{ KEYBOARD, 0, false, 49, "1", true },
	{ KEYBOARD, 0, false, 50, "2", true },
	{ KEYBOARD, 0, false, 51, "3", true },
	{ KEYBOARD, 0, false, 52, "4", true },
	{ KEYBOARD, 0, false, 53, "5", true },
	{ KEYBOARD, 0, false, 54, "6", true },
	{ KEYBOARD, 0, false, 55, "7", true },
	{ KEYBOARD, 0, false, 56, "8", true },
	{ KEYBOARD, 0, false, 57, "9", true },
	{ KEYBOARD, 0, false, 65, "A", true },
	{ KEYBOARD, 0, false, 66, "B", true },
	{ KEYBOARD, 0, false, 67, "C", true },
	{ KEYBOARD, 0, false, 68, "D", true },
	{ KEYBOARD, 0, false, 69, "E", true },
	{ KEYBOARD, 0, false, 70, "F", true },
	{ KEYBOARD, 0, false, 71, "G", true },
	{ KEYBOARD, 0, false, 72, "H", true },
	{ KEYBOARD, 0, false, 73, "I", true },
	{ KEYBOARD, 0, false, 74, "J", true },
	{ KEYBOARD, 0, false, 75, "K", true },
	{ KEYBOARD, 0, false, 76, "L", true },
	{ KEYBOARD, 0, false, 77, "M", true },
	{ KEYBOARD, 0, false, 78, "N", true },
	{ KEYBOARD, 0, false, 79, "O", true },
	{ KEYBOARD, 0, false, 80, "P", true },
	{ KEYBOARD, 0, false, 81, "Q", true },
	{ KEYBOARD, 0, false, 82, "R", true },
	{ KEYBOARD, 0, false, 83, "S", true },
	{ KEYBOARD, 0, false, 84, "T", true },
	{ KEYBOARD, 0, false, 85, "U", true },
	{ KEYBOARD, 0, false, 86, "V", true },
	{ KEYBOARD, 0, false, 87, "W", true },
	{ KEYBOARD, 0, false, 88, "X", true },
	{ KEYBOARD, 0, false, 89, "Y", true },
	{ KEYBOARD, 0, false, 90, "Z", true },
	{ KEYBOARD, 0, false, VK_MULTIPLY, "NUMPAD STAR", true },
	{ KEYBOARD, 0, false, VK_NONAME, "NO NAME", true },
	{ KEYBOARD, 0, false, VK_NUMPAD0, "NUMPAD 0", true },
	{ KEYBOARD, 0, false, VK_NUMPAD1, "NUMPAD 1", true },
	{ KEYBOARD, 0, false, VK_NUMPAD2, "NUMPAD 2", true },
	{ KEYBOARD, 0, false, VK_NUMPAD3, "NUMPAD 3", true },
	{ KEYBOARD, 0, false, VK_NUMPAD4, "NUMPAD 4", true },
	{ KEYBOARD, 0, false, VK_NUMPAD5, "NUMPAD 5", true },
	{ KEYBOARD, 0, false, VK_NUMPAD6, "NUMPAD 6", true },
	{ KEYBOARD, 0, false, VK_NUMPAD7, "NUMPAD 7", true },
	{ KEYBOARD, 0, false, VK_NUMPAD8, "NUMPAD 8", true },
	{ KEYBOARD, 0, false, VK_NUMPAD9, "NUMPAD 9", true },
	{ KEYBOARD, 0, false, VK_OEM_1, "COLON", true },
	{ KEYBOARD, 0, false, VK_OEM_102, "LESS GREATER", true },
	{ KEYBOARD, 0, false, VK_OEM_2, "FORWARD SLASH", true },
	{ KEYBOARD, 0, false, VK_OEM_3, "TILDE", true },
	{ KEYBOARD, 0, false, VK_OEM_4, "LEFT BRACKET", true },
	{ KEYBOARD, 0, false, VK_OEM_5, "LINE", true },
	{ KEYBOARD, 0, false, VK_OEM_6, "RIGHT BRACKET", true },
	{ KEYBOARD, 0, false, VK_OEM_7, "SINGLE QUOTE", true },
	{ KEYBOARD, 0, false, VK_OEM_8, "EXCLAMATION POINT", true },
	{ KEYBOARD, 0, false, VK_OEM_ATTN, "OAM ATTN", true },
	{ KEYBOARD, 0, false, VK_OEM_AUTO, "AUTO ", true },
	{ KEYBOARD, 0, false, VK_OEM_AX, "AX", true },
	{ KEYBOARD, 0, false, VK_OEM_BACKTAB, "BACK TAB", true },
	{ KEYBOARD, 0, false, VK_OEM_CLEAR, "OEM CLEAR", true },
	{ KEYBOARD, 0, false, VK_OEM_COMMA, "COMMA", true },
	{ KEYBOARD, 0, false, VK_OEM_COPY, "COPY", true },
	{ KEYBOARD, 0, false, VK_OEM_CUSEL, "CU SELECT", true },
	{ KEYBOARD, 0, false, VK_OEM_ENLW, "ENLW", true },
	{ KEYBOARD, 0, false, VK_OEM_FINISH, "FINISH", true },
	{ KEYBOARD, 0, false, VK_OEM_FJ_LOYA, "LOYA", true },
	{ KEYBOARD, 0, false, VK_OEM_FJ_MASSHOU, "MASHU", true },
	{ KEYBOARD, 0, false, VK_OEM_FJ_ROYA, "ROYA", true },
	{ KEYBOARD, 0, false, VK_OEM_FJ_TOUROKU, "TOUROKU", true },
	{ KEYBOARD, 0, false, VK_OEM_JUMP, "JUMP", true },
	{ KEYBOARD, 0, false, VK_OEM_MINUS, "MINUS", true },
	{ KEYBOARD, 0, false, VK_OEM_PA1, "OEM PA 1", true },
	{ KEYBOARD, 0, false, VK_OEM_PA2, "OEM PA 2", true },
	{ KEYBOARD, 0, false, VK_OEM_PA3, "OEM PA 3", true },
	{ KEYBOARD, 0, false, VK_OEM_PERIOD, "PERIOD", true },
	{ KEYBOARD, 0, false, VK_OEM_PLUS, "EQUALS", true },
	{ KEYBOARD, 0, false, VK_OEM_RESET, "RESET", true },
	{ KEYBOARD, 0, false, VK_OEM_WSCTRL, "WS CTRL", true },
	{ KEYBOARD, 0, false, VK_PA1, "PA ONE", true },
	{ KEYBOARD, 0, false, VK_PACKET, "PACKET", true },
	{ KEYBOARD, 0, false, VK_PLAY, "PLAY", true },
	{ KEYBOARD, 0, false, VK_PROCESSKEY, "PROCESS", true },
	{ KEYBOARD, 0, false, VK_RETURN, "ENTER", true },
	{ KEYBOARD, 0, false, VK_SELECT, "SELECT", true },
	{ KEYBOARD, 0, false, VK_SEPARATOR, "SEPARATOR", true },
	{ KEYBOARD, 0, false, VK_SPACE, "SPACE", true },
	{ KEYBOARD, 0, false, VK_SUBTRACT, "NUMPAD MINUS", true },
	{ KEYBOARD, 0, false, VK_TAB, "TAB", true },
	{ KEYBOARD, 0, false, VK_ZOOM, "ZOOM", true },
	{ KEYBOARD, 0, false, VK_ACCEPT, "ACCEPT", true },
	{ KEYBOARD, 0, false, VK_APPS, "CONTEXT MENU", true },
	{ KEYBOARD, 0, false, VK_BROWSER_BACK, "BROWSER BACK", true },
	{ KEYBOARD, 0, false, VK_BROWSER_FAVORITES, "BROWSER FAVORITES", true },
	{ KEYBOARD, 0, false, VK_BROWSER_FORWARD, "BROWSER FORWARD", true },
	{ KEYBOARD, 0, false, VK_BROWSER_HOME, "BROWSER HOME", true },
	{ KEYBOARD, 0, false, VK_BROWSER_REFRESH, "BROWSER REFRESH", true },
	{ KEYBOARD, 0, false, VK_BROWSER_SEARCH, "BROWSER SEARCH", true },
	{ KEYBOARD, 0, false, VK_BROWSER_STOP, "BROWSER STOP", true },
	{ KEYBOARD, 0, false, VK_CAPITAL, "CAPS LOCK", true },
	{ KEYBOARD, 0, false, VK_CONVERT, "CONVERT", true },
	{ KEYBOARD, 0, false, VK_DELETE, "DELETE", true },
	{ KEYBOARD, 0, false, VK_DOWN, "DOWN ARROW", true },
	{ KEYBOARD, 0, false, VK_END, "END", true },
	{ KEYBOARD, 0, false, VK_F1, "F1", true },
	{ KEYBOARD, 0, false, VK_F10, "F10", true },
	{ KEYBOARD, 0, false, VK_F11, "F11", true },
	{ KEYBOARD, 0, false, VK_F12, "F12", true },
	{ KEYBOARD, 0, false, VK_F13, "F13", true },
	{ KEYBOARD, 0, false, VK_F14, "F14", true },
	{ KEYBOARD, 0, false, VK_F15, "F15", true },
	{ KEYBOARD, 0, false, VK_F16, "F16", true },
	{ KEYBOARD, 0, false, VK_F17, "F17", true },
	{ KEYBOARD, 0, false, VK_F18, "F18", true },
	{ KEYBOARD, 0, false, VK_F19, "F19", true },
	{ KEYBOARD, 0, false, VK_F2, "F2", true },
	{ KEYBOARD, 0, false, VK_F20, "F20", true },
	{ KEYBOARD, 0, false, VK_F21, "F21", true },
	{ KEYBOARD, 0, false, VK_F22, "F22", true },
	{ KEYBOARD, 0, false, VK_F23, "F23", true },
	{ KEYBOARD, 0, false, VK_F24, "F24", true },
	{ KEYBOARD, 0, false, VK_F3, "F3", true },
	{ KEYBOARD, 0, false, VK_F4, "F4", true },
	{ KEYBOARD, 0, false, VK_F5, "F5", true },
	{ KEYBOARD, 0, false, VK_F6, "F6", true },
	{ KEYBOARD, 0, false, VK_F7, "F7", true },
	{ KEYBOARD, 0, false, VK_F8, "F8", true },
	{ KEYBOARD, 0, false, VK_F9, "F9", true },
	{ KEYBOARD, 0, false, VK_FINAL, "FINAL", true },
	{ KEYBOARD, 0, false, VK_HELP, "HELP", true },
	{ KEYBOARD, 0, false, VK_HOME, "HOME", true },
	{ KEYBOARD, 0, false, VK_ICO_00, "ICON ZEROZERO", true },
	{ KEYBOARD, 0, false, VK_INSERT, "INSERT", true },
	{ KEYBOARD, 0, false, VK_JUNJA, "JUNJA", true },
	{ KEYBOARD, 0, false, VK_KANA, "KANA", true },
	{ KEYBOARD, 0, false, VK_KANJI, "KANJI", true },
	{ KEYBOARD, 0, false, VK_LAUNCH_APP1, "APP ONE", true },
	{ KEYBOARD, 0, false, VK_LAUNCH_APP2, "APP TWO", true },
	{ KEYBOARD, 0, false, VK_LAUNCH_MAIL, "MAIL", true },
	{ KEYBOARD, 0, false, VK_LAUNCH_MEDIA_SELECT, "MEDIA", true },
	{ KEYBOARD, 0, false, VK_LBUTTON, "LEFT BUTTON", true },
	{ KEYBOARD, 0, false, VK_LCONTROL, "LEFT CTRL", true },
	{ KEYBOARD, 0, false, VK_LEFT, "LEFT ARROW", true },
	{ KEYBOARD, 0, false, VK_LMENU, "LEFT ALT", true },
	{ KEYBOARD, 0, false, VK_LSHIFT, "LEFT SHIFT", true },
	{ KEYBOARD, 0, false, VK_LWIN, "LEFT WIN", true },
	{ KEYBOARD, 0, false, VK_MBUTTON, "MIDDLE MOUSE", true },
	{ KEYBOARD, 0, false, VK_MEDIA_NEXT_TRACK, "NEXT TRACK", true },
	{ KEYBOARD, 0, false, VK_MEDIA_PLAY_PAUSE, "PLAY / PAUSE", true },
	{ KEYBOARD, 0, false, VK_MEDIA_PREV_TRACK, "PREVIOUS TRACK", true },
	{ KEYBOARD, 0, false, VK_MEDIA_STOP, "STOP", true },
	{ KEYBOARD, 0, false, VK_MODECHANGE, "MODE CHANGE", true },
	{ KEYBOARD, 0, false, VK_NEXT, "PAGE DOWN", true },
	{ KEYBOARD, 0, false, VK_NONCONVERT, "NON CONVERT", true },
	{ KEYBOARD, 0, false, VK_NUMLOCK, "NUM LOCK", true },
	{ KEYBOARD, 0, false, VK_OEM_FJ_JISHO, "JISHO", true },
	{ KEYBOARD, 0, false, VK_PAUSE, "PAUSE", true },
	{ KEYBOARD, 0, false, VK_PRINT, "PRINT", true },
	{ KEYBOARD, 0, false, VK_PRIOR, "PAGE UP", true },
	{ KEYBOARD, 0, false, VK_RBUTTON, "RIGHT MOUSE", true },
	{ KEYBOARD, 0, false, VK_RCONTROL, "RIGHT CTRL", true },
	{ KEYBOARD, 0, false, VK_RIGHT, "RIGHT ARROW", true },
	{ KEYBOARD, 0, false, VK_RMENU, "RIGHT ALT", true },
	{ KEYBOARD, 0, false, VK_RSHIFT, "RIGHT SHIFT", true },
	{ KEYBOARD, 0, false, VK_RWIN, "RIGHT WIN", true },
	{ KEYBOARD, 0, false, VK_SCROLL, "SCROL LOCK", true },
	{ KEYBOARD, 0, false, VK_SLEEP, "SLEEP", true },
	{ KEYBOARD, 0, false, VK_SNAPSHOT, "PRINT SCREEN", true },
	{ KEYBOARD, 0, false, VK_UP, "UP ARROW", true },
	{ KEYBOARD, 0, false, VK_VOLUME_DOWN, "VOLUME DOWN", true },
	{ KEYBOARD, 0, false, VK_VOLUME_MUTE, "VOLUME MUTE", true },
	{ KEYBOARD, 0, false, VK_VOLUME_UP, "VOLUME UP", true },
	{ KEYBOARD, 0, false, VK_XBUTTON1, "MOUSE BACK", true },
	{ KEYBOARD, 0, false, VK_XBUTTON2, "MOUSE FORWARD", true },
	{ GAMEPAD, 0, true, AX_L_STICK_UP, "JOY1_L_UP", true },
	{ GAMEPAD, 0, true, AX_L_STICK_DOWN, "JOY1_L_DOWN", true },
	{ GAMEPAD, 0, true, AX_L_STICK_LEFT, "JOY1_L_LEFT", true },
	{ GAMEPAD, 0, true, AX_L_STICK_RIGHT, "JOY1_L_RIGHT", true },
	{ GAMEPAD, 0, true, AX_R_STICK_UP, "JOY1_R_UP", true },
	{ GAMEPAD, 0, true, AX_R_STICK_DOWN, "JOY1_R_DOWN", true },
	{ GAMEPAD, 0, true, AX_R_STICK_LEFT, "JOY1_R_LEFT", true },
	{ GAMEPAD, 0, true, AX_R_STICK_RIGHT, "JOY1_R_RIGHT", true },
	{ GAMEPAD, 0, false, BTN_L_CLICK, "JOY1_L_CLICK", true },
	{ GAMEPAD, 0, false, BTN_R_CLICK, "JOY1_R_CLICK", true },
	{ GAMEPAD, 0, true, AX_L_TRIGGER, "JOY1_L_TRIGGER", true },
	{ GAMEPAD, 0, true, AX_R_TRIGGER, "JOY1_R_TRIGGER", true },
	{ GAMEPAD, 0, false, BTN_L_BUMPER, "JOY1_L_BUMPER", true },
	{ GAMEPAD, 0, false, BTN_R_BUMPER, "JOY1_R_BUMPER", true },
	{ GAMEPAD, 0, false, DPAD_UP, "JOY1_DPAD_UP", true },
	{ GAMEPAD, 0, false, DPAD_DOWN, "JOY1_DPAD_DOWN", true },
	{ GAMEPAD, 0, false, DPAD_LEFT, "JOY1_DPAD_LEFT", true },
	{ GAMEPAD, 0, false, DPAD_RIGHT, "JOY1_DPAD_RIGHT", true },
	{ GAMEPAD, 0, false, BTN_A, "JOY1_BTN_A", true },
	{ GAMEPAD, 0, false, BTN_B, "JOY1_BTN_B", true },
	{ GAMEPAD, 0, false, BTN_X, "JOY1_BTN_X", true },
	{ GAMEPAD, 0, false, BTN_Y, "JOY1_BTN_Y", true },
	{ GAMEPAD, 0, false, BTN_START, "JOY1_BTN_START", true },
	{ GAMEPAD, 0, false, BTN_SELECT, "JOY1_BTN_SELECT", true },
	{ GAMEPAD, 1, true, AX_L_STICK_UP, "JOY2_L_UP", true },
	{ GAMEPAD, 1, true, AX_L_STICK_DOWN, "JOY2_L_DOWN", true },
	{ GAMEPAD, 1, true, AX_L_STICK_LEFT, "JOY2_L_LEFT", true },
	{ GAMEPAD, 1, true, AX_L_STICK_RIGHT, "JOY2_L_RIGHT", true },
	{ GAMEPAD, 1, true, AX_R_STICK_UP, "JOY2_R_UP", true },
	{ GAMEPAD, 1, true, AX_R_STICK_DOWN, "JOY2_R_DOWN", true },
	{ GAMEPAD, 1, true, AX_R_STICK_LEFT, "JOY2_R_LEFT", true },
	{ GAMEPAD, 1, true, AX_R_STICK_RIGHT, "JOY2_R_RIGHT", true },
	{ GAMEPAD, 1, false,BTN_L_CLICK, "JOY2_L_CLICK", true },
	{ GAMEPAD, 1, false, BTN_R_CLICK, "JOY2_R_CLICK", true },
	{ GAMEPAD, 1, true, AX_L_TRIGGER, "JOY2_L_TRIGGER", true },
	{ GAMEPAD, 1, true, AX_R_TRIGGER, "JOY2_R_TRIGGER", true },
	{ GAMEPAD, 1, true, BTN_L_BUMPER, "JOY2_L_BUMPER", true },
	{ GAMEPAD, 1, true, BTN_R_BUMPER, "JOY2_R_BUMPER", true },
	{ GAMEPAD, 1, false, DPAD_UP, "JOY2_DPAD_UP", true },
	{ GAMEPAD, 1, false, DPAD_DOWN, "JOY2_DPAD_DOWN", true },
	{ GAMEPAD, 1, false, DPAD_LEFT, "JOY2_DPAD_LEFT", true },
	{ GAMEPAD, 1, false, DPAD_RIGHT, "JOY2_DPAD_RIGHT", true },
	{ GAMEPAD, 1, false, BTN_A, "JOY2_BTN_A", true },
	{ GAMEPAD, 1, false, BTN_B, "JOY2_BTN_B", true },
	{ GAMEPAD, 1, false, BTN_X, "JOY2_BTN_X", true },
	{ GAMEPAD, 1, false, BTN_Y, "JOY2_BTN_Y", true },
	{ GAMEPAD, 1, false, BTN_START, "JOY2_BTN_START", true },
	{ GAMEPAD, 1, false, BTN_SELECT, "JOY2_BTN_SELECT", true },
};

const FunctionMapping functionMap[FUNCTION_COUNT] = {
	{ nes_p1_a,	"NES P1 A", true, "K", "JOY1_BTN_B" },
	{ nes_p1_b, "NES P1 B", true, "J", "JOY1_BTN_A" },
	{ nes_p1_up, "NES P1 UP", true, "W", "JOY1_DPAD_UP" },
	{ nes_p1_left, "NES P1 LEFT", true, "A", "JOY1_DPAD_LEFT" },
	{ nes_p1_down, "NES P1 DOWN", true, "S", "JOY1_DPAD_DOWN" },
	{ nes_p1_right, "NES P1 RIGHT", true, "D", "JOY1_DPAD_RIGHT" },
	{ nes_p1_start, "NES P1 START", true, "I", "JOY1_BTN_START" },
	{ nes_p1_select, "NES P1 SELECT", true, "U", "JOY1_BTN_SELECT" },
	{ nes_p2_a, "NES P2 A", true, "", "JOY2_BTN_B" },
	{ nes_p2_b, "NES P2 B", true, "", "JOY2_BTN_A" },
	{ nes_p2_up, "NES P2 UP", true, "", "JOY2_DPAD_UP" },
	{ nes_p2_left, "NES P2 LEFT", true, "", "JOY2_DPAD_LEFT" },
	{ nes_p2_down, "NES P2 DOWN", true, "", "JOY2_DPAD_DOWN" },
	{ nes_p2_right, "NES P2 RIGHT", true, "", "JOY2_DPAD_RIGHT" },
	{ nes_p2_start, "NES P2 START", true, "", "JOY2_BTN_START" },
	{ nes_p2_select, "NES P2 SELECT", true, "", "JOY2_BTN_SELECT" },
	{ emu_pause, "EMU PAUSE TOGGLE", true, "F3", "JOY1_R_BUMPER" },
	{ emu_reset, "EMU RESET", true, "F4", "" },
	{ emu_savestate, "EMU SAVE", true, "F5", "" },
	{ emu_loadstate, "EMU LOAD", true, "F9", "" },
	{ emu_nextstate, "NEXT SAVESTATE", true, "F7", "" },
	{ emu_prevstate, "PREVIOUS SAVESTATE", true, "F6", "" },
	{ tog_game, "GAME MODE", true, "F1", "" },
	{ tog_editor, "EDITOR MODE", true, "F2", "" },
	{ editor_alt, "EDITOR ALT", false, "", "" },
	{ cam_left, "LOOK LEFT", true, "LEFT ARROW", "" },
	{ cam_right, "LOOK RIGHT", true, "RIGHT ARROW", "" },
	{ cam_up, "LOOK UP", true, "UP ARROW", "" },
	{ cam_down, "LOOK DOWN", true, "DOWN ARROW", "" },
	{ cam_pan_in, "PAN IN", true, "EQUALS", "" },
	{ cam_pan_out, "PAN OUT", true, "MINUS", "" },
	{ cam_move_left, "MOVE CAMERA LEFT", true, "B", "" },
	{ cam_move_right, "MOVE CAMERA RIGHT", true, "N", "" },
	{ cam_move_up, "MOVE CAMERA UP", true, "M", "" },
	{ cam_move_down, "MOVE CAMERA DOWN", true, "V", "" },
	{ cam_reset, "RESET CAMERA", true, "C", "JOY1_L_BUMPER" },
	{ selection_add, "SELECTION ADD", false, "LEFT SHIFT", "RIGHT SHIFT" },
	{ selection_remove, "SELECTION REMOVE", false, "LEFT ALT", "RIGHT ALT" },
	{ selection_copy, "SELECTION COPY", false, "LEFT CTRL", "RIGHT CTRL" },
	{ selection_delete, "SELECTION DELETE", false, "DELETE", "" },
	{ selection_deselect, "SELECTION DESELECT", false, "ESCAPE", "" },
	{ editor_moveleft, "EDITOR MOVE LEFT", false, "LEFT ARROW", "" },
	{ editor_moveright, "EDITOR MOVE RIGHT", false, "RIGHT ARROW", "" },
	{ editor_moveup, "EDITOR MOVE UP", false, "UP ARROW", "" },
	{ editor_movedown, "EDITOR MOVE DOWN", false, "DOWN ARROW", "" },
	{ voxeleditor_movein, "VEDITOR MOVE IN", false, "", "" },
	{ voxeleditor_moveout, "VEDITOR MOVE OUT", false, "", "" },
	{ voxeleditor_setvoxel, "VEDITOR SET VOXEL", false, "", "" },
	{ voxeleditor_deletevoxel, "VEDITOR DELETE VOXEL", false, "", "" },
	{ voxeleditor_color0, "VEDITOR COLOR ZERO", false, "", "" },
	{ voxeleditor_color1, "VEDITOR COLOR ONE", false, "", "" },
	{ voxeleditor_color2, "VEDITOR COLOR TWO", false, "", "" },
	{ voxeleditor_color3, "VEDITOR COLOR THREE", false, "", "" },
	{ voxeleditor_exit, "VEDITOR EXIT", false, "ESCAPE", "" },
	{ voxeleditor_mirror, "VEDITOR SET MIRROR", false, "", "" },
	{ editor_copy, "EDITOR COPY", false, "C", "" },
	{ editor_paste, "EDITOR PASTE", false, "V", "" },
	{ palette_copy, "PALETTE COPY", false, "O", "" },
	{ palette_paste, "PALETTE PASTE", false, "P", "" },
};