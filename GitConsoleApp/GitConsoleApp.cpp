// GitConsoleApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <git2.h>
#include <SFML\Graphics.hpp>
#include "TacObject.h"
#include "TacCommit.h"
#include "tac.h"
#include <vector>
using namespace std;
int placeY;
bool AddTacCommit(vector<TacCommit*>& commits, TacCommit * newTacCommit)
{
	for (int i = 0; i < commits.size(); i++)
	{
		if (*commits[i] == *newTacCommit)
		{
			return false;
		}
	}
	commits.push_back(newTacCommit);
	return true;
}
// DEPRECATED
void AttachParent(vector<TacCommit*> &objects, TacCommit* child, int yParam)
{
	git_commit* parent;
	git_commit* gitChild = child->GetGitCommit();
	git_commit_parent(&parent, gitChild, 0);
	for (int i = 0; i < objects.size(); i++)
	{
		// if parent is a parent of gitChild, then
		//we connect the resulting TacCommits
		if (objects[i]->GetGitCommit() == parent)
		{
			for (int j = 0; j < git_commit_parentcount(gitChild); j++)
			{
				git_commit_parent(&parent, gitChild, j);
				TacCommit* tacParent = new TacCommit(parent, git_commit_message(parent), 0, yParam);
				child->AddParent(tacParent);
			}
		}
	}
}
// connects two TacObjects together if they are a parent/child pair
void AttachObjects(vector<TacCommit*>& objects, TacCommit * parent, int yParam)
{
	git_commit* gitChild;
	git_commit* gitParent = parent->GetGitCommit();
	git_commit* testParent;
	for (int i = 0; i < objects.size(); i++)
	{
		gitChild = objects[i]->GetGitCommit();
		// continue if gitChild has no parent
		if (git_commit_parent(&testParent, gitChild, 0))
		{
			continue;
		}
		// if testParent is parent of gitChild, connect gitChild to gitParent 
		if (testParent == gitParent)
		{
			parent->AddChild(objects[i]);
			objects[i]->AddParent(parent);
		}
	}
}
// Finds the TacCommit that is derived from the git_Commit "current"
TacCommit* FindExistingTacCommit(vector<TacCommit*> objects, git_commit* current)
{
	for (int i = 0; i < objects.size(); i++)
	{
		if (objects[i]->GetGitCommit() == current)
		{
			return objects[i];
		}
	}
	return nullptr;
}

void BuildTacTree(vector<TacCommit*> &objects, git_commit* current)
{
	TacCommit* tacCurrent = FindExistingTacCommit(objects, current);
	//git_commit* current;
	git_commit* parent;
	int placeX = 0;
	//placeY = 30;
	if (tacCurrent == nullptr)
	{
		tacCurrent = new TacCommit(current, git_commit_message(current), placeX, placeY);
	}
	git_commit_parent(&parent, current, 0);
	printf(git_commit_message(current));
	// exit the function if tacCurrent is already in objects
	//if (!AddTacCommit(objects, tacCurrent))
	//{
	//	return;
	//}
	AddTacCommit(objects, tacCurrent);
	placeY += 300;
	if (objects.size() > 1)
	{
		// do not link tacCurrent its preceding TacCommit if the preceding TacCommit is a root
		// this does not apply when there are only 2 TacCommit* in objects
		AttachObjects(objects, tacCurrent, placeY);
	}
	// until we reach a root
	for (int i = 0; i < git_commit_parentcount(current); i++)
	{
		current = parent;
		BuildTacTree(objects, parent);
	}
}

int main()
{
	// instantiate the objects to be displayed
	vector<TacCommit*> objects;
	// open the repository
	git_repository* repo = nullptr;
	int error = git_repository_init(&repo, "C:/Users/Caleb/Desktop/GitTest1", false);
	git_commit* current = nullptr;
	git_commit* parent = nullptr;
	TacCommit* tacCurrent; 
	git_oid oid;
	int placeX = 0;
	placeY = 30;
	// get all the branches
	git_branch_iterator* it;
	git_branch_iterator_new(&it, repo, GIT_BRANCH_ALL);
	git_reference* branch;
	git_branch_t local = GIT_BRANCH_LOCAL;
	while (git_branch_next(&branch, &local, it) != GIT_ITEROVER)
	{
		const char* branchName;
		branchName = git_reference_name(branch);
		git_reference_name_to_id(&oid, repo, branchName);
		git_commit_lookup(&current, repo, &oid);
		placeY = 0;
		BuildTacTree(objects, current);
	}
	for (int i = 0; i < objects.size(); i++)
	{
		objects[i]->PlaceChildren();
	}
	for (int i = 0; i < objects.size(); i++)
	{
		if (i == 0)
		{
			continue;
		}
	}
	sf::RenderWindow window(sf::VideoMode(1000, 1000), "SFML works!");
	View v1 = window.getDefaultView();
	View v2(FloatRect(0, 0, 1000, 1000));
	window.setView(v1);
	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
		}
		if (Keyboard::isKeyPressed(Keyboard::Left))
		{
			v1 = window.getView();
			v1.move(-1, 0);
			window.setView(v1);
		}
		if (Keyboard::isKeyPressed(Keyboard::Right))
		{
			v1 = window.getView();
			v1.move(1, 0);
			window.setView(v1);
		}
		if (Keyboard::isKeyPressed(Keyboard::Up))
		{
			v1 = window.getView();
			v1.move(0, -1);
			window.setView(v1);
		}
		if (Keyboard::isKeyPressed(Keyboard::Down))
		{
			v1 = window.getView();
			v1.move(0, 1);
			window.setView(v1);
		}
		window.clear(Color(255, 255, 255));
		for (int i = 0; i < objects.size(); i++)
		{
			objects[i]->DrawLines(window);
		}
		for (int i = 0; i < objects.size(); i++)
		{
			window.draw(*objects[i]);
			if (i != 0)
			{
			}
		}
		window.display();
	}
	for (int i = 0; i < objects.size(); i++)
	{
		delete objects[i];
	}
    return 0;
}