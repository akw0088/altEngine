#include "include.h"

Parse::Parse()
{
	memset(stack, 0, 80);
	sp = 0;
}

bool Parse::parse_entity(const char *input, Entity_list &entity_list)
{
	Entity *entity;
	char state = 'S';
	char prevstate = 'S';
	char key[80];
	char val[80];
	int i, j;

	for(i = 0; state != 'F' && state != '\0' && i < strlen(input); i++)
	{		
		prevstate = state;
		state = machine(state, input[i]);

		switch (state)
		{
		case 'K':
			if (prevstate == 'K')
			{
				key[j++] = input[i];
				key[j] = '\0';
				break;
			}
		case 'V':
			if (prevstate == 'V')
			{
				val[j++] = input[i];
				val[j] = '\0';
				break;
			}
		case 'A':
			if (prevstate == 'S')
			{
				entity = new Entity(10.0f, vec3(0.0f, 0.0f, 0.0f));
				entity_list.addEntity(entity);
			}
			j = 0;
			break;
		case 'B':
			j = 0;
			break;
		case 'R':
			i--;
//			printf("%s = %s\n", key, val);
			if (strcmp(key, "origin") == 0)
			{
				int x, y, z;

				sscanf(val, "%d %d %d", &x, &y, &z);
				// change axis
				//y = z;
				//z = -y;
				entity->position.x = (float)x;
				entity->position.y = (float)z;
				entity->position.z = (float)-y;
			}
			else if (strcmp(key, "classname") == 0)
			{
				if (strcmp(val, "light") == 0)
				{
					entity->angular_momentum.x = 100.0;
				}
			}
			break;
		}
	}

	if (state == 'F' && sp == 0)
		return true;
	else
		return false;
}

char Parse::machine(char state, char input)
{
	switch (state)
	{
	case 'S':
		switch (input)
		{
		case '{':
			push('}');
			return 'A';
		case '\n':
			return 'S';
		case '\0':
			return 'F';
		}
		return '\0';

	case 'A':
		switch (input)
		{
		case '"':
			push('"');
			return 'K';
		case '}':
			pop('}');
			return 'S';
		case ' ':
			return 'A';
		case '\n':
			return 'A';
		}
		return '\0';

	case 'B':
		switch (input)
		{
		case '"':
			push('"');
			return 'V';
		case ' ':
			return 'B';
		case '\n':
			return 'B';
		}
		return '\0';

	case 'K':
		switch (input)
		{
		case '"':
			pop('"');
			return 'B';
		}
		return 'K';

	case 'V':
		switch (input)
		{
		case '"':
			pop('"');
			return 'R';

		}
		return 'V';

	case 'R':
		return 'A';

	case 'F':
		return 'F';
	}
	return '\0';
}

void Parse::push(char input)
{
	stack[sp] = input;
	sp++;
}

void Parse::pop(char input)
{
	if (stack[sp - 1] == input)
		sp--;
}

