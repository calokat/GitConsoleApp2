#include "stdafx.h"
#include "TacObject.h"
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif // !_USE_MATH_DEFINES
#include <math.h>
Font TacObject::font;
TacObject::TacObject()
{
	sprite = CircleShape(50);
	sprite.setFillColor(Color(255, 0, 0));
	hasParent = false;
}

TacObject::TacObject(string nm, float xParam, float yParam)
{
	x = xParam;
	y = yParam;
	sprite = CircleShape(50);
	sprite.setPosition(x, y);
	sprite.setFillColor(Color(255, 0, 0));
	name = nm;
	if (font.getInfo().family == "")
	{
		TacObject::font.loadFromFile("C:\\Users\\Caleb\\Documents\\Github\\GitConsoleApp2\\Debug\\Montserrat-Black.ttf");
	}
	text.setFont(TacObject::font);
	text.setString(name);
	text.setCharacterSize(24);
	text.setFillColor(sf::Color::Black);
	// places text
	text.setPosition(x, y);
	text.move(-(text.getLocalBounds().width) / 3, sprite.getLocalBounds().height / 2 + text.getLocalBounds().height);
	hasParent = false;
	lineOffset = Vector2f(0, 75);
	//SetUpLines();
}

TacObject::TacObject(vector<TacObject*> pnt, string nm, float xParam, float yParam) : TacObject(nm, xParam, yParam)
{
	parents = pnt;
	SetUpLines();
}

TacObject::TacObject(TacObject * pnt, string nm, float xParam, float yParam) : TacObject(nm, xParam, yParam)
{
	AddParent(pnt);
	SetUpLines();
}

TacObject::TacObject(const TacObject& cpy)
{
	this->name = cpy.name;
	this->sprite = cpy.sprite;
	this->text = cpy.text;
	this->x = cpy.x;
	this->y = cpy.y;
	if (hasParent)
	{
		this->parents = cpy.parents;
		this->hasParent = true;
		SetUpLines();
	}
	else
	{
		this->hasParent = false;
	}
}
// draw all of the lines. Because SFML does not support layering, this must be done before the regular draw()
void TacObject::DrawLines(RenderTarget &target)
{
	for (RectangleShape line : lines)
	{
		target.draw(line);
	}
}

// draws everything except the lines
void TacObject::draw(RenderTarget &target, RenderStates states) const
{
	target.draw(sprite);
	target.draw(text);

}

// gets the center of the circle sprite, as opposed to the origin (upper left corner)
Vector2f TacObject::GetCenterOfSprite()
{
	float centerX = (sprite.getPosition().x + (sprite.getGlobalBounds().width) / 2);
	float centerY = (sprite.getPosition().y + (sprite.getGlobalBounds().height) / 2);
	return Vector2f(centerX, centerY);
}

TacObject::~TacObject()
{
}

// returns the parent vector
vector<TacObject*> TacObject::GetParents()
{
	return parents;
}

// adds a parent to the parents vector
void TacObject::AddParent(TacObject * newParent)
{
	if (newParent != nullptr && newParent != (TacObject*)0xcdcdcdcd)
	{
		this->parents.push_back(newParent);
		hasParent = true;
		SetUpLines();
	}
}

// adds a child to the children vector
void TacObject::AddChild(TacObject * newChild)
{
	if (newChild != nullptr)
	{
		this->children.push_back(newChild);
	}
}

// in the event of multiple children, place the children equidistant apart, centered above the parent
void TacObject::PlaceChildren()
{
	int numChildren = this->children.size();
	//int placeX = -((numChildren - 1) * 300) / 2;
	int placeX = 0;
	for (int i = 0; i < numChildren; i++, placeX += 300)
	{
		this->children[i]->Move(placeX, 0);
		if (i > 0)
		{
			// if the text overlaps, move it again
			if (this->children[i]->text.getGlobalBounds().intersects(this->children[i - 1]->text.getGlobalBounds()))
			{
				float xDiff = this->children[i - 1]->text.getGlobalBounds().left + this->children[i - 1]->text.getGlobalBounds().width;
				xDiff -= this->children[i]->text.getGlobalBounds().left;
				this->children[i]->Move(xDiff + 50, 0);
			}
		}
	}
	// do not move if the child has already been placed
	if (hasParent)
	{
		Move(0, -300);
	}
}

void TacObject::FixChildrenWithMultipleParents()
{
	if (parents.size() > 1)
	{
		Move(0, 300 * parents.size());
	}
}

// create the lines that connect the TacObjects together.
void TacObject::SetUpLines()
{
	lines.clear();
	if (hasParent)
	{
		for (TacObject* parent : parents)
		{
			// initialize the line and set its origin
			float xDistance = (this->GetCenterOfSprite().x - parent->GetCenterOfSprite().x);
			float yDistance = this->GetCenterOfSprite().y - parent->GetCenterOfSprite().y + lineOffset.y;
			float distance = sqrt(pow(xDistance, 2) + pow(yDistance, 2));
			RectangleShape line = RectangleShape(Vector2f(5, distance));
			line.setOrigin(2.5, 0);
			//line.setPosition(this->GetCenterOfSprite());
			line.setPosition(this->GetCenterOfSprite() + lineOffset);
			float angle;
			if (xDistance == 0)
			{
				angle = 0;
			}
			else
			{
				// xDistance and yDistance are switched because the x axis is opposite the angle and the y axis is adjacent
				angle = atan(xDistance / -yDistance) * (180 / M_PI);
				//angle = 45;
			}
			line.setRotation(angle);
			line.setFillColor(Color::Black);
			lines.push_back(line);
		}
	}
}
// translates the TacObject
void TacObject::Move(int deltaX, int deltaY, bool moveChildren)
{
	this->sprite.move(deltaX, deltaY);
	this->x += deltaX;
	this->y += deltaY;
	this->text.move(deltaX, deltaY);
	this->SetUpLines();
	if (moveChildren)
	{
		for (int i = 0; i < this->children.size(); i++)
		{
			this->children[i]->Move(deltaX, deltaY);
		}
	}
}
// sets the position of the TacObject
void TacObject::MoveTo(int newX, int newY)
{
	this->sprite.setPosition(newX, newY);
	this->x = newX;
	this->y = newY;
	this->text.setPosition(newX, newY);
	this->SetUpLines();
}
