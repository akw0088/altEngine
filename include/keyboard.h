class Keyboard
{
public:
	Keyboard()
	{
		enter = shift = control = escape = up = left = down = right = false;
	}
	bool enter;
	bool escape;
	bool shift;
	bool control;
	bool up;
	bool left;
	bool down;
	bool right;
private:
};

