#pragma once

class GuiElement
{
public:
	virtual bool update(bool mouseAvailable) = 0;
	virtual void render() = 0;
};

class SceneSelector : public GuiElement
{
public:
	bool update(bool mouseAvailable);
	void render();
private:
	bool mouseCaptured = false;
	int highlightedTab = 0;
	static const int buttonHeight = 20;
	static const int buttonWidth = 20;
	static const int buttonGap = 10;
};

inline bool mouseInRectangle(int mouseX, int mouseY, int rectX, int rectY, int rectWidth, int rectHeight) {
	if (mouseX >= rectX && mouseX < rectX + rectWidth && mouseY >= rectY && mouseY < rectY + rectHeight)
		return true;
	else
		return false;
}